#include "cthugha.h"
#include "Border.h"
#include "CthughaBuffer.h"
#include "CthughaDisplay.h"
#include "Flashlight.h"
#include "Image.h"
#include "VideoFilters.h"
#include "VideoDirector.h"
#include "cth_buffer.h"
#include "display.h"
#include "imath.h"

double paletteSmoothingChance = 1.0;

template <class Filter>
static Filter* stageFilter(VideoFilterchain& filterchain, VideoFilterchainSequence::Stage stage) {
    return dynamic_cast<Filter*>(filterchain.stageFilter(stage));
}

static int paletteSmoothingFrameBudget() {
    const int PALETTE_SMOOTH_SECONDS = 2;
    int fps = 60;

    if (cthughaDisplay != 0 && cthughaDisplay->fps > 0)
        fps = cthughaDisplay->fps;

    return max(fps * PALETTE_SMOOTH_SECONDS, 1);
}

static int paletteChangeFrameBudget() {
    if (paletteSmoothingChance <= 0.0)
        return 0;

    if (((double)rand() / ((double)RAND_MAX + 1.0)) >= paletteSmoothingChance)
        return 0;

    return paletteSmoothingFrameBudget();
}

VideoDirector::VideoDirector()
    : images(0, "image")
    , imagePlacementStrategy()
    , scene(0)
    , filterchain(0)
    , buffer(0)
    , pendingSceneChanges(SceneAllChanged)
    , pendingImageCue(0)
    , pendingImageCueId(0)
    , appliedImageCueId(0)
    , imageLoadingEnabledValue(1) { }

VideoDirector::~VideoDirector() {
    if (scene != 0)
        scene->removeObserver(*this);
}

VideoDirector& videoDirector() {
    static VideoDirector director;
    return director;
}

ImageOption& VideoDirector::imageOption() {
    return images;
}

void VideoDirector::bindScene(Scene& scene_) {
    if (scene == &scene_)
        return;

    if (scene != 0)
        scene->removeObserver(*this);

    scene = &scene_;
    scene->addObserver(*this);
    pendingSceneChanges |= SceneAllChanged;
}

void VideoDirector::unbindScene() {
    if (scene != 0)
        scene->removeObserver(*this);

    scene = 0;
    filterchain = 0;
    buffer = 0;
    pendingSceneChanges = SceneAllChanged;
    pendingImageCue = 0;
    pendingImageCueId = 0;
    appliedImageCueId = 0;
}

int VideoDirector::imageLoadingEnabled() const {
    return imageLoadingEnabledValue;
}

void VideoDirector::setImageLoadingEnabled(int enabled) {
    imageLoadingEnabledValue = enabled ? 1 : 0;
}

int VideoDirector::loadImages() {
    if (!imageLoadingEnabledValue)
        return 0;

    CTH_INFO("  loading image files...\n");
    CthughaBuffer& targetBuffer = CthughaBuffer::buffer;
    int result = images.loadImages(targetBuffer.width(), targetBuffer.height());
    CTH_INFO("  number of loaded image files: %d\n", images.getNEntries());

    return result;
}

VideoFilterchainSequence VideoDirector::defaultFilterchainSequence() const {
    VideoFilterchainSequence sequence;

    sequence.append(VideoFilterchainSequence::ImageStage);
    sequence.append(VideoFilterchainSequence::BorderStage);
    sequence.append(VideoFilterchainSequence::FlameStage);
    sequence.append(VideoFilterchainSequence::TranslateStage);
    sequence.append(VideoFilterchainSequence::WaveStage);
    sequence.append(VideoFilterchainSequence::FrameCommitStage);
    sequence.append(VideoFilterchainSequence::PaletteStage);
    sequence.append(VideoFilterchainSequence::FlashlightStage);

    CTH_DEBUG("video director: default stage sequence stages=%d\n",
        int(sequence.sequence().size()));
    return sequence;
}

