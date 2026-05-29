#include "cthugha.h"
#include "Border.h"
#include "CthughaBuffer.h"
#include "CthughaDisplay.h"
#include "Flame.h"
#include "Flashlight.h"
#include "VisualDirector.h"
#include "cth_buffer.h"
#include "imath.h"
#include "waves.h"

double paletteSmoothingChance = 1.0;

static CthughaBuffer* currentBuffer() {
    return CthughaBuffer::current;
}

static const Flame* flameFromOption(CoreOption& option) {
    FlameEntry* entry = dynamic_cast<FlameEntry*>(option.current());
    return (entry != 0) ? &entry->flame() : 0;
}

template <class Module>
static Module* stageModule(VisualPipeline& pipeline, VisualPlan::Stage stage) {
    return dynamic_cast<Module*>(pipeline.stageModule(stage));
}

class ImageStageModule : public VisualModule {
    PCXEntry* image;

public:
    ImageStageModule()
        : image(0) { }

    void setImage(PCXEntry* image_) {
        image = image_;
    }

    void execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
        (void)context;

        CTH_TRACE("executing image stage\n", "visual pipeline");
        if (image != 0)
            image->overlay(buffer);
    }
};

class FlameStageModule : public VisualModule {
    const Flame* flame;

public:
    FlameStageModule()
        : flame(0) { }

    void setFlame(const Flame* flame_) {
        flame = flame_;
    }

    void execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
        CTH_TRACE("executing flame stage\n", "visual pipeline");

        if (flame != 0)
            flame->execute(buffer, context);
    }
};

class TranslateStageModule : public VisualModule {
    TranslateOption* translate;

public:
    TranslateStageModule()
        : translate(0) { }

    void setTranslateProvider(TranslateOption* translate_) {
        translate = translate_;
    }

    void execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
        CTH_TRACE("executing translate stage\n", "visual pipeline");
        TranslateEntry* entry = NULL;
        if (translate != 0
            && translate->prepareCurrentEntry(entry) == 0
            && entry != NULL)
            entry->execute(buffer, context);
    }
};

class WaveStageModule : public VisualModule {
    WaveEntry* wave;

public:
    WaveStageModule()
        : wave(0) { }

    void setWave(WaveEntry* wave_) {
        wave = wave_;
    }

    void execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
        CTH_TRACE("executing wave stage\n", "visual pipeline");
        if (wave != NULL)
            wave->execute(buffer, context);
    }
};

class FrameCommitModule : public VisualModule {
public:
    FrameCommitModule() { }

    void execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
        (void)context;

        CTH_TRACE("committing indexed buffer frame\n", "visual pipeline");
        static int debugReports = 0;

        if (CTH_LOG_ENABLED(CTH_LOG_DEBUG) && (debugReports < 16)) {
            int nonzero = 0;
            int peak = 0;
            for (int i = 0; i < BUFF_SIZE; i++) {
                int value = buffer.activePixels()[i];
                if (value != 0)
                    nonzero++;
                if (value > peak)
                    peak = value;
            }
            debugReports++;
            CTH_DEBUG("visual buffer: wave=%s wave-scale=%s flame=%s table=%s nonzero-pixels=%d peak-pixel=%d size=%d\n",
                buffer.wave.currentName(),
                buffer.waveScale.currentName(),
                buffer.flame.currentName(),
                buffer.table.currentName(),
                nonzero, peak, BUFF_SIZE);
        }

        buffer.swapBuffers();
    }
};

class FlashlightVisualModule : public VisualModule {
public:
    FlashlightVisualModule() { }

    void execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
        CTH_TRACE("executing flashlight stage\n", "visual pipeline");
        apply_flashlight(buffer, context);
    }
};

class BorderVisualModule : public VisualModule {
    int borderMode;

public:
    BorderVisualModule()
        : borderMode(0) { }

    void setBorderMode(int borderMode_) {
        borderMode = borderMode_;
    }

