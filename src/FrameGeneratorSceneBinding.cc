// Scene binding for the Frame Generator module.

#include "FrameGeneratorSceneBinding.h"

#include "Border.h"
#include "Configuration.h"
#include "Flashlight.h"
#include "FramePalette.h"
#include "PaletteTransition.h"
#include "PaletteEntry.h"
#include "ProcessServices.h"
#include "VideoFilters.h"

template <class Filter>
static Filter* stageFilter(VideoFilterchain& filterchain,
    VideoFilterchainSequence::Stage stage) {
    return dynamic_cast<Filter*>(filterchain.stageFilter(stage));
}

static const ColorPalette* scenePaletteColors(const SceneSettings& settings) {
    return (settings.palette != 0) ? &settings.palette->colors() : 0;
}

FrameGeneratorSceneBinding::FrameGeneratorSceneBinding(
    const FrameGeometry& geometry, FrameTransitionController& transitionController_,
    RandomSource& randomSource, CountdownTimerFactory& timerFactory)
    : geometryValue(geometry)
    , transitionController(transitionController_)
    , images(0, "image")
    , imagePlacementStrategy()
    , randomSourceValue(randomSource)
    , silenceMessage()
    , scene(0)
    , filterchain(0)
    , pendingSceneChanges(SceneAllChanged)
    , pendingImageCue(0)
    , pendingImageCueId(0)
    , appliedImageCueId(0)
    , pendingTextMessage()
    , pendingTextCueId(0)
    , appliedTextCueId(0)
    , pendingTextFrames(0)
    , pendingTextInkColor(-1) {
    silenceMessage.setRandomSource(randomSourceValue);
    silenceMessage.setTimerFactory(timerFactory);
}

FrameGeneratorSceneBinding::~FrameGeneratorSceneBinding() {
    unbindScene();
}

ImageOption& FrameGeneratorSceneBinding::imageOption() {
    return images;
}

SilenceMessage& FrameGeneratorSceneBinding::silenceMessages() {
    return silenceMessage;
}

void FrameGeneratorSceneBinding::bindScene(Scene& scene_) {
    if (scene == &scene_)
        return;

    if (scene != 0)
        scene->removeObserver(*this);

    scene = &scene_;
    scene->addObserver(*this);
    pendingSceneChanges |= SceneAllChanged;
}

void FrameGeneratorSceneBinding::unbindScene() {
    if (scene != 0)
        scene->removeObserver(*this);

    scene = 0;
    filterchain = 0;
    pendingSceneChanges = SceneAllChanged;
    pendingImageCue = 0;
    pendingImageCueId = 0;
    appliedImageCueId = 0;
    pendingTextMessage.clear();
    pendingTextCueId = 0;
    appliedTextCueId = 0;
    pendingTextFrames = 0;
    pendingTextInkColor = -1;
}

int FrameGeneratorSceneBinding::observeQuiet(int quietLength,
    int quietMessageThresholdMs, int frameBudgetFramesPerSecond) {
    if (!quietMessageThresholdMs || quietLength <= quietMessageThresholdMs)
        return 0;

    if (scene == 0)
        return 0;

    std::string message = silenceMessage.nextMessage();
    scene->emitTextCue(message.c_str(),
        transitionController.quietMessageFrameBudget(frameBudgetFramesPerSecond,
            quietMessageThresholdMs),
        -1);
    return 1;
}

VideoFilterchainSequence FrameGeneratorSceneBinding::defaultFilterchainSequence() const {
    VideoFilterchainSequence sequence;

    sequence.append(VideoFilterchainSequence::ImageStage);
    sequence.append(VideoFilterchainSequence::BorderStage);
    sequence.append(VideoFilterchainSequence::FlameStage);
    sequence.append(VideoFilterchainSequence::TranslateStage);
    sequence.append(VideoFilterchainSequence::WaveStage);
    sequence.append(VideoFilterchainSequence::TextStage);
    sequence.append(VideoFilterchainSequence::FrameCommitStage);
    sequence.append(VideoFilterchainSequence::PaletteStage);
    sequence.append(VideoFilterchainSequence::FlashlightStage);
    sequence.append(VideoFilterchainSequence::IndexedFrameStage);

    CTH_DEBUG("frame generator: default stage sequence stages=%d\n",
        int(sequence.sequence().size()));
    return sequence;
}

