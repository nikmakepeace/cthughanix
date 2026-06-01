#ifndef __VIDEO_FILTERCHAIN_SEQUENCE_H
#define __VIDEO_FILTERCHAIN_SEQUENCE_H

#include <vector>

class VideoFilterchainSequence {
    std::vector<unsigned int> sequenceValue;

public:
    enum Stage {
        ImageStage,
        FlashlightStage,
        BorderStage,
        FlameStage,
        TranslateStage,
        WaveStage,
        FrameCommitStage,
        PaletteStage
    };

    VideoFilterchainSequence();

    void append(Stage stage);
    int includes(Stage stage) const;
    const std::vector<unsigned int>& sequence() const { return sequenceValue; }
};

#endif