void VideoDirector::applySceneToFilterchain(unsigned int changes) {
    if (scene == 0 || filterchain == 0 || buffer == 0)
        return;

    const SceneSettings& settings = scene->settings();

    FlameFilter* flameFilter
        = stageFilter<FlameFilter>(*filterchain, VideoFilterchainSequence::FlameStage);
    if (flameFilter != 0) {
        flameFilter->setFlame(settings.flame);
        flameFilter->setGeneralFlame(settings.generalFlame);
    }

    TranslateFilter* translateFilter
        = stageFilter<TranslateFilter>(*filterchain, VideoFilterchainSequence::TranslateStage);
    if (translateFilter != 0 && (changes & SceneTranslationChanged))
        translateFilter->setTranslate(settings.translationTable);

    WaveFilter* waveFilter
        = stageFilter<WaveFilter>(*filterchain, VideoFilterchainSequence::WaveStage);
    if (waveFilter != 0)
        waveFilter->setWave(settings.wave, settings.waveConfig);

    BorderFilter* borderFilter
        = stageFilter<BorderFilter>(*filterchain, VideoFilterchainSequence::BorderStage);
    if (borderFilter != 0)
        borderFilter->setBorderMode(settings.borderMode);

    FrameCommitFilter* frameCommitFilter
        = stageFilter<FrameCommitFilter>(*filterchain, VideoFilterchainSequence::FrameCommitStage);
    if (frameCommitFilter != 0)
        frameCommitFilter->setSceneNames(settings.flameName, settings.waveName,
            settings.waveScaleName, settings.tableName);

    PaletteFilter* paletteFilter
        = stageFilter<PaletteFilter>(*filterchain, VideoFilterchainSequence::PaletteStage);
    if (paletteFilter != 0) {
        int frameBudget = paletteFilter->needsTarget(settings.palette)
            ? paletteChangeFrameBudget()
            : 0;
        paletteFilter->setTargetPalette(settings.palette, frameBudget,
            randomPaletteTransitionStrategy());
    }

    filterchain->setStageMode(VideoFilterchainSequence::FlashlightStage,
        settings.flashlightEnabled ? VideoFilterEnabled : VideoFilterDisabled);
    filterchain->setStageMode(VideoFilterchainSequence::BorderStage, VideoFilterEnabled);
    filterchain->setStageMode(VideoFilterchainSequence::FlameStage, VideoFilterEnabled);
    filterchain->setStageMode(VideoFilterchainSequence::TranslateStage, VideoFilterEnabled);
    filterchain->setStageMode(VideoFilterchainSequence::WaveStage, VideoFilterEnabled);
    filterchain->setStageMode(VideoFilterchainSequence::FrameCommitStage, VideoFilterEnabled);
    filterchain->setStageMode(VideoFilterchainSequence::PaletteStage, VideoFilterEnabled);
}

void VideoDirector::applyPendingImageCue() {
    if (filterchain == 0 || buffer == 0 || pendingImageCue == 0
        || pendingImageCueId == appliedImageCueId)
        return;

    ImageFilter* imageFilter
        = stageFilter<ImageFilter>(*filterchain, VideoFilterchainSequence::ImageStage);
    if (imageFilter == 0)
        return;

    imageFilter->setImage(pendingImageCue);
    imageFilter->setPlacement(imagePlacementStrategy.choose(*pendingImageCue,
        buffer->width(), buffer->height()));
    imageFilter->setOverlayPassiveBuffer(1);
    filterchain->setStageMode(VideoFilterchainSequence::ImageStage, VideoFilterArmedOnce);

    appliedImageCueId = pendingImageCueId;
    pendingImageCue = 0;
}

void VideoDirector::sceneChanged(Scene& scene_, unsigned int changes) {
    (void)scene_;

    pendingSceneChanges |= changes;
    if (filterchain != 0 && buffer != 0) {
        applySceneToFilterchain(pendingSceneChanges);
        pendingSceneChanges = SceneNoChange;
    }
}

void VideoDirector::sceneCue(Scene& scene_, const SceneCue& cue) {
    (void)scene_;

    if (cue.type == SceneCueInjectImage) {
        pendingImageCue = cue.image;
        pendingImageCueId = cue.id;
        applyPendingImageCue();
    }
}

CthughaBuffer* VideoDirector::configureFilterchain(VideoFilterchain& filterchain_) {
    CthughaBuffer* targetBuffer = &CthughaBuffer::buffer;

    if (filterchain != &filterchain_) {
        filterchain = &filterchain_;
        pendingSceneChanges |= SceneAllChanged;
    }
    if (buffer != targetBuffer) {
        buffer = targetBuffer;
        pendingSceneChanges |= SceneAllChanged;
    }

    if (pendingSceneChanges != SceneNoChange) {
        applySceneToFilterchain(pendingSceneChanges);
        pendingSceneChanges = SceneNoChange;
    }
    applyPendingImageCue();

    return targetBuffer;
}
