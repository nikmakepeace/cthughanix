#ifndef __PIPELINE_STAGE_MODULES_H
#define __PIPELINE_STAGE_MODULES_H

#include "PaletteTransition.h"
#include "VisualPipeline.h"

class Flame;
class PaletteEntry;
class PCXEntry;
class TranslateOption;
class Wave;

class ImageStageModule : public VisualModule {
    PCXEntry* image;

public:
    ImageStageModule();

    void setImage(PCXEntry* image_);
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

public:
    TranslateStageModule();

    void setTranslateProvider(TranslateOption* translate_);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

class WaveStageModule : public VisualModule {
    Wave* currentWave;

public:
    WaveStageModule();

    void setWave(Wave* wave_);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

class FrameCommitModule : public VisualModule {
    const char* flameName;

public:
    FrameCommitModule();

    void setFlameName(const char* flameName_);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

class FlashlightVisualModule : public VisualModule {
public:
    FlashlightVisualModule();

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
    PaletteTransition palette;

public:
    PaletteStageModule();

    int needsTarget(PaletteEntry* paletteEntry) const;
    void setPalette(PaletteEntry* paletteEntry, int frameBudget);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

#endif
