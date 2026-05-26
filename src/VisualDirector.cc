#include "cthugha.h"
#include "CthughaFrameBuffer.h"
#include "VisualDirector.h"

static VisualBufferTransformFn visualBufferTransform = 0;

class LegacyBufferTransformModule : public VisualModule {
public:
    void execute(CthughaFrameBuffer& frameBuffer, const VisualFrameContext& context) {
        (void)frameBuffer;
        (void)context;

        // This is the temporary coarse visual stage: all current internal
        // indexed-buffer mutation happens here, before DisplayDevice handoff.
        CTH_TRACE("executing legacy buffer transform\n", "visual pipeline");
        if (visualBufferTransform != 0)
            visualBufferTransform();
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

    CTH_TRACE("planned default stages=0x%x\n", "visual director", plan.stages());
    return plan;
}

VisualPipelineFactory::VisualPipelineFactory() { }

VisualPipeline* VisualPipelineFactory::create(const VisualPlan& plan) const {
    VisualPipeline* pipeline = new VisualPipeline();

    if (plan.includes(VisualPlan::BufferTransformStage))
        pipeline->add(new LegacyBufferTransformModule(), 1);

    CTH_TRACE("created pipeline=%p stages=0x%x modules=%d\n", "visual pipeline factory",
        pipeline, plan.stages(), pipeline->size());
    return pipeline;
}

void VisualPipelineFactory::refresh(VisualPipeline& pipeline, const VisualPlan& plan) const {
    CTH_TRACE("refreshing pipeline=%p stages=0x%x modules=%d\n", "visual pipeline factory",
        &pipeline, plan.stages(), pipeline.size());
    pipeline.refresh();
}
