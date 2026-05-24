#include "cthugha.h"
#include "AudioFrame.h"
#include "AudioRuntime.h"

void audioFrameTick() {
    if (audioRuntimeProcessor()) {
        (*audioRuntimeProcessor())();
        return;
    }

    if (soundDevice)
        (*soundDevice)();
}

void audioFrameChange() {
    if (audioRuntimeProcessor()) {
        audioRuntimeProcessor()->change();
        return;
    }

    if (soundDevice)
        soundDevice->change();
}

char2* audioFrameData() {
    if (audioRuntimeProcessor())
        return audioRuntimeProcessor()->data;

    return soundDevice ? soundDevice->data : NULL;
}

char2* audioFrameProcessedData() {
    if (audioRuntimeProcessor())
        return audioRuntimeProcessor()->dataProc;

    return soundDevice ? soundDevice->dataProc : NULL;
}

int audioFrameBroadcastBytes() {
    if (audioRuntimeProcessor())
        return audioRuntimeProcessor()->frameRawSize();

    return soundDevice ? soundDevice->frameRawSize() : 0;
}
