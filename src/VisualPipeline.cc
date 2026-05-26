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

void VisualPipeline::add(VisualModule* module, int takeOwnership) {
    if (module == 0)
        return;
    modules.push_back(Entry(module, takeOwnership));
    CTH_TRACE("added module=%p owned=%d size=%d\n", "visual pipeline",
        module, takeOwnership, size());
}

void VisualPipeline::refresh() {
    for (unsigned int i = 0; i < modules.size(); i++)
        modules[i].module->refresh();
}

void VisualPipeline::run(CthughaFrameBuffer& frameBuffer, const VisualFrameContext& context) {
    for (unsigned int i = 0; i < modules.size(); i++)
        modules[i].module->execute(frameBuffer, context);
}

int VisualPipeline::size() const {
    return int(modules.size());
}
