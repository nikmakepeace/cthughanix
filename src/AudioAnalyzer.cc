#include "AudioAnalyzer.h"
#include "ProcessServices.h"

AcousticContext::AcousticContext(LogSink* log_)
    : log(log_)
    , intensityValue(0.0)
    , lastAmplitudeValue(0)
    , attackLevelValue(0)
    , fireValue(0)
    , cumulativeFireLevelValue(0)
    , fireSensitivityValue(100) { }

static int clampFireSensitivity(int sensitivity) {
    if (sensitivity < 0)
        return 0;
    if (sensitivity > 100)
        return 100;
    return sensitivity;
}

static int minimumFireForSensitivity(int sensitivity) {
    return (100 - clampFireSensitivity(sensitivity)) * 2;
}

void AcousticContext::update(const AudioMetrics& metrics) {
    /* Rolling acoustic state lives here rather than in the frame metrics.
       Future context providers can add slower signals without changing the
       frame-local AudioMetrics contract. */
    int amplitude = metrics.amplitude;

    if (amplitude < lastAmplitudeValue - 9) /* ignore such a small decrease */
        amplitude = lastAmplitudeValue - 9;

    if (amplitude > lastAmplitudeValue)
        attackLevelValue += amplitude - lastAmplitudeValue;

    /* If the attack is over, fire at the intensity of the accumulated attack. */
    if (amplitude < lastAmplitudeValue) {
        fireValue = attackLevelValue;
        attackLevelValue = 0;
        if (fireValue <= minimumFireForSensitivity(fireSensitivityValue))
            fireValue = 0;

        if ((fireValue > 0) && (log != NULL))
            log->trace("sound fire", "fire=%d amplitude=%d lastamp=%d\n",
                fireValue, amplitude, lastAmplitudeValue);
    } else
        fireValue = 0;

    lastAmplitudeValue = amplitude;
    intensityValue = intensityValue * 0.95 + (metrics.amplitude / 128.0) * 0.05;
    cumulativeFireLevelValue += fireValue;
}

void AcousticContext::setFireSensitivity(int sensitivity) {
    fireSensitivityValue = clampFireSensitivity(sensitivity);
}

int AcousticContext::fireSensitivity() const {
    return fireSensitivityValue;
}

double AcousticContext::intensity() const {
    return intensityValue;
}

int AcousticContext::fire() const {
    return fireValue;
}

int AcousticContext::cumulativeFireLevel() const {
    return cumulativeFireLevelValue;
}

void AcousticContext::resetCumulativeFireLevel() {
    cumulativeFireLevelValue = 0;
}
