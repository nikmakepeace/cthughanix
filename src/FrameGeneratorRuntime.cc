// Frame Generator module root.

#include "FrameGeneratorRuntime.h"

#include "FrameFilterchain.h"
#include "IndexedFrame.h"
#include "ProcessServices.h"
#include "SceneGeometry.h"

namespace {

static int stageFromUserName(
    const std::string& name, FrameFilterchainSequence::Stage* stage) {
    if (name == "image") {
        *stage = FrameFilterchainSequence::ImageStage;
        return 1;
    }
    if (name == "border") {
        *stage = FrameFilterchainSequence::BorderStage;
        return 1;
    }
    if (name == "flame") {
        *stage = FrameFilterchainSequence::FlameStage;
        return 1;
    }
    if (name == "translate") {
        *stage = FrameFilterchainSequence::TranslateStage;
        return 1;
    }
    if (name == "wave") {
        *stage = FrameFilterchainSequence::WaveStage;
        return 1;
    }
    if (name == "text") {
        *stage = FrameFilterchainSequence::TextStage;
        return 1;
    }
    if (name == "flashlight") {
        *stage = FrameFilterchainSequence::FlashlightStage;
        return 1;
    }
    return 0;
}

static void appendUniqueStage(
    FrameFilterchainSequence& sequence, FrameFilterchainSequence::Stage stage) {
    if (!sequence.includes(stage))
        sequence.append(stage);
}

static void appendDefaultUserStages(FrameFilterchainSequence& sequence) {
    appendUniqueStage(sequence, FrameFilterchainSequence::ImageStage);
    appendUniqueStage(sequence, FrameFilterchainSequence::BorderStage);
    appendUniqueStage(sequence, FrameFilterchainSequence::FlameStage);
    appendUniqueStage(sequence, FrameFilterchainSequence::TranslateStage);
    appendUniqueStage(sequence, FrameFilterchainSequence::WaveStage);
    appendUniqueStage(sequence, FrameFilterchainSequence::TextStage);
    appendUniqueStage(sequence, FrameFilterchainSequence::FlashlightStage);
}

static FrameFilterchainSequence filterchainSequenceFromStageNames(
    const std::vector<std::string>& stages) {
    FrameFilterchainSequence sequence;
    for (std::vector<std::string>::const_iterator it = stages.begin();
         it != stages.end(); ++it) {
        FrameFilterchainSequence::Stage stage
            = FrameFilterchainSequence::ImageStage;
        if (stageFromUserName(*it, &stage))
            appendUniqueStage(sequence, stage);
    }

    appendDefaultUserStages(sequence);
    sequence.append(FrameFilterchainSequence::FrameCommitStage);
    sequence.append(FrameFilterchainSequence::PaletteStage);
    sequence.append(FrameFilterchainSequence::IndexedFrameStage);
    return sequence;
}

static void applyFilterchainStageGate(FrameFilterchain& filterchain,
    FrameFilterchainSequence::Stage stage, int enabled) {
    filterchain.setStageEnabled(stage, enabled);
}

} // namespace

FrameGeneratorRuntimeConfig::FrameGeneratorRuntimeConfig()
    : frameSize(160, 100)
    , paletteSmoothingChance(0.0)
    , paletteSmoothSeconds(1)
    , quietMessageDurationMs(0)
    , silenceMessages() { }

FrameGeneratorRuntime::FrameGeneratorRuntime(RandomSource& randomSource,
    CountdownTimerFactory& timerFactory, LogSink& log)
    : randomSourceValue(randomSource)
    , logValue(log)
    , geometryValue()
    , frameStoreValue()
    , transitionControllerValue()
    , sceneBindingValue(geometryValue, transitionControllerValue,
          randomSourceValue, timerFactory, logValue)
    , pipelineValue(logValue)
    , filterchainSequenceValue(
          sceneBindingValue.defaultFilterchainSequence())
    , filterchainStageNamesValue()
    , filterchainStageEnabledValue()
    , filterchainStagePolicyActive(0) { }

FrameGeneratorRuntime::~FrameGeneratorRuntime() {
    unbindScene();
}

void FrameGeneratorRuntime::configure(
    const FrameGeneratorRuntimeConfig& config) {
    geometryValue = FrameGeometry(config.frameSize);
    frameStoreValue.resize(geometryValue);
    transitionControllerValue.configureTransitions(
        config.paletteSmoothingChance, config.paletteSmoothSeconds);
    transitionControllerValue.configureQuietMessages(
        config.quietMessageDurationMs);
    sceneBindingValue.silenceMessages().configure(config.silenceMessages);
}

const FrameGeometry& FrameGeneratorRuntime::geometry() const {
    return geometryValue;
}

SceneGeometry& FrameGeneratorRuntime::sceneGeometry() {
    return geometryValue;
}

FrameStore& FrameGeneratorRuntime::frameStore() {
    return frameStoreValue;
}

