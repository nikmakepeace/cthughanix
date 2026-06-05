/** @file
 * Runtime audio control adapter.
 */

#include "RuntimeAudioControls.h"

#include "AudioAnalyzer.h"
#include "AudioFrame.h"
#include "AudioProcessor.h"

void DefaultRuntimeAudioControls::changeSoundProcessingBy(int by) {
    audioProcessing.change(by);
}

void DefaultRuntimeAudioControls::changeSoundProcessingTo(const char* to) {
    audioProcessing.change(to);
}

void DefaultRuntimeAudioControls::resetAudioFrame() {
    audioFrameChange();
}

int DefaultRuntimeAudioControls::changeAudioOptionBy(
    Option& option, int by, RuntimeChangeSet& changes) {
    if ((&option != &audioProcessing) && (&option != &sound_minnoise))
        return 0;

    option.change(by);
    changes.audioProcessingChanged = (&option == &audioProcessing);
    return 1;
}

int DefaultRuntimeAudioControls::changeAudioOptionTo(
    Option& option, const char* to, RuntimeChangeSet& changes) {
    if ((&option != &audioProcessing) && (&option != &sound_minnoise))
        return 0;

    option.change(to);
    changes.audioProcessingChanged = (&option == &audioProcessing);
    return 1;
}
