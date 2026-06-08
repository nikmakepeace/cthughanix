// Frame Generator filterchain ownership.

#include "FrameFilterchainPipeline.h"

#include "FramePalette.h"
#include "IndexedFrame.h"
#include "VideoFilterchain.h"
#include "VideoFilterchainSequence.h"
#include "VideoFilters.h"

FrameFilterchainPipeline::FrameFilterchainPipeline()
    : factoryValue()
    , filterchainValue()
    , sequenceValue() { }

FrameFilterchainPipeline::~FrameFilterchainPipeline() { }

void FrameFilterchainPipeline::initialize(
    const VideoFilterchainSequence& sequence) {
    if (filterchainValue.get() != 0)
        return;

    sequenceValue = sequence;
    filterchainValue.reset(factoryValue.create(sequenceValue));
}

void FrameFilterchainPipeline::reset() {
    filterchainValue.reset();
    sequenceValue = VideoFilterchainSequence();
}

VideoFilterchain& FrameFilterchainPipeline::filterchain() {
    return *filterchainValue;
}

FramePalette* FrameFilterchainPipeline::framePalette() const {
    if (filterchainValue.get() == 0)
        return 0;

    return framePaletteFromFilterchain(*filterchainValue);
}

const IndexedFrame& FrameFilterchainPipeline::render(CthughaBuffer& buffer,
    const VideoFrameContext& context) {
    filterchainValue->run(buffer, context);
    return filterchainValue->indexedFrame();
}
