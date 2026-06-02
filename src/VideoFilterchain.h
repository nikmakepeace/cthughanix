// Internal video filterchain scaffold.

#ifndef __VIDEO_FILTERCHAIN_H
#define __VIDEO_FILTERCHAIN_H

#include "AudioFrame.h"
#include "AudioAnalyzer.h"
#include "IndexedFrame.h"

#include <vector>

class CthughaBuffer;
class FramePalette;

/**
 * Per-frame inputs shared by every video filter.
 *
 * The pointers are borrowed for the duration of one filterchain run. Time values
 * are in seconds and match the visual frame clock, not the audio sample clock.
 */
class VideoFrameContext {
public:
    /** Audio frame facade for this visual frame, or NULL when audio is absent. */
    const AudioFrame* audioFrame;

    /** Raw signed 8-bit stereo audio samples, usually audioFrame->raw. */
    const char2* rawAudioData;

    /** Processed signed 8-bit stereo samples after the selected audio processor. */
    const char2* processedWaveData;

    /** Analysis metrics derived from the current audio frame, or NULL. */
    const AudioMetrics* audioMetrics;

    /** Higher-level acoustic state used by sound-reactive filters, or NULL. */
    const AcousticContext* acousticContext;

    /** Current visual-frame timestamp, in seconds. */
    double now;

    /** Seconds elapsed since the previous visual frame. */
    double deltaT;

    VideoFrameContext();
};

/**
 * Mutable frame object passed through one filterchain execution.
 *
 * Filters use this to access the active/passive indexed buffers, frame context,
 * optional frame palette, and final published IndexedFrame.
 */
class VideoFrame {
    CthughaBuffer* bufferValue;
    const VideoFrameContext* contextValue;
    FramePalette* framePaletteValue;
    IndexedFrame* indexedFrameValue;

public:
    /**
     * Wraps the state for one video filterchain run.
     *
     * @param buffer_ Active/passive indexed pixel buffer for this visual frame.
     * @param context_ Borrowed per-frame audio/time context.
     * @param framePalette_ Palette state to update or read, or NULL.
     * @param indexedFrame_ Destination for final display frame publication.
     */
    VideoFrame(CthughaBuffer& buffer_, const VideoFrameContext& context_,
        FramePalette* framePalette_, IndexedFrame* indexedFrame_);

    /** @return Active/passive indexed pixel buffer for this frame. */
    CthughaBuffer& buffer();

    /** @return Borrowed audio/time context for this frame. */
    const VideoFrameContext& context() const;

    /** @return Mutable frame palette, or NULL when no palette stage is installed. */
    FramePalette* framePalette();

    /** @return Frame palette, or NULL when no palette stage is installed. */
    const FramePalette* framePalette() const;

    /**
     * Publishes the final indexed frame descriptor for display.
     *
     * @param indexedFrame Descriptor containing pixels, dimensions, pitch, and palette.
     */
    void publishIndexedFrame(const IndexedFrame& indexedFrame);

    /** @return Last IndexedFrame published during this filterchain run. */
    const IndexedFrame& indexedFrame() const;
};

/**
 * Base interface for one video filter stage.
 */
class VideoFilter {
public:
    virtual ~VideoFilter();

    /** Rebuilds internal lookup/cache state after display or scene changes. */
    virtual void refresh() { }

    /**
     * Runs this filter for one visual frame.
     *
     * @param frame Mutable frame wrapper containing buffers, palette, and context.
     */
    virtual void execute(VideoFrame& frame) = 0;
};

enum VideoFilterRunMode {
    /** Stage is skipped during filterchain runs. */
    VideoFilterDisabled,

    /** Stage executes on every filterchain run. */
    VideoFilterEnabled,

    /** Stage executes on the next run, then changes back to VideoFilterDisabled. */
    VideoFilterArmedOnce
};

/**
 * Ordered collection of video filters keyed by stage id.
 *
 * Stage ids are normally VideoFilterchainSequence::Stage values. A stage can
 * contain more than one filter, and run modes are applied to every filter with
 * the matching stage id.
 */
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

    /** Deletes owned filters and clears stage order, palette, and published frame. */
    void clear();

    /**
     * Adds a filter to a stage.
     *
     * @param stage Stage id, normally a VideoFilterchainSequence::Stage value.
     * @param filter Filter instance to add; ignored when NULL.
     * @param takeOwnership Nonzero to delete filter in clear()/destructor.
     */
    void add(unsigned int stage, VideoFilter* filter, int takeOwnership = 0);

    /**
     * Replaces the stage execution order.
     *
     * @param stages Ordered stage ids to visit each run.
     */
    void setStageSequence(const std::vector<unsigned int>& stages);

    /**
     * Moves one stage before another in the current sequence.
     *
     * @param stage Stage id to move.
     * @param beforeStage Existing stage id to move before.
     * @return Nonzero on success, zero if either stage is absent.
     */
    int moveStageBefore(unsigned int stage, unsigned int beforeStage);

    /**
     * Moves one stage after another in the current sequence.
     *
     * @param stage Stage id to move.
     * @param afterStage Existing stage id to move after.
     * @return Nonzero on success, zero if either stage is absent.
     */
    int moveStageAfter(unsigned int stage, unsigned int afterStage);

    /**
     * Sets the run mode for every filter registered under a stage.
     *
     * @param stage Stage id to update.
     * @param mode Disabled, enabled, or armed-once execution mode.
     * @return Number of filters matched by the stage id.
     */
    int setStageMode(unsigned int stage, VideoFilterRunMode mode);

    /**
     * @param stage Stage id to query.
     * @return First matching filter's run mode, or VideoFilterDisabled.
     */
    VideoFilterRunMode stageMode(unsigned int stage) const;

    /**
     * @param stage Stage id to query.
     * @return First matching filter for the stage, or NULL.
     */
    VideoFilter* stageFilter(unsigned int stage);

    /**
     * Sets the shared frame palette pointer supplied to VideoFrame.
     *
     * @param framePalette Palette owned by a filter or caller; not owned here.
     */
    void setFramePalette(FramePalette* framePalette);

    /** @return Shared frame palette pointer, or NULL. */
    FramePalette* framePalette() const;

    /** @return Most recent final indexed frame descriptor published by filters. */
    const IndexedFrame& indexedFrame() const;

    /** Calls refresh() on every registered filter. */
    void refresh();

    /**
     * Executes enabled filters in configured stage order for one visual frame.
     *
     * @param buffer Active/passive indexed pixel buffer to mutate.
     * @param context Per-frame audio/time context; borrowed during the call.
     */
    void run(CthughaBuffer& buffer, const VideoFrameContext& context);

    /** @return Number of registered filter entries, not number of stages. */
    int size() const;
};

#endif
