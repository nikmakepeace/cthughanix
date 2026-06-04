#include "cthugha.h"
#include "AudioSettings.h"
#include "Configuration.h"

#include <string.h>

AudioSettings::AudioSettings()
    : audioInputMode(0)
    , soundDSPMethod(0)
    , silent(0) {
    fileName[0] = '\0';
}

void AudioSettings::refreshFromConfig(const AudioConfig& config) {
    audioInputMode = int(config.inputMode);
    strncpy(fileName, config.inputFile.c_str(), PATH_MAX);
    fileName[PATH_MAX - 1] = '\0';
}

AudioSettings AudioSettings::fromConfig(const AudioConfig& config) {
    AudioSettings settings;
    settings.refreshFromConfig(config);
    settings.soundDSPMethod = config.dspMethod;
    settings.silent = config.silentEnabled;

    CTH_DEBUG("runtime settings: audio-input-mode=%d sound-dsp-method=%d silent=%d file=`%s'\n",
        settings.audioInputMode, settings.soundDSPMethod, settings.silent, settings.fileName);

    return settings;
}
