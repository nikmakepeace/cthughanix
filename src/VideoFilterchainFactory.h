#ifndef __VIDEO_FILTERCHAIN_FACTORY_H
#define __VIDEO_FILTERCHAIN_FACTORY_H

#include "VideoFilterchainSequence.h"

class VideoFilterchain;

/**
 * Builds concrete video filterchains from stage sequences.
 */
class VideoFilterchainFactory {
public:
    VideoFilterchainFactory();

    /**
     * Allocates and populates a filterchain for a sequence.
     *
     * @param sequence Stage order and set of filters to install.
     * @return Newly allocated filterchain owned by the caller.
     */
    VideoFilterchain* create(const VideoFilterchainSequence& sequence) const;

    /**
     * Refreshes filters after display or scene configuration changes.
     *
     * @param filterchain Existing filterchain to refresh.
     * @param sequence Stage sequence used for diagnostic context.
     */
    void refresh(VideoFilterchain& filterchain, const VideoFilterchainSequence& sequence) const;
};

#endif
