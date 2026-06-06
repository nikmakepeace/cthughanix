/** @file
 * Automatic scene-change policy implementation.
 */

#include "AutoChanger.h"
#include "AutoChangeSettings.h"
#include "AudioAnalyzer.h"
#include "ProcessServices.h"
#include "RuntimeCommandSink.h"

AutoChanger::AutoChanger(RuntimeCommandSink& runtimeCommands_,
    const AutoChangeSettings& settings_,
    AcousticContext& acousticContext_, MillisecondClock& clock_,
    RandomSource& randomSource_, AutoChangeQuietObserver& quietObserver_,
    LogSink& log_)
    : runtimeCommands(runtimeCommands_)
    , settings(settings_)
    , acousticContextValue(acousticContext_)
    , clock(clock_)
    , randomSource(randomSource_)
    , quietObserver(quietObserver_)
    , log(log_)
    , quietSince(0)
    , lastChange(0)
    , statusTextValue() {

    /* set initial wait-time till change */
    waitTime = nextWaitTime();

    lastChange = clock.milliseconds();
    quietSince = clock.milliseconds();
}

AutoChanger::~AutoChanger() { }

int AutoChanger::nextWaitTime() {
    return settings.waitMinMs()
        + randomSource.uniformInt(settings.waitRandomRangeMs());
}

void AutoChanger::operator()(const AudioMetrics& metrics) {

    int now = clock.milliseconds();

    /* get time since last sound */
    int quiet_length = now - quietSince;
    if (metrics.noisy)
        quietSince = now;

    /* Report long quietness to visual policy. */
    if (!metrics.noisy && quietObserver.observeQuiet(quiet_length))
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
            log.debug("autochange: cumulativeFireLevel threshold reached level=%d threshold=%d\n",
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
            waitTime = nextWaitTime();
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
    if (settings.locked()) {
        snprintf(statusTextValue, sizeof(statusTextValue), "locked ");
    } else {
        int now = clock.milliseconds();

        snprintf(statusTextValue, sizeof(statusTextValue), "change: T:%.2f F:%d S:%.2f ", double(waitTime - (now - lastChange)) / 1000.0,
            settings.cumulativeFireLevel() - acousticContextValue.cumulativeFireLevel(),
            double(settings.quietMs() - (now - quietSince)) / 1000.0);
    }

    return statusTextValue;
}

const char* AutoChanger::autoChangerStatus() const {
    return status();
}
