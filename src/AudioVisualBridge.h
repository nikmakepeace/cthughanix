// Audio-to-visual bridge.
//
// This layer consumes the current audio frame, updates acoustic context, and
// runs option-changing policy before visual buffer mutation begins.

#ifndef __AUDIO_VISUAL_BRIDGE_H
#define __AUDIO_VISUAL_BRIDGE_H

class AudioVisualBridge {
    int pipelineRefreshRequestedValue;

public:
    AudioVisualBridge();
    ~AudioVisualBridge();

    void runFrame();
    int pipelineRefreshRequested() const { return pipelineRefreshRequestedValue; }
    void clearPipelineRefreshRequest() { pipelineRefreshRequestedValue = 0; }
};

#endif
