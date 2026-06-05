/** @file
 * Automatic scene-change policy implementation.
 */

#include "cthugha.h"
#include "imath.h"
#include "AutoChanger.h"
#include "AutoChangeSettings.h"
#include "AudioAnalyzer.h"
#include "CthughaBuffer.h"
#include "RuntimeCommandSink.h"
#include "Scene.h"
#include "VideoDirector.h"

AutoChanger::AutoChanger(RuntimeCommandSink& runtimeCommands_,
    const AutoChangeSettings& settings_,
    AcousticContext& acousticContext_)
    : runtimeCommands(runtimeCommands_)
    , settings(settings_)
    , acousticContextValue(acousticContext_)
    , quietSince(0)
    , lastChange(0) {

    /* set initial wait-time till change */
    waitTime = settings.waitMinMs() + rand() % settings.waitRandomRangeMs();

    lastChange = gettime();
    quietSince = gettime();
}

AutoChanger::~AutoChanger() { }

void AutoChanger::operator()(const AudioMetrics& metrics) {

    int now = gettime();

    /* get time since last sound */
    int quiet_length = now - quietSince;
    if (metrics.noisy)
        quietSince = now;

    /* Report long quietness to visual policy. */
    if (!metrics.noisy && videoDirector().observeQuiet(quiet_length))
        quietSince = now; // start quiet-length again

    if (settings.locked())
        return;

    /* Check for interrupted silence (like the pause btw. 2 tracks on a CD) */
    if (settings.quietMs())
        if (metrics.noisy && (quiet_length > settings.quietMs())) {
            change();
            return;
        }

    /* Check for enough fire to change */
    if (settings.cumulativeFireLevel())
        if (acousticContextValue.cumulativeFireLevel() > settings.cumulativeFireLevel()) {
            CTH_DEBUG("autochange: cumulativeFireLevel threshold reached level=%d threshold=%d\n",
                acousticContextValue.cumulativeFireLevel(),
                settings.cumulativeFireLevel());
            acousticContextValue.resetCumulativeFireLevel();
            change();
            return;
        }

    /* nothing special happend, maybe we waited long enough */
    if ((settings.waitMinMs() + settings.waitRandomMs()) > 0)
        if ((now - lastChange) > int(waitTime)) {
            lastChange = now;
            waitTime = settings.waitMinMs()
                + rand() % settings.waitRandomRangeMs();
            change();
            return;
        }
}

void AutoChanger::change() {

    if (settings.changeLittle()) {
        runtimeCommands.apply(RuntimeCommand::changeOne());
    } else {
        runtimeCommands.apply(RuntimeCommand::changeAll());
    }
}

const char* AutoChanger::status() const {
    static char txt[512];

    if (settings.locked()) {
        snprintf(txt, sizeof(txt), "locked ");
    } else {
        int now = gettime();

        snprintf(txt, sizeof(txt), "change: T:%.2f F:%d S:%.2f ", double(waitTime - (now - lastChange)) / 1000.0,
            settings.cumulativeFireLevel() - acousticContextValue.cumulativeFireLevel(),
            double(settings.quietMs() - (now - quietSince)) / 1000.0);
    }

    return txt;
}
