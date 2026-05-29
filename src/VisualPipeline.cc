#include "cthugha.h"
#include "CthughaFrameBuffer.h"
#include "VisualPipeline.h"

VisualFrameContext::VisualFrameContext()
    : audioFrame(0)
    , audioAnalysis(0)
    , acousticContext(0)
    , now(0.0)
    , deltaT(0.0) { }

VisualModule::~VisualModule() { }

VisualPipeline::VisualPipeline() { }

VisualPipeline::~VisualPipeline() {
    clear();
}

void VisualPipeline::clear() {
    for (unsigned int i = 0; i < modules.size(); i++) {
        if (modules[i].owned)
            delete modules[i].module;
    }
    modules.clear();
}

void VisualPipeline::add(unsigned int stage, VisualModule* module, int takeOwnership) {
    if (module == 0)
        return;
    modules.push_back(Entry(stage, module, takeOwnership));
    CTH_TRACE("added stage=0x%x module=%p owned=%d mode=%d size=%d\n",
        "visual pipeline", stage, module, takeOwnership, int(VisualStageDisabled), size());
}

int VisualPipeline::setStageMode(unsigned int stage, VisualStageRunMode mode) {
    int matched = 0;

    for (unsigned int i = 0; i < modules.size(); i++) {
        if (modules[i].stage == stage) {
            matched++;
            if (modules[i].mode != mode)
                modules[i].mode = mode;
        }
    }

    CTH_TRACE("set stage=0x%x mode=%d entries=%d\n", "visual pipeline",
        stage, int(mode), matched);
    return matched;
}

VisualStageRunMode VisualPipeline::stageMode(unsigned int stage) const {
    for (unsigned int i = 0; i < modules.size(); i++) {
        if (modules[i].stage == stage)
            return modules[i].mode;
    }

    return VisualStageDisabled;
}

void VisualPipeline::refresh() {
    for (unsigned int i = 0; i < modules.size(); i++)
        modules[i].module->refresh();
}

void VisualPipeline::run(CthughaFrameBuffer& frameBuffer, const VisualFrameContext& context) {
    for (unsigned int i = 0; i < modules.size(); i++) {
        if (modules[i].mode == VisualStageDisabled) {
            CTH_TRACE("skipping disabled stage=0x%x module=%p\n",
                "visual pipeline", modules[i].stage, modules[i].module);
            continue;
        }

        modules[i].module->execute(frameBuffer, context);

        if (modules[i].mode == VisualStageArmedOnce) {
            CTH_TRACE("disarming one-shot stage=0x%x module=%p\n",
                "visual pipeline", modules[i].stage, modules[i].module);
            modules[i].mode = VisualStageDisabled;
        }
    }
}

int VisualPipeline::size() const {
    return int(modules.size());
}
