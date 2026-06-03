#include "cthugha.h"
#include "imath.h"
#include "AutoChanger.h"
#include "AudioAnalyzer.h"
#include "options.h"
#include "CthughaBuffer.h"
#include "Scene.h"
#include "VideoDirector.h"

OptionTime changeQuiet("quiet-change", DEFAULT_CHANGE_QUIET_MS); /* change after quiet-pause */

/* Default to roughly the DOS Cthugha 5.3 dwell: 200-949 frames at the old
   320x200 VGA mode's ~70 Hz scan rate, or about 3-14 seconds. */
OptionTime changeWaitMin("min-time", DEFAULT_CHANGE_WAIT_MIN_MS); /* min time between change */
OptionTime changeWaitRandom("random-time", DEFAULT_CHANGE_WAIT_RANDOM_MS); /* extra random wait-time */

OptionInt changeCumulativeFireLevel("cumulative-fire-level", DEFAULT_CHANGE_CUMULATIVE_FIRE_LEVEL);

OptionOnOff lock("lock", DEFAULT_AUTOCHANGER_LOCKED); /* change automatically */
OptionOnOff change_little("little", DEFAULT_AUTOCHANGER_CHANGE_LITTLE); /* only change one options */

AutoChanger* autoChanger = NULL;

AutoChanger::AutoChanger(SceneCommands& sceneCommands_)
    : sceneCommands(sceneCommands_)
    , quietSince(0)
    , lastChange(0) {

    if (changeWaitRandom <= 0)
        changeWaitRandom.setValue(DEFAULT_CHANGE_WAIT_RANDOM_MIN_MS);

    /* set initial wait-time till change */
    waitTime = changeWaitMin + rand() % changeWaitRandom;

    lastChange = gettime();
    quietSince = gettime();
}

AutoChanger::~AutoChanger() {
    if (options_save) {
        write_ini();
    }
}

void AutoChanger::operator()() {

    int now = gettime();

    /* get time since last sound */
    int quiet_length = now - quietSince;
    if (audioMetrics.noisy)
        quietSince = now;

    /* Report long quietness to visual policy. */
    if (!audioMetrics.noisy && videoDirector().observeQuiet(quiet_length))
        quietSince = now; // start quiet-length again

    if (int(lock))
        return;

    /* Check for interrupted silence (like the pause btw. 2 tracks on a CD) */
    if (int(changeQuiet))
        if (audioMetrics.noisy && (quiet_length > int(changeQuiet))) {
            change();
            return;
        }

    /* Check for enough fire to change */
    if (int(changeCumulativeFireLevel))
        if (acousticContext.cumulativeFireLevel() > int(changeCumulativeFireLevel)) {
            CTH_DEBUG("autochange: cumulativeFireLevel threshold reached level=%d threshold=%d\n",
                acousticContext.cumulativeFireLevel(), int(changeCumulativeFireLevel));
            acousticContext.resetCumulativeFireLevel();
            change();
            return;
        }

    /* nothing special happend, maybe we waited long enough */
    if ((changeWaitMin + changeWaitRandom) > 0)
        if ((now - lastChange) > int(waitTime)) {
            lastChange = now;
            waitTime = int(changeWaitMin)
                + rand() % max(DEFAULT_CHANGE_WAIT_RANDOM_MIN_MS, int(changeWaitRandom));
            change();
            return;
        }
}

void AutoChanger::change() {

    if (int(change_little)) {
        sceneCommands.changeOne();
    } else {
        sceneCommands.changeAll();
    }
}

const char* AutoChanger::status() {
    static char txt[512];

    if (lock) {
        snprintf(txt, sizeof(txt), "locked ");
    } else {
        int now = gettime();

        snprintf(txt, sizeof(txt), "change: T:%.2f F:%d S:%.2f ", double(waitTime - (now - lastChange)) / 1000.0,
            changeCumulativeFireLevel - acousticContext.cumulativeFireLevel(),
            double(changeQuiet - (now - quietSince)) / 1000.0);
    }

    return txt;
}
