#include "cthugha.h"
#include "Border.h"
#include "CthughaBuffer.h"
#include "CthughaFrameBuffer.h"
#include "CthughaDisplay.h"
#include "Flashlight.h"
#include "VisualDirector.h"
#include "cth_buffer.h"
#include "imath.h"

static VisualBufferTransformFn visualBufferTransform = 0;
double paletteSmoothingChance = 1.0;

class NullVisualStageModule : public VisualModule {
    const char* stageName;

public:
    NullVisualStageModule(const char* stageName_)
        : stageName(stageName_) { }

    void execute(CthughaFrameBuffer& frameBuffer, const VisualFrameContext& context) {
        (void)frameBuffer;
        (void)context;

        CTH_TRACE("executing null stage=%s\n", "visual pipeline", stageName);
    }
};

class LegacyBufferTransformModule : public VisualModule {
public:
    void execute(CthughaFrameBuffer& frameBuffer, const VisualFrameContext& context) {
        (void)frameBuffer;
        (void)context;

        // This is the temporary coarse visual stage: all current internal
        // indexed-buffer mutation happens here, before DisplayDevice handoff.
        CTH_TRACE("executing classic buffer transform\n", "visual pipeline");
        if (visualBufferTransform != 0)
            visualBufferTransform();
        if (CthughaBuffer::current != 0)
            frameBuffer.bind(CthughaBuffer::current->activeBuffer,
                CthughaBuffer::current->passiveBuffer, BUFF_WIDTH, BUFF_HEIGHT, BUFF_WIDTH,
                &CthughaBuffer::current->currentPalette, &CthughaBuffer::current->palChanged,
                &CthughaBuffer::current->palette, &CthughaBuffer::current->lastPalette);
    }
};

class FlashlightVisualModule : public VisualModule {
public:
    void execute(CthughaFrameBuffer& frameBuffer, const VisualFrameContext& context) {
        CTH_TRACE("executing flashlight stage\n", "visual pipeline");
        apply_flashlight(frameBuffer, context);
    }
};

class BorderVisualModule : public VisualModule {
public:
    void execute(CthughaFrameBuffer& frameBuffer, const VisualFrameContext& context) {
        CTH_TRACE("executing border stage mode=%d\n", "visual pipeline", int(border));
        apply_border(frameBuffer, context);
    }
};

class PaletteStageModule : public VisualModule {
public:
    void execute(CthughaFrameBuffer& frameBuffer, const VisualFrameContext& context) {
        (void)context;

        CoreOption* paletteOption = frameBuffer.paletteOption();
        Palette* currentPalette = frameBuffer.palette();
        int* lastPalette = frameBuffer.lastPalette();

        if (paletteOption == 0 || currentPalette == 0 || lastPalette == 0)
            return;

        int selectedPalette = paletteOption->currentN();
        if ((*lastPalette == selectedPalette) && (frameBuffer.paletteChanged() == 0))
            return;

        Palette* desiredPalette = &(((PaletteEntry*)paletteOption->current())->pal);

        if (*lastPalette != selectedPalette) {
            *lastPalette = selectedPalette;
            if (((double)rand() / ((double)RAND_MAX + 1.0)) >= paletteSmoothingChance) {
                // Skip smoothing, jump directly to the new palette (DOS behaviour).
                frameBuffer.setPalette(*desiredPalette);
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

        frameBuffer.setPaletteChanged(256 * 3);
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 3; j++) {
                int d = (*desiredPalette)[i][j] - (*currentPalette)[i][j];
                if (d == 0)
                    frameBuffer.setPaletteChanged(frameBuffer.paletteChanged() - 1);
                else {
                    if (d < -maxChange)
                        d = -maxChange;
                    else if (d > maxChange)
                        d = maxChange;
                    (*currentPalette)[i][j] += d;
                }
            }
        }
    }
};

VisualPlan::VisualPlan()
    : stagesValue(0) { }

void VisualPlan::include(Stage stage) {
    stagesValue |= stage;
}

int VisualPlan::includes(Stage stage) const {
    return (stagesValue & stage) != 0;
}

void setVisualBufferTransform(VisualBufferTransformFn transform) {
    visualBufferTransform = transform;
    CTH_TRACE("set buffer transform=%p\n", "visual director", (void*)visualBufferTransform);
}

VisualPlan VisualDirector::planDefaultPipeline() const {
    VisualPlan plan;

    plan.include(VisualPlan::BufferTransformStage);
    plan.include(VisualPlan::FlashlightStage);
    plan.include(VisualPlan::BorderStage);
    plan.include(VisualPlan::ImageStage);
    plan.include(VisualPlan::FlameStage);
    plan.include(VisualPlan::TranslateStage);
    plan.include(VisualPlan::WaveStage);
    plan.include(VisualPlan::PaletteStage);

    CTH_TRACE("planned default stages=0x%x\n", "visual director", plan.stages());
    return plan;
}

VisualPipelineFactory::VisualPipelineFactory() { }

VisualPipeline* VisualPipelineFactory::create(const VisualPlan& plan) const {
    VisualPipeline* pipeline = new VisualPipeline();

    if (plan.includes(VisualPlan::FlashlightStage))
        pipeline->add(new FlashlightVisualModule(), 1);
    if (plan.includes(VisualPlan::BorderStage))
        pipeline->add(new BorderVisualModule(), 1);
    if (plan.includes(VisualPlan::BufferTransformStage))
        pipeline->add(new LegacyBufferTransformModule(), 1);
    if (plan.includes(VisualPlan::ImageStage))
        pipeline->add(new NullVisualStageModule("image"), 1);
    if (plan.includes(VisualPlan::FlameStage))
        pipeline->add(new NullVisualStageModule("flame"), 1);
    if (plan.includes(VisualPlan::TranslateStage))
        pipeline->add(new NullVisualStageModule("translate"), 1);
    if (plan.includes(VisualPlan::WaveStage))
        pipeline->add(new NullVisualStageModule("wave"), 1);
    if (plan.includes(VisualPlan::PaletteStage))
        pipeline->add(new PaletteStageModule(), 1);

    CTH_TRACE("created pipeline=%p stages=0x%x modules=%d\n", "visual pipeline factory",
        pipeline, plan.stages(), pipeline->size());
    return pipeline;
}

void VisualPipelineFactory::refresh(VisualPipeline& pipeline, const VisualPlan& plan) const {
    CTH_TRACE("refreshing pipeline=%p stages=0x%x modules=%d\n", "visual pipeline factory",
        &pipeline, plan.stages(), pipeline.size());
    pipeline.refresh();
}
