// Audio option registry and lifecycle wrapper for the current audio runtime.
// Backend selection enters through AudioRuntime/RuntimeFactory; option changes
// notify the active Audio* source so it can reconfigure device buffers.

#include "cthugha.h"
#include "AudioSystem.h"
#include "AudioOptions.h"
#include "AudioFrame.h"
#include "AudioRuntime.h"
#include "Interface.h"

class OptionAudio : public OptionInt {
public:
    OptionAudio(const char* name, int iV, int maxV = 0, int minV = 0)
        : OptionInt(name, iV, maxV, minV) { }

    virtual void change(int by) {
        OptionInt::change(by);
        audioFrameChange();
    }

    virtual void change(const char* to) { OptionInt::change(to); }
};

class OptionAudioOnOff : public OptionOnOff {
public:
    OptionAudioOnOff(const char* name, int iV)
        : OptionOnOff(name, iV) { }

    virtual void change(int by) {
        OptionOnOff::change(by);
        audioFrameChange();
    }

    virtual void change(const char* to) { OptionOnOff::change(to); }
};

class OptionChannels : public OptionAudio {
public:
    OptionChannels(const char* name, int iV)
        // OptionInt's maximum is exclusive, so this accepts 1 or 2 channels.
        : OptionAudio(name, iV, SOUND_CHANNELS_MAX_EXCLUSIVE, SOUND_CHANNELS_MIN) { }

    const char* text() const {
        switch (value) {
        case 1:
            return "mono";
        case 2:
            return "stereo";
        default:
            return "unknown";
        }
    }
};

class OptionFormat : public OptionAudio {
    static const char* fmts[];
    static const int nFmts;

public:
    OptionFormat(const char* name, int iV)
        : OptionAudio(name, iV, nFmts) { }

    virtual void change(const char* to) {
        for (value = 0; value < nFmts; value++)
            if (strcasecmp(fmts[value], to) == 0)
                return;

        OptionInt::change(to);
    }

    const char* text() const {
        if ((value >= 0) && (value < nFmts))
            return fmts[value];
        return "unknown";
    }
};

const char* OptionFormat::fmts[] = {
    "8bit unsigned",
    "8bit signed",
    "16bit unsigned (le)",
    "16bit signed (le)",
    "16bit unsigned (be)",
    "16bit signed (be)",
};
const int OptionFormat::nFmts = sizeof(OptionFormat::fmts) / sizeof(const char*);

int audioInputLoop = DEFAULT_AUDIO_INPUT_LOOP_ENABLED;

char dev_dsp[PATH_MAX] = DEFAULT_DSP_DEVICE_PATH;

static OptionInt audioInputModeImpl = OptionInt("sound-device-number", DEFAULT_AUDIO_INPUT_MODE, AIM_Max);

static OptionAudio soundSampleRateImpl("sound-sample-rate", DEFAULT_SOUND_SAMPLE_RATE_HZ);
static OptionChannels soundChannelsImpl("sound-channels", DEFAULT_SOUND_CHANNELS);
static OptionFormat soundFormatImpl("sound-format", DEFAULT_SOUND_FORMAT);

static OptionAudio soundDSPMethodImpl("sound-method", DEFAULT_SOUND_DSP_METHOD);
static OptionAudio soundDSPFragmentsImpl("sound-fragments", DEFAULT_SOUND_DSP_FRAGMENTS);
static OptionAudio soundDSPFragmentSizeImpl("sound-fragment-size", DEFAULT_SOUND_DSP_FRAGMENT_SIZE);
static OptionOnOff soundDSPSyncImpl("sound-sync", DEFAULT_SOUND_DSP_SYNC_ENABLED);

static OptionAudioOnOff soundSilentImpl("silent", DEFAULT_SOUND_SILENT_ENABLED);

Option& audioInputMode = audioInputModeImpl;
Option& soundSampleRate = soundSampleRateImpl;
Option& soundChannels = soundChannelsImpl;
Option& soundFormat = soundFormatImpl;

Option& soundDSPMethod = soundDSPMethodImpl;
Option& soundDSPFragments = soundDSPFragmentsImpl;
Option& soundDSPFragmentSize = soundDSPFragmentSizeImpl;
Option& soundDSPSync = soundDSPSyncImpl;

Option& soundSilent = soundSilentImpl;

InterfaceElement* elementsAudio[] = {
    new InterfaceElementOption("Sample rate       : %10s Hz", &soundSampleRate, 500, 1000, 5000),
    new InterfaceElementOption("Channels          : %10s", &soundChannels),
    new InterfaceElementOption("Format            : %10s", &soundFormat, 1, 1, 1),
#if WITH_DSP == 1
    new InterfaceElementOption("DSP Method        : %10s", &soundDSPMethod),
    new InterfaceElementOption("DSP Fragments     : %10s", &soundDSPFragments),
    new InterfaceElementOption("DSP Fragment Size : %10s", &soundDSPFragmentSize),
    new InterfaceElementOption("DSP Sound sync    : %10s", &soundDSPSync),
#endif
};
int nElementsAudio = sizeof(elementsAudio) / sizeof(InterfaceElement*);

Interface interfaceAudio("sound", "Audio Interface", NULL, elementsAudio, nElementsAudio);

int init_sound(int visualMaxDimension) {
    if (audioRuntimeIsInitialized())
        return 0;

    return audioRuntimeInit(1, visualMaxDimension);
}

int exit_sound() {
    audioRuntimeShutdown();
    return 0;
}

Interface interfacePlayList("playList", "Sound Play List", "Not yet implemented");
