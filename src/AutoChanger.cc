#include "cthugha.h"
#include "imath.h"
#include "AutoChanger.h"
#include "AudioAnalyzer.h"
#include "options.h"
#include "CthughaBuffer.h"
#include "Scene.h"
#include "VideoDirector.h"

OptionTime changeQuiet("quiet-change", 1500); /* change after quiet-pause (1.5 sec) */

/* Default to roughly the DOS Cthugha 5.3 dwell: 200-949 frames at the old
   320x200 VGA mode's ~70 Hz scan rate, or about 3-14 seconds. */
OptionTime changeWaitMin("min-time", 3000); /* min time between change (3 sec) */
OptionTime changeWaitRandom("random-time", 11000); /* extra random wait-time (11 sec) */

OptionInt changeCumulativeFireLevel("cumulative-fire-level", 1000);

OptionOnOff lock("lock", 0); /* change automatically */
OptionOnOff change_little("little", 0); /* only change one options */

AutoChanger* autoChanger = NULL;

AutoChanger::AutoChanger(SceneCommands& sceneCommands_)
    : sceneCommands(sceneCommands_)
    , quietSince(0)
    , lastChange(0) {

    if (changeWaitRandom <= 0)
        changeWaitRandom.setValue(1);

    /* set initial wait-time till change */
    waitTime = changeWaitMin + rand() % changeWaitRandom;

    read_ini_usage();

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
            waitTime = int(changeWaitMin) + rand() % max(1, int(changeWaitRandom));
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
        sprintf(txt, "locked ");
    } else {
        int now = gettime();

        sprintf(txt, "change: T:%.2f F:%d S:%.2f ", double(waitTime - (now - lastChange)) / 1000.0,
            changeCumulativeFireLevel - acousticContext.cumulativeFireLevel(),
            double(changeQuiet - (now - quietSince)) / 1000.0);
    }

    return txt;
}
