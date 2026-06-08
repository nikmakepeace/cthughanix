#ifndef __VIDEO_FILTERCHAIN_SEQUENCE_H
#define __VIDEO_FILTERCHAIN_SEQUENCE_H

#include <vector>

/**
 * Ordered list of video filter stages used when constructing a filterchain.
 */
class VideoFilterchainSequence {
    std::vector<unsigned int> sequenceValue;

public:
    /** Named stage ids understood by VideoFilterchainFactory and Frame Generator. */
    enum Stage {
        /** One-shot indexed image injection into active/passive buffers. */
        ImageStage,

        /** Palette flashlight adjustment driven by acoustic context. */
        FlashlightStage,

        /** Hidden border rows used by flame feedback. */
        BorderStage,

        /** Flame feedback stage. */
        FlameStage,

        /** Coordinate remap/translation stage. */
        TranslateStage,

        /** Sound-reactive wave drawing stage. */
        WaveStage,

        /** Text cue injection stage. */
        TextStage,

        /** Active/passive buffer swap and frame diagnostics. */
        FrameCommitStage,

        /** Palette transition stage. */
        PaletteStage,

        /** Final IndexedFrame publication stage. */
        IndexedFrameStage
    };

    VideoFilterchainSequence();

    /**
     * Appends a stage to the execution order.
     *
     * @param stage Stage id to append.
     */
    void append(Stage stage);

    /**
     * @param stage Stage id to search for.
     * @return Nonzero when the stage appears in the sequence.
     */
    int includes(Stage stage) const;

    /** @return Ordered stage ids as unsigned ints for VideoFilterchain. */
    const std::vector<unsigned int>& sequence() const { return sequenceValue; }
};

#endif