    void execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
        CTH_TRACE("executing border stage mode=%d\n", "visual pipeline", borderMode);
        apply_border(buffer, context, borderMode);
    }
};

class PaletteStageModule : public VisualModule {
public:
    PaletteStageModule() { }

    void execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
        (void)context;

        int selectedPalette = buffer.palette.currentN();
        if ((buffer.lastPalette == selectedPalette) && (buffer.palChanged == 0))
            return;

        Palette* desiredPalette = &(((PaletteEntry*)buffer.palette.current())->pal);

        if (buffer.lastPalette != selectedPalette) {
            buffer.lastPalette = selectedPalette;
            if (((double)rand() / ((double)RAND_MAX + 1.0)) >= paletteSmoothingChance) {
                // Skip smoothing, jump directly to the new palette (DOS behaviour).
                memcpy(buffer.currentPalette, *desiredPalette, sizeof(Palette));
                buffer.palChanged = 1;
                return;
            }
        }

        const int PALETTE_CHANNEL_RANGE = 256;
        const int PALETTE_SMOOTH_SECONDS = 2;

        static int oldMaxChange = 1;
        int maxChange = (cthughaDisplay->fps > 0)
            ? max(int((double)(PALETTE_CHANNEL_RANGE / PALETTE_SMOOTH_SECONDS) / cthughaDisplay->fps), 1)
            : oldMaxChange;
        oldMaxChange = maxChange;

        buffer.palChanged = 256 * 3;
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 3; j++) {
                int d = (*desiredPalette)[i][j] - buffer.currentPalette[i][j];
                if (d == 0)
                    buffer.palChanged--;
                else {
                    if (d < -maxChange)
                        d = -maxChange;
                    else if (d > maxChange)
                        d = maxChange;
                    buffer.currentPalette[i][j] += d;
                }
            }
        }
    }
};

VisualPlan::VisualPlan() { }

void VisualPlan::append(Stage stage) {
    sequenceValue.push_back(stage);
}

int VisualPlan::includes(Stage stage) const {
    for (unsigned int i = 0; i < sequenceValue.size(); i++) {
        if (sequenceValue[i] == stage)
            return 1;
    }

    return 0;
}

VisualDirector::VisualDirector()
    : lastPcxSelection(-1) { }

VisualPlan VisualDirector::planDefaultPipeline() const {
    VisualPlan plan;

    plan.append(VisualPlan::ImageStage);
    plan.append(VisualPlan::FlashlightStage);
    plan.append(VisualPlan::BorderStage);
    plan.append(VisualPlan::FlameStage);
    plan.append(VisualPlan::TranslateStage);
    plan.append(VisualPlan::WaveStage);
    plan.append(VisualPlan::FrameCommitStage);
    plan.append(VisualPlan::PaletteStage);

    CTH_TRACE("planned default stages=%d\n", "visual director", int(plan.sequence().size()));
    return plan;
}

int VisualDirector::pcxSelectionChanged() {
    CthughaBuffer* buffer = currentBuffer();
    if (buffer == 0)
        return 0;

    int selectedPcx = buffer->pcx.currentN();
    if (lastPcxSelection == selectedPcx)
        return 0;

    lastPcxSelection = selectedPcx;
    return 1;
}

void VisualDirector::syncCurrentBuffer() {
    CthughaBuffer::current = &CthughaBuffer::buffer;
}

