#include "cthugha.h"
#include "VideoFilterchain.h"

VideoFrameContext::VideoFrameContext()
    : audioFrame(0)
    , rawAudioData(0)
    , processedWaveData(0)
    , audioMetrics(0)
    , acousticContext(0)
    , now(0.0)
    , deltaT(0.0) { }

VideoFilter::~VideoFilter() { }

VideoFrame::VideoFrame(CthughaBuffer& buffer_, const VideoFrameContext& context_,
    FramePalette* framePalette_, IndexedFrame* indexedFrame_)
    : bufferValue(&buffer_)
    , contextValue(&context_)
    , framePaletteValue(framePalette_)
    , indexedFrameValue(indexedFrame_) { }

CthughaBuffer& VideoFrame::buffer() {
    return *bufferValue;
}

const VideoFrameContext& VideoFrame::context() const {
    return *contextValue;
}

FramePalette* VideoFrame::framePalette() {
    return framePaletteValue;
}

const FramePalette* VideoFrame::framePalette() const {
    return framePaletteValue;
}

void VideoFrame::publishIndexedFrame(const IndexedFrame& indexedFrame) {
    if (indexedFrameValue != 0)
        *indexedFrameValue = indexedFrame;
}

const IndexedFrame& VideoFrame::indexedFrame() const {
    return *indexedFrameValue;
}

static int findStageIndex(const std::vector<unsigned int>& sequence, unsigned int stage) {
    for (unsigned int i = 0; i < sequence.size(); i++) {
        if (sequence[i] == stage)
            return int(i);
    }

    return -1;
}

VideoFilterchain::VideoFilterchain()
    : framePaletteValue(0) { }

VideoFilterchain::~VideoFilterchain() {
    clear();
}

void VideoFilterchain::clear() {
    for (unsigned int i = 0; i < filters.size(); i++) {
        if (filters[i].owned)
            delete filters[i].filter;
    }
    filters.clear();
    sequence.clear();
    framePaletteValue = 0;
    indexedFrameValue = IndexedFrame();
}

void VideoFilterchain::add(unsigned int stage, VideoFilter* filter, int takeOwnership) {
    if (filter == 0)
        return;
    filters.push_back(Entry(stage, filter, takeOwnership));
    CTH_DEBUG("video filterchain: added stage=%u filter=%p owned=%d mode=%d size=%d\n",
        stage, filter, takeOwnership, int(VideoFilterDisabled), size());
}

void VideoFilterchain::setStageSequence(const std::vector<unsigned int>& stages) {
    sequence = stages;
    CTH_DEBUG("video filterchain: set sequence stages=%d\n", int(sequence.size()));
}

int VideoFilterchain::moveStageBefore(unsigned int stage, unsigned int beforeStage) {
    if (stage == beforeStage)
        return 1;

    int stageIndex = findStageIndex(sequence, stage);
    int beforeIndex = findStageIndex(sequence, beforeStage);
    if (stageIndex < 0 || beforeIndex < 0)
        return 0;

    unsigned int movingStage = sequence[stageIndex];
    sequence.erase(sequence.begin() + stageIndex);
    beforeIndex = findStageIndex(sequence, beforeStage);
    sequence.insert(sequence.begin() + beforeIndex, movingStage);

    CTH_DEBUG("video filterchain: moved stage=%u before stage=%u\n",
        stage, beforeStage);
    return 1;
}

int VideoFilterchain::moveStageAfter(unsigned int stage, unsigned int afterStage) {
    if (stage == afterStage)
        return 1;

    int stageIndex = findStageIndex(sequence, stage);
    int afterIndex = findStageIndex(sequence, afterStage);
    if (stageIndex < 0 || afterIndex < 0)
        return 0;

    unsigned int movingStage = sequence[stageIndex];
    sequence.erase(sequence.begin() + stageIndex);
    afterIndex = findStageIndex(sequence, afterStage);
    sequence.insert(sequence.begin() + afterIndex + 1, movingStage);

    CTH_DEBUG("video filterchain: moved stage=%u after stage=%u\n",
        stage, afterStage);
    return 1;
}

int VideoFilterchain::setStageMode(unsigned int stage, VideoFilterRunMode mode) {
    int matched = 0;

    for (unsigned int i = 0; i < filters.size(); i++) {
        if (filters[i].stage == stage) {
            matched++;
            if (filters[i].mode != mode)
                filters[i].mode = mode;
        }
    }

    CTH_TRACE("set stage=%u mode=%d entries=%d\n", "video filterchain",
        stage, int(mode), matched);
    return matched;
}

VideoFilterRunMode VideoFilterchain::stageMode(unsigned int stage) const {
    for (unsigned int i = 0; i < filters.size(); i++) {
        if (filters[i].stage == stage)
            return filters[i].mode;
    }

    return VideoFilterDisabled;
}

VideoFilter* VideoFilterchain::stageFilter(unsigned int stage) {
    for (unsigned int i = 0; i < filters.size(); i++) {
        if (filters[i].stage == stage)
            return filters[i].filter;
    }

    return 0;
}

void VideoFilterchain::setFramePalette(FramePalette* framePalette) {
    framePaletteValue = framePalette;
}

FramePalette* VideoFilterchain::framePalette() const {
    return framePaletteValue;
}

const IndexedFrame& VideoFilterchain::indexedFrame() const {
    return indexedFrameValue;
}

void VideoFilterchain::refresh() {
    for (unsigned int i = 0; i < filters.size(); i++)
        filters[i].filter->refresh();
}

void VideoFilterchain::run(CthughaBuffer& buffer, const VideoFrameContext& context) {
    indexedFrameValue = IndexedFrame();
    VideoFrame frame(buffer, context, framePaletteValue, &indexedFrameValue);

    for (unsigned int stageIndex = 0; stageIndex < sequence.size(); stageIndex++) {
        unsigned int stage = sequence[stageIndex];
        for (unsigned int filterIndex = 0; filterIndex < filters.size(); filterIndex++) {
            if (filters[filterIndex].stage != stage)
                continue;

            if (filters[filterIndex].mode == VideoFilterDisabled) {
                CTH_TRACE("skipping disabled stage=%u filter=%p\n",
                    "video filterchain", filters[filterIndex].stage, filters[filterIndex].filter);
                continue;
            }

            filters[filterIndex].filter->execute(frame);

            if (filters[filterIndex].mode == VideoFilterArmedOnce) {
                CTH_TRACE("disarming one-shot stage=%u filter=%p\n",
                    "video filterchain", filters[filterIndex].stage, filters[filterIndex].filter);
                filters[filterIndex].mode = VideoFilterDisabled;
            }
        }
    }
}

int VideoFilterchain::size() const {
    return int(filters.size());
}
