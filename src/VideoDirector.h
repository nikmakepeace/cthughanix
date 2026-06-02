// Video filterchain policy.

#ifndef __VIDEO_DIRECTOR_H
#define __VIDEO_DIRECTOR_H

#include "Image.h"
#include "Option.h"
#include "Scene.h"
#include "SilenceMessage.h"
#include "VideoFilterchainSequence.h"

#include <string>

class CthughaBuffer;
class VideoFilterchain;

class VideoDirector : public SceneObserver {
    ImageOption images;
    RandomLegalImagePlacementStrategy imagePlacementStrategy;
    SilenceMessage silenceMessage;
    Scene* scene;
    VideoFilterchain* filterchain;
    CthughaBuffer* buffer;
    unsigned int pendingSceneChanges;
    const IndexedImage* pendingImageCue;
    unsigned int pendingImageCueId;
    unsigned int appliedImageCueId;
    std::string pendingTextMessage;
    unsigned int pendingTextCueId;
    unsigned int appliedTextCueId;
    int pendingTextFrames;
    int pendingTextInkColor;
    int imageLoadingEnabledValue;

    void applySceneToFilterchain(unsigned int changes);
    void applyPendingImageCue();
    void applyPendingTextCue();

public:
    VideoDirector();
    ~VideoDirector();

    void bindScene(Scene& scene_);
    void unbindScene();

    ImageOption& imageOption();
    int imageLoadingEnabled() const;
    void setImageLoadingEnabled(int enabled);
    int loadImages();
    SilenceMessage& silenceMessages();
    int observeQuiet(int quietLength);

    VideoFilterchainSequence defaultFilterchainSequence() const;
    CthughaBuffer* configureFilterchain(VideoFilterchain& filterchain);

    virtual void sceneChanged(Scene& scene, unsigned int changes);
    virtual void sceneCue(Scene& scene, const SceneCue& cue);
};

VideoDirector& videoDirector();

extern OptionTime changeMsgTime; /* max. quiet interval before a text cue */
extern double paletteSmoothingChance;

#endif
