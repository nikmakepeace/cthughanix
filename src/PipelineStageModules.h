#ifndef __PIPELINE_STAGE_MODULES_H
#define __PIPELINE_STAGE_MODULES_H

#include "FramePalette.h"
#include "Image.h"
#include "PaletteTransition.h"
#include "VisualPipeline.h"
#include "VisualPipelineSequence.h"

class Flame;
class PaletteEntry;
class TranslateOption;
class Wave;

class ImageStageModule : public VisualModule {
    const IndexedImage* image;
    ImagePlacement placement;
    int overlayPassiveBuffer;

public:
    ImageStageModule();

    void setImage(const IndexedImage* image_);
    void setPlacement(const ImagePlacement& placement_);
    void setOverlayPassiveBuffer(int enabled);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

class FlameStageModule : public VisualModule {
    const Flame* flame;
    int generalFlame;

public:
    FlameStageModule();

    void setFlame(const Flame* flame_);
    void setGeneralFlame(int generalFlame_);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

class TranslateStageModule : public VisualModule {
    TranslateOption* translate;
    int translateIndex;

public:
    TranslateStageModule();

    void setTranslate(TranslateOption* translate_, int translateIndex_);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

class WaveStageModule : public VisualModule {
    Wave* wave;

public:
    WaveStageModule();

    void setWave(Wave* wave_);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

class FrameCommitModule : public VisualModule {
    const char* flameName;
    const char* waveName;
    const char* waveScaleName;
    const char* tableName;

public:
    FrameCommitModule();

    void setSceneNames(const char* flameName_, const char* waveName_,
        const char* waveScaleName_, const char* tableName_);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

class FlashlightVisualModule : public VisualModule {
    FramePalette* framePalette;

public:
    FlashlightVisualModule();

    void setFramePalette(FramePalette* framePalette_);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

class BorderVisualModule : public VisualModule {
    int borderMode;

public:
    BorderVisualModule();

    void setBorderMode(int borderMode_);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

class PaletteStageModule : public VisualModule {
    PaletteTransition transition;
    FramePalette framePaletteValue;

public:
    PaletteStageModule();

    FramePalette& framePalette();
    int needsTarget(PaletteEntry* paletteEntry) const;
    void setTargetPalette(PaletteEntry* paletteEntry, int frameBudget,
        const PaletteTransitionStrategy& strategy);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

FramePalette* framePaletteFromPipeline(VisualPipeline& pipeline);

#endif