void FrameGeneratorSceneBinding::applySceneToFilterchain(unsigned int changes,
    int frameBudgetFramesPerSecond) {
    if (scene == 0 || filterchain == 0)
        return;

    const SceneSettings& settings = scene->settings();

    FlameFilter* flameFilter
        = stageFilter<FlameFilter>(*filterchain, VideoFilterchainSequence::FlameStage);
    if (flameFilter != 0) {
        flameFilter->setFlame(settings.flame);
        flameFilter->setGeneralFlame(settings.generalFlame);
    }

    TranslateFilter* translateFilter
        = stageFilter<TranslateFilter>(*filterchain,
            VideoFilterchainSequence::TranslateStage);
    if (translateFilter != 0 && (changes & SceneTranslationChanged))
        translateFilter->setTranslate(settings.translationTable);

    WaveFilter* waveFilter
        = stageFilter<WaveFilter>(*filterchain, VideoFilterchainSequence::WaveStage);
    if (waveFilter != 0) {
        waveFilter->setRandomSource(randomSourceValue);
        waveFilter->setWave(settings.wave, settings.waveConfig);
    }

    BorderFilter* borderFilter
        = stageFilter<BorderFilter>(*filterchain, VideoFilterchainSequence::BorderStage);
    if (borderFilter != 0)
        borderFilter->setBorderMode(settings.borderMode);

    FrameCommitFilter* frameCommitFilter
        = stageFilter<FrameCommitFilter>(*filterchain,
            VideoFilterchainSequence::FrameCommitStage);
    if (frameCommitFilter != 0)
        frameCommitFilter->setSceneNames(settings.flameName, settings.waveName,
            settings.waveScaleName, settings.tableName);

    if (changes & ScenePaletteChanged) {
        PaletteFilter* paletteFilter
            = stageFilter<PaletteFilter>(*filterchain,
                VideoFilterchainSequence::PaletteStage);
        if (paletteFilter != 0) {
            int frameBudget = paletteFilter->needsTarget(settings.palette)
                ? transitionController.paletteChangeFrameBudget(randomSourceValue,
                    frameBudgetFramesPerSecond)
                : 0;
            paletteFilter->setTargetPalette(settings.palette, frameBudget,
                randomPaletteTransitionStrategy(randomSourceValue));
        }
    }

    filterchain->setStageMode(VideoFilterchainSequence::FlashlightStage,
        settings.flashlightEnabled ? VideoFilterEnabled : VideoFilterDisabled);
    filterchain->setStageMode(VideoFilterchainSequence::BorderStage,
        VideoFilterEnabled);
    filterchain->setStageMode(VideoFilterchainSequence::FlameStage,
        VideoFilterEnabled);
    filterchain->setStageMode(VideoFilterchainSequence::TranslateStage,
        VideoFilterEnabled);
    filterchain->setStageMode(VideoFilterchainSequence::WaveStage,
        VideoFilterEnabled);
    filterchain->setStageMode(VideoFilterchainSequence::FrameCommitStage,
        VideoFilterEnabled);
    filterchain->setStageMode(VideoFilterchainSequence::PaletteStage,
        VideoFilterEnabled);
    filterchain->setStageMode(VideoFilterchainSequence::IndexedFrameStage,
        VideoFilterEnabled);
}

