// Per-frame inputs for the Frame Generator module.

#include "FrameGeneratorContext.h"

FrameGeneratorContext::FrameGeneratorContext()
    : videoFrameContextValue()
    , frameBudgetFramesPerSecondValue(60) { }

FrameGeneratorContext::FrameGeneratorContext(
    const VideoFrameContext& videoFrameContext, int frameBudgetFramesPerSecond)
    : videoFrameContextValue(videoFrameContext)
    , frameBudgetFramesPerSecondValue(
          frameBudgetFramesPerSecond > 0 ? frameBudgetFramesPerSecond : 60) { }

const VideoFrameContext& FrameGeneratorContext::videoFrameContext() const {
    return videoFrameContextValue;
}

int FrameGeneratorContext::frameBudgetFramesPerSecond() const {
    return frameBudgetFramesPerSecondValue;
}
