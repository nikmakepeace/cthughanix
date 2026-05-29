// Visual pipeline selection scaffold.

#ifndef __VISUAL_DIRECTOR_H
#define __VISUAL_DIRECTOR_H

#include "VisualPipeline.h"

#include <vector>

class CthughaBuffer;
class Environment;

class VisualPlan {
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

    VisualPlan();

    void append(Stage stage);
    int includes(Stage stage) const;
    const std::vector<unsigned int>& sequence() const { return sequenceValue; }
};

class VisualDirector {
    int lastPcxSelection;

    int pcxSelectionChanged();
    void syncCurrentBuffer();
    void updatePipelineStages(VisualPipeline& pipeline, CthughaBuffer& buffer);

public:
    VisualDirector();

    VisualPlan planDefaultPipeline() const;
    CthughaBuffer* configurePipeline(VisualPipeline& pipeline);
};

extern double paletteSmoothingChance;

class VisualPipelineFactory {
public:
    VisualPipelineFactory();

    VisualPipeline* create(const VisualPlan& plan) const;
    void refresh(VisualPipeline& pipeline, const VisualPlan& plan) const;
};

#endif