void FrameGeneratorSceneBinding::applyPendingImageCue(
    int frameBudgetFramesPerSecond) {
    if (filterchain == 0 || pendingImageCue == 0
        || pendingImageCueId == appliedImageCueId)
        return;

    ImageFilter* imageFilter
        = stageFilter<ImageFilter>(*filterchain, VideoFilterchainSequence::ImageStage);
    if (imageFilter == 0)
        return;

    imageFilter->setImage(pendingImageCue);
    imageFilter->setPlacement(imagePlacementStrategy.choose(*pendingImageCue,
        geometryValue.width(), geometryValue.height(), randomSourceValue));
    imageFilter->setOverlayPassiveBuffer(1);
    filterchain->setStageMode(VideoFilterchainSequence::ImageStage,
        VideoFilterArmedOnce);
    applyImageCuePalette(*pendingImageCue, frameBudgetFramesPerSecond);

    appliedImageCueId = pendingImageCueId;
    pendingImageCue = 0;
}

void FrameGeneratorSceneBinding::applyImageCuePalette(const IndexedImage& image,
    int frameBudgetFramesPerSecond) {
    if (scene == 0 || filterchain == 0)
        return;

    const ColorPalette* imagePalette = image.palette();
    if (imagePalette == 0)
        return;

    PaletteFilter* paletteFilter
        = stageFilter<PaletteFilter>(*filterchain,
            VideoFilterchainSequence::PaletteStage);
    if (paletteFilter == 0)
        return;

    FrameImageCuePaletteMode mode = transitionController.chooseImageCuePaletteMode(
        randomSourceValue, frameBudgetFramesPerSecond);
    if (mode == FrameImageCuePaletteIgnore)
        return;

    if (mode == FrameImageCuePaletteAdopt) {
        paletteFilter->setTargetPalette(*imagePalette, 0,
            linearPaletteTransitionStrategy());
        return;
    }

    const ColorPalette* returnPalette = scenePaletteColors(scene->settings());
    if (returnPalette == 0) {
        paletteFilter->setTargetPalette(*imagePalette, 0,
            linearPaletteTransitionStrategy());
        return;
    }

    paletteFilter->snapThenTransitionPalette(*imagePalette, *returnPalette,
        transitionController.paletteSmoothingFrameBudget(frameBudgetFramesPerSecond),
        linearPaletteTransitionStrategy());
}

void FrameGeneratorSceneBinding::applyPendingTextCue() {
    if (filterchain == 0 || pendingTextCueId == appliedTextCueId
        || pendingTextMessage.empty() || pendingTextFrames <= 0)
        return;

    TextInjectionFilter* textFilter
        = stageFilter<TextInjectionFilter>(*filterchain,
            VideoFilterchainSequence::TextStage);
    if (textFilter == 0)
        return;

    textFilter->setInkColor(pendingTextInkColor);
    textFilter->setMessage(pendingTextMessage.c_str(), pendingTextFrames);
    filterchain->setStageMode(VideoFilterchainSequence::TextStage,
        VideoFilterEnabled);

    appliedTextCueId = pendingTextCueId;
}

void FrameGeneratorSceneBinding::configureFilterchain(
    VideoFilterchain& filterchain_, const FrameGeneratorContext& context) {
    if (filterchain != &filterchain_) {
        filterchain = &filterchain_;
        pendingSceneChanges |= SceneAllChanged;
    }

    if (pendingSceneChanges != SceneNoChange) {
        applySceneToFilterchain(pendingSceneChanges,
            context.frameBudgetFramesPerSecond());
        pendingSceneChanges = SceneNoChange;
    }
    applyPendingImageCue(context.frameBudgetFramesPerSecond());
    applyPendingTextCue();
}

void FrameGeneratorSceneBinding::sceneChanged(Scene& scene_,
    unsigned int changes) {
    (void)scene_;

    pendingSceneChanges |= changes;
}

void FrameGeneratorSceneBinding::sceneCue(Scene& scene_, const SceneCue& cue) {
    (void)scene_;

    if (cue.type == SceneCueInjectImage) {
        pendingImageCue = cue.image;
        pendingImageCueId = cue.id;
    } else if (cue.type == SceneCueInjectText) {
        pendingTextMessage = cue.text;
        pendingTextFrames = cue.textFrames;
        pendingTextInkColor = cue.textInkColor;
        pendingTextCueId = cue.id;
    }
}
