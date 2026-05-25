#include "cthugha.h"
#include "AudioFrame.h"
#include "AudioRuntime.h"

static char2 silentAudioFrameData[1024];
static char2 silentAudioFrameProcessedData[1024];

AudioFrame::AudioFrame() {
    clear();
}

void AudioFrame::clear() {
    centerByte = 0;
    samples = 0;
    rawBytes = 0;
    memset(data, 0, sizeof(data));
    memset(processed, 0, sizeof(processed));
}

void audioFrameTick() {
    double tickStart = getTime();
    audioRuntimeTick();
    CTH_TRACE("audioFrameTick-ms=%.3f\n", "audio timing", (getTime() - tickStart) * 1000.0);
}

void audioFrameChange() {
    if (audioRuntimeProcessor()) {
        audioRuntimeProcessor()->change();
        return;
    }

    if (soundDevice)
        soundDevice->change();
}

AudioFrame* audioFrameCurrent() {
    return audioRuntimeCurrentFrame();
}

char2* audioFrameData() {
    if (audioRuntimeCurrentFrame())
        return audioRuntimeCurrentFrame()->data;

    if (audioRuntimeProcessor())
        return audioRuntimeProcessor()->data;

    return soundDevice ? soundDevice->data : silentAudioFrameData;
}

char2* audioFrameProcessedData() {
    if (audioRuntimeCurrentFrame())
        return audioRuntimeCurrentFrame()->processed;

    if (audioRuntimeProcessor())
        return audioRuntimeProcessor()->dataProc;

    return soundDevice ? soundDevice->dataProc : silentAudioFrameProcessedData;
}

int audioFrameBroadcastBytes() {
    if (audioRuntimeCurrentFrame())
        return audioRuntimeCurrentFrame()->rawBytes;

    if (audioRuntimeProcessor())
        return audioRuntimeProcessor()->frameRawSize();

    return soundDevice ? soundDevice->frameRawSize() : 0;
}