SilenceMessage& FrameGeneratorRuntime::silenceMessages() {
    return sceneBindingValue.silenceMessages();
}

void FrameGeneratorRuntime::setPaletteSmoothingChance(double chance) {
    transitionControllerValue.setPaletteSmoothingChance(chance);
}

int FrameGeneratorRuntime::filterchainStageEnabled(
    FrameFilterchainSequence::Stage stage) const {
    if (!filterchainStagePolicyActive)
        return 1;

    for (std::size_t i = 0; i < filterchainStageNamesValue.size(); i++) {
        FrameFilterchainSequence::Stage candidate
            = FrameFilterchainSequence::ImageStage;
        if (!stageFromUserName(filterchainStageNamesValue[i], &candidate)
            || candidate != stage)
            continue;

        if (i >= filterchainStageEnabledValue.size())
            return 1;
        return filterchainStageEnabledValue[i] != 0;
    }

    return 1;
}

void FrameGeneratorRuntime::applyFilterchainStageGates() {
    if (!filterchainStagePolicyActive || !pipelineValue.initialized())
        return;

    FrameFilterchain& filterchain = pipelineValue.filterchain();
    applyFilterchainStageGate(filterchain, FrameFilterchainSequence::ImageStage,
        filterchainStageEnabled(FrameFilterchainSequence::ImageStage));
    applyFilterchainStageGate(filterchain, FrameFilterchainSequence::BorderStage,
        filterchainStageEnabled(FrameFilterchainSequence::BorderStage));
    applyFilterchainStageGate(filterchain, FrameFilterchainSequence::FlameStage,
        filterchainStageEnabled(FrameFilterchainSequence::FlameStage));
    applyFilterchainStageGate(filterchain,
        FrameFilterchainSequence::TranslateStage,
        filterchainStageEnabled(FrameFilterchainSequence::TranslateStage));
    applyFilterchainStageGate(filterchain, FrameFilterchainSequence::WaveStage,
        filterchainStageEnabled(FrameFilterchainSequence::WaveStage));
    applyFilterchainStageGate(filterchain, FrameFilterchainSequence::TextStage,
        filterchainStageEnabled(FrameFilterchainSequence::TextStage));
    applyFilterchainStageGate(filterchain,
        FrameFilterchainSequence::FlashlightStage,
        filterchainStageEnabled(FrameFilterchainSequence::FlashlightStage));
}

void FrameGeneratorRuntime::setFilterchainSequence(
    const std::vector<std::string>& stages, const std::vector<int>& enabled) {
    filterchainStageNamesValue = stages;
    filterchainStageEnabledValue = enabled;
    filterchainStagePolicyActive = 1;
    filterchainSequenceValue = filterchainSequenceFromStageNames(stages);
    pipelineValue.setSequence(filterchainSequenceValue);
    applyFilterchainStageGates();
}

void FrameGeneratorRuntime::setFilterchainStageEnabled(
    const std::vector<std::string>& stages, const std::vector<int>& enabled) {
    filterchainStageNamesValue = stages;
    filterchainStageEnabledValue = enabled;
    filterchainStagePolicyActive = 1;
    applyFilterchainStageGates();
}

double FrameGeneratorRuntime::paletteSmoothingChance() const {
    return transitionControllerValue.paletteSmoothingChance();
}

int FrameGeneratorRuntime::paletteSmoothSeconds() const {
    return transitionControllerValue.paletteSmoothSeconds();
}

void FrameGeneratorRuntime::bindScene(Scene& scene) {
    sceneBindingValue.bindScene(scene);
}

void FrameGeneratorRuntime::unbindScene() {
    sceneBindingValue.unbindScene();
}

int FrameGeneratorRuntime::observeQuiet(int quietLength,
    int quietMessageThresholdMs, int frameBudgetFramesPerSecond) {
    return sceneBindingValue.observeQuiet(quietLength, quietMessageThresholdMs,
        frameBudgetFramesPerSecond);
}

void FrameGeneratorRuntime::initializePipeline() {
    if (pipelineValue.initialized())
        return;

    pipelineValue.initialize(filterchainSequenceValue);
}

void FrameGeneratorRuntime::resetPipeline() {
    pipelineValue.reset();
}

FramePalette* FrameGeneratorRuntime::framePalette() {
    initializePipeline();
    return pipelineValue.framePalette();
}

const IndexedFrame& FrameGeneratorRuntime::render(
    const FrameGeneratorContext& context) {
    initializePipeline();
    sceneBindingValue.configureFilterchain(pipelineValue.filterchain(), context);
    applyFilterchainStageGates();

    logValue.trace("frame generator", "running filterchain=%p filters=%d\n",
        &pipelineValue.filterchain(), pipelineValue.filterchain().size());
    return pipelineValue.render(frameStoreValue.renderTarget(), context);
}
