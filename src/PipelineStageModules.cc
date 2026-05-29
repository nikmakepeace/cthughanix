#include "cthugha.h"
#include "Border.h"
#include "CthughaBuffer.h"
#include "Flame.h"
#include "Flashlight.h"
#include "PipelineStageModules.h"
#include "cth_buffer.h"
#include "pcx.h"
#include "translate.h"
#include "waves.h"

ImageStageModule::ImageStageModule()
    : image(0) { }

void ImageStageModule::setImage(PCXEntry* image_) {
    image = image_;
}

void ImageStageModule::execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
    (void)context;

    CTH_TRACE("executing image stage\n", "visual pipeline");
    if (image != 0)
        image->overlay(buffer);
}

FlameStageModule::FlameStageModule()
    : flame(0)
    , generalFlame(0) { }

void FlameStageModule::setFlame(const Flame* flame_) {
    flame = flame_;
}

void FlameStageModule::setGeneralFlame(int generalFlame_) {
    generalFlame = generalFlame_;
}

void FlameStageModule::execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
    CTH_TRACE("executing flame stage\n", "visual pipeline");

    if (flame != 0)
        flame->execute(buffer, context, generalFlame);
}

TranslateStageModule::TranslateStageModule()
    : translate(0) { }

void TranslateStageModule::setTranslateProvider(TranslateOption* translate_) {
    translate = translate_;
}

void TranslateStageModule::execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
    CTH_TRACE("executing translate stage\n", "visual pipeline");
    TranslateEntry* entry = NULL;
    if (translate != 0
        && translate->prepareCurrentEntry(entry) == 0
        && entry != NULL)
        entry->execute(buffer, context);
}

WaveStageModule::WaveStageModule()
    : currentWave(0) { }

void WaveStageModule::setWave(Wave* wave_) {
    currentWave = wave_;
}

void WaveStageModule::execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
    CTH_TRACE("executing wave stage\n", "visual pipeline");
    if (currentWave != NULL)
        currentWave->execute(buffer, context);
}

FrameCommitModule::FrameCommitModule()
    : flameName("unknown") { }

void FrameCommitModule::setFlameName(const char* flameName_) {
    flameName = (flameName_ != 0) ? flameName_ : "unknown";
}

void FrameCommitModule::execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
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
            wave.currentName(),
            waveScale.currentName(),
            flameName,
            table.currentName(),
            nonzero, peak, BUFF_SIZE);
    }

    buffer.swapBuffers();
}

FlashlightVisualModule::FlashlightVisualModule() { }

void FlashlightVisualModule::execute(CthughaBuffer& buffer,
    const VisualFrameContext& context) {
    CTH_TRACE("executing flashlight stage\n", "visual pipeline");
    apply_flashlight(buffer, context);
}

BorderVisualModule::BorderVisualModule()
    : borderMode(0) { }

void BorderVisualModule::setBorderMode(int borderMode_) {
    borderMode = borderMode_;
}

void BorderVisualModule::execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
    CTH_TRACE("executing border stage mode=%d\n", "visual pipeline", borderMode);
    apply_border(buffer, context, borderMode);
}

PaletteStageModule::PaletteStageModule() { }

int PaletteStageModule::needsTarget(PaletteEntry* paletteEntry) const {
    return paletteEntry != 0 && !palette.hasTarget(paletteEntry->pal);
}

void PaletteStageModule::setPalette(PaletteEntry* paletteEntry, int frameBudget) {
    if (paletteEntry != 0)
        palette.achieve(paletteEntry->pal, frameBudget);
}

void PaletteStageModule::execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
    (void)context;

    CTH_TRACE("executing palette stage\n", "visual pipeline");
    palette.execute(buffer);
}
