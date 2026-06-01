// Audio-to-visual bridge.
//
// This layer consumes the current audio frame, updates acoustic context, and
// runs option-changing policy before visual buffer mutation begins.

#ifndef __AUDIO_VISUAL_BRIDGE_H
#define __AUDIO_VISUAL_BRIDGE_H

class SceneCommands;

class AudioVisualBridge {
    int filterchainRefreshRequestedValue;
    SceneCommands* sceneCommands;

public:
    AudioVisualBridge(SceneCommands* sceneCommands_ = 0);
    ~AudioVisualBridge();

    void runFrame();
    int filterchainRefreshRequested() const { return filterchainRefreshRequestedValue; }
    void clearFilterchainRefreshRequest() { filterchainRefreshRequestedValue = 0; }
};

#endif
