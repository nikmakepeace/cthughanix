// Internal video filterchain scaffold.

#ifndef __VIDEO_FILTERCHAIN_H
#define __VIDEO_FILTERCHAIN_H

#include "AudioFrame.h"
#include "AudioAnalyzer.h"
#include "IndexedFrame.h"

#include <vector>

class CthughaBuffer;
class FramePalette;

class VideoFrameContext {
public:
    const AudioFrame* audioFrame;
    const char2* rawAudioData;
    const char2* processedWaveData;
    const AudioMetrics* audioMetrics;
    const AcousticContext* acousticContext;
    double now;
    double deltaT;

    VideoFrameContext();
};

class VideoFrame {
    CthughaBuffer* bufferValue;
    const VideoFrameContext* contextValue;
    FramePalette* framePaletteValue;
    IndexedFrame* indexedFrameValue;

public:
    VideoFrame(CthughaBuffer& buffer_, const VideoFrameContext& context_,
        FramePalette* framePalette_, IndexedFrame* indexedFrame_);

    CthughaBuffer& buffer();
    const VideoFrameContext& context() const;
    FramePalette* framePalette();
    const FramePalette* framePalette() const;
    void publishIndexedFrame(const IndexedFrame& indexedFrame);
    const IndexedFrame& indexedFrame() const;
};

class VideoFilter {
public:
    virtual ~VideoFilter();

    virtual void refresh() { }
    virtual void execute(VideoFrame& frame) = 0;
};

enum VideoFilterRunMode {
    VideoFilterDisabled,
    VideoFilterEnabled,
    // Executes on the next run, then the filterchain changes it to Disabled.
    VideoFilterArmedOnce
};

class VideoFilterchain {
    struct Entry {
        unsigned int stage;
        VideoFilter* filter;
        int owned;
        VideoFilterRunMode mode;

        Entry(unsigned int stage_, VideoFilter* filter_, int owned_)
            : stage(stage_)
            , filter(filter_)
            , owned(owned_)
            , mode(VideoFilterDisabled) { }
    };

    std::vector<Entry> filters;
    std::vector<unsigned int> sequence;
    FramePalette* framePaletteValue;
    IndexedFrame indexedFrameValue;

public:
    VideoFilterchain();
    ~VideoFilterchain();

    void clear();
    void add(unsigned int stage, VideoFilter* filter, int takeOwnership = 0);
    void setStageSequence(const std::vector<unsigned int>& stages);
    int moveStageBefore(unsigned int stage, unsigned int beforeStage);
    int moveStageAfter(unsigned int stage, unsigned int afterStage);
    int setStageMode(unsigned int stage, VideoFilterRunMode mode);
    VideoFilterRunMode stageMode(unsigned int stage) const;
    VideoFilter* stageFilter(unsigned int stage);
    void setFramePalette(FramePalette* framePalette);
    FramePalette* framePalette() const;
    const IndexedFrame& indexedFrame() const;
    void refresh();
    void run(CthughaBuffer& buffer, const VideoFrameContext& context);
    int size() const;
};

#endif
