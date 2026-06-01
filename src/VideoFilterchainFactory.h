#ifndef __VIDEO_FILTERCHAIN_FACTORY_H
#define __VIDEO_FILTERCHAIN_FACTORY_H

#include "VideoFilterchainSequence.h"

class VideoFilterchain;

class VideoFilterchainFactory {
public:
    VideoFilterchainFactory();

    VideoFilterchain* create(const VideoFilterchainSequence& sequence) const;
    void refresh(VideoFilterchain& filterchain, const VideoFilterchainSequence& sequence) const;
};

#endif