void VisualDirector::updatePipelineStages(VisualPipeline& pipeline, CthughaBuffer& buffer) {
    ImageStageModule* imageModule
        = stageModule<ImageStageModule>(pipeline, VisualPlan::ImageStage);
    if (imageModule != 0)
        imageModule->setImage(static_cast<PCXEntry*>(buffer.pcx.current()));

    FlameStageModule* flameModule
        = stageModule<FlameStageModule>(pipeline, VisualPlan::FlameStage);
    if (flameModule != 0)
        flameModule->setFlame(flameFromOption(buffer.flame));

    TranslateStageModule* translateModule
        = stageModule<TranslateStageModule>(pipeline, VisualPlan::TranslateStage);
    if (translateModule != 0)
        translateModule->setTranslateProvider(&buffer.translate);

    WaveStageModule* waveModule
        = stageModule<WaveStageModule>(pipeline, VisualPlan::WaveStage);
    if (waveModule != 0)
        waveModule->setWave(static_cast<WaveEntry*>(buffer.wave.current()));

    BorderVisualModule* borderModule
        = stageModule<BorderVisualModule>(pipeline, VisualPlan::BorderStage);
    if (borderModule != 0)
        borderModule->setBorderMode(int(border));
}

CthughaBuffer* VisualDirector::configurePipeline(VisualPipeline& pipeline) {
    syncCurrentBuffer();
    CthughaBuffer* buffer = currentBuffer();
    if (buffer == 0)
        return 0;

    updatePipelineStages(pipeline, *buffer);

    if (pcxSelectionChanged())
        pipeline.setStageMode(VisualPlan::ImageStage, VisualStageArmedOnce);
    pipeline.setStageMode(VisualPlan::FlashlightStage,
        (int(flashlight) != 0) ? VisualStageEnabled : VisualStageDisabled);
    pipeline.setStageMode(VisualPlan::BorderStage, VisualStageEnabled);
    pipeline.setStageMode(VisualPlan::FlameStage, VisualStageEnabled);
    pipeline.setStageMode(VisualPlan::TranslateStage, VisualStageEnabled);
    pipeline.setStageMode(VisualPlan::WaveStage, VisualStageEnabled);
    pipeline.setStageMode(VisualPlan::FrameCommitStage, VisualStageEnabled);
    pipeline.setStageMode(VisualPlan::PaletteStage, VisualStageEnabled);

    return buffer;
}

VisualPipelineFactory::VisualPipelineFactory() { }

VisualPipeline* VisualPipelineFactory::create(const VisualPlan& plan) const {
    VisualPipeline* pipeline = new VisualPipeline();

    pipeline->setStageSequence(plan.sequence());

    if (plan.includes(VisualPlan::ImageStage))
        pipeline->add(VisualPlan::ImageStage, new ImageStageModule(), 1);
    if (plan.includes(VisualPlan::FlashlightStage))
        pipeline->add(VisualPlan::FlashlightStage, new FlashlightVisualModule(), 1);
    if (plan.includes(VisualPlan::BorderStage))
        pipeline->add(VisualPlan::BorderStage, new BorderVisualModule(), 1);
    if (plan.includes(VisualPlan::FlameStage))
        pipeline->add(VisualPlan::FlameStage, new FlameStageModule(), 1);
    if (plan.includes(VisualPlan::TranslateStage))
        pipeline->add(VisualPlan::TranslateStage, new TranslateStageModule(), 1);
    if (plan.includes(VisualPlan::WaveStage))
        pipeline->add(VisualPlan::WaveStage, new WaveStageModule(), 1);
    if (plan.includes(VisualPlan::FrameCommitStage))
        pipeline->add(VisualPlan::FrameCommitStage, new FrameCommitModule(), 1);
    if (plan.includes(VisualPlan::PaletteStage))
        pipeline->add(VisualPlan::PaletteStage, new PaletteStageModule(), 1);

    CTH_TRACE("created pipeline=%p stages=%d modules=%d\n", "visual pipeline factory",
        pipeline, int(plan.sequence().size()), pipeline->size());
    return pipeline;
}

void VisualPipelineFactory::refresh(VisualPipeline& pipeline, const VisualPlan& plan) const {
    CTH_TRACE("refreshing pipeline=%p stages=%d modules=%d\n", "visual pipeline factory",
        &pipeline, int(plan.sequence().size()), pipeline.size());
    pipeline.refresh();
}
