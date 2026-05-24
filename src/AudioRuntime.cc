#include "cthugha.h"
#include "AudioRuntime.h"

static AudioProcessor* audioProcessor = NULL;

static const char* audioRuntimeContextName(RuntimeSoundInputContext context) {
    return (context == RSIC_FileChild) ? "file child" : "main process";
}

void audioRuntimeInit(RuntimeSoundInputContext context, int initializeInputControls) {
    CTH_TRACE("audio runtime: init requested context=%s initialize-input-controls=%d\n",
        audioRuntimeContextName(context), initializeInputControls);

    if (audioRuntimeIsInitialized()) {
        CTH_TRACE("audio runtime: init skipped because audio is already installed\n");
        return;
    }

    RuntimeFactory runtimeFactory(Settings::fromCurrentOptions(), Environment::detect());
    audioProcessor = runtimeFactory.createAudioProcessor();
    if (audioProcessor != NULL) {
        CTH_TRACE("audio runtime: installed native AudioProcessor path\n");
        if (initializeInputControls && audioProcessor->audioInput()->initInputControls()) {
            CTH_TRACE("audio runtime: native input control initialization failed\n");
            exit(0);
        }
    } else {
        CTH_TRACE("audio runtime: native AudioProcessor unavailable; using legacy SoundDevice path\n");
        SoundDevice::install(runtimeFactory.createLegacySoundDevice(context), initializeInputControls);
    }

    CTH_TRACE("audio runtime: init completed context=%s\n", audioRuntimeContextName(context));
}

void audioRuntimeShutdown() {
    CTH_TRACE("audio runtime: shutdown requested native=%d legacy=%d\n",
        audioProcessor != NULL, soundDevice != NULL);
    delete audioProcessor;
    audioProcessor = NULL;

    delete soundDevice;
    soundDevice = NULL;
    CTH_TRACE("audio runtime: shutdown completed\n");
}

int audioRuntimeIsInitialized() {
    return (audioProcessor != NULL) || (soundDevice != NULL);
}

AudioProcessor* audioRuntimeProcessor() {
    return audioProcessor;
}
