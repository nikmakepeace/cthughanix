// Visual pipeline selection scaffold.

#ifndef __VISUAL_DIRECTOR_H
#define __VISUAL_DIRECTOR_H

#include "VisualPipeline.h"

class Environment;
class Settings;

class VisualPlan {
    unsigned int stagesValue;
    std::vector<unsigned int> sequenceValue;


public:

    // bitmask of stages included in the plan
    enum Stage {
        BufferFrameBeginStage = 1 << 0,
        ImageStage = 1 << 1,
        FlashlightStage = 1 << 2,
        BorderStage = 1 << 3,
        FlameStage = 1 << 4,
        TranslateStage = 1 << 5,
        WaveStage = 1 << 6,
        BufferFrameEndStage = 1 << 7,
        PaletteStage = 1 << 8
    };

    VisualPlan();

    void include(Stage stage);
    void append(Stage stage);
    int includes(Stage stage) const;
    unsigned int stages() const { return stagesValue; }
    const std::vector<unsigned int>& sequence() const { return sequenceValue; }
};

class VisualDirector {
public:
    VisualPlan planDefaultPipeline() const;
    void configurePipeline(VisualPipeline& pipeline) const;
};

extern double paletteSmoothingChance;

class VisualPipelineFactory {
public:
    VisualPipelineFactory();

    VisualPipeline* create(const VisualPlan& plan) const;
    void refresh(VisualPipeline& pipeline, const VisualPlan& plan) const;
};

#endif
