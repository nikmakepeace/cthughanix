// Video filterchain policy.

#ifndef __VIDEO_DIRECTOR_H
#define __VIDEO_DIRECTOR_H

#include "Image.h"
#include "Scene.h"
#include "VideoFilterchainSequence.h"

class CthughaBuffer;
class VideoFilterchain;

class VideoDirector : public SceneObserver {
    ImageOption images;
    RandomLegalImagePlacementStrategy imagePlacementStrategy;
    Scene* scene;
    VideoFilterchain* filterchain;
    CthughaBuffer* buffer;
    unsigned int pendingSceneChanges;
    const IndexedImage* pendingImageCue;
    unsigned int pendingImageCueId;
    unsigned int appliedImageCueId;
    int imageLoadingEnabledValue;

    void applySceneToFilterchain(unsigned int changes);
    void applyPendingImageCue();

public:
    VideoDirector();
    ~VideoDirector();

    void bindScene(Scene& scene_);
    void unbindScene();

    ImageOption& imageOption();
    int imageLoadingEnabled() const;
    void setImageLoadingEnabled(int enabled);
    int loadImages();

    VideoFilterchainSequence defaultFilterchainSequence() const;
    CthughaBuffer* configureFilterchain(VideoFilterchain& filterchain);

    virtual void sceneChanged(Scene& scene, unsigned int changes);
    virtual void sceneCue(Scene& scene, const SceneCue& cue);
};

VideoDirector& videoDirector();

extern double paletteSmoothingChance;

#endif
