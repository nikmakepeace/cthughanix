// Visual pipeline selection scaffold.

#ifndef __VISUAL_DIRECTOR_H
#define __VISUAL_DIRECTOR_H

#include "VisualPipeline.h"

class Environment;
class Settings;

class VisualPlan {
    unsigned int stagesValue;

public:
    enum Stage {
        BufferTransformStage = 1 << 0,
        FlashlightStage = 1 << 1,
        BorderStage = 1 << 2,
        ImageStage = 1 << 3,
        FlameStage = 1 << 4,
        TranslateStage = 1 << 5,
        WaveStage = 1 << 6,
        PaletteStage = 1 << 7
    };

    VisualPlan();

    void include(Stage stage);
    int includes(Stage stage) const;
    unsigned int stages() const { return stagesValue; }
};

class VisualDirector {
public:
    VisualPlan planDefaultPipeline() const;
};

typedef void (*VisualBufferTransformFn)();
void setVisualBufferTransform(VisualBufferTransformFn transform);

class VisualPipelineFactory {
public:
    VisualPipelineFactory();

    VisualPipeline* create(const VisualPlan& plan) const;
    void refresh(VisualPipeline& pipeline, const VisualPlan& plan) const;
};

#endif
