/** @file
 * Application lifecycle, shutdown handling, and one-frame runtime dispatcher.
 */

#include "Application.h"

#include "information.h"
#include "display.h"
#include "AudioFrame.h"
#include "AudioFramePipeline.h"
#include "AudioIngest.h"
#include "AudioAnalyzer.h"
#include "AudioProcessing.h"
#include "AudioProcessor.h"
#include "AutoChangeControls.h"
#include "AutoChangeSettings.h"
#include "Border.h"
#include "EffectChoiceLoader.h"
#include "CthughaBuffer.h"
#include "CthughaDisplay.h"
#include "DisplayBackend.h"
#include "DisplayDevice.h"
#include "DisplayRuntime.h"
#include "Flashlight.h"
#include "IndexedFrame.h"
#include "Interface.h"
#include "InterfaceRuntime.h"
#include "LegacySceneVisualCatalogs.h"
#include "Mixer.h"
#include "IniFiles.h"
#include "Option.h"
#include "RuntimeAudioControls.h"
#include "RuntimeAutoChangeControls.h"
#include "RuntimeConfigRegistry.h"
#include "RuntimeChangeMediator.h"
#include "RuntimeCommandTargets.h"
#include "RuntimeDisplayControls.h"
#include "RuntimeEffectControls.h"
#include "RuntimePersistence.h"
#include "RuntimeShutdown.h"
#include "Scene.h"
#include "SceneChangeScheduler.h"
#include "SceneRuntime.h"
#include "Screen.h"
#include "VideoDirector.h"
#include "VideoFilterchain.h"
#include "VideoFilterchainFactory.h"
#include "VideoFilters.h"
#include "TranslationOptions.h"
#include "flames.h"
#include "imath.h"
#include "keymap.h"
#include "keys.h"
#include "waves.h"

#ifdef CTH_XWIN
#include "xcthugha.h"
#endif

#include <unistd.h>

static int initializeVisualCatalogs(const CthughaBuffer& buffer,
    const PathConfig& pathConfig, RandomSource& randomSource);
static int loadEffectPolicyImages(const EffectPolicy& effectPolicy,
    const PathConfig& pathConfig, CthughaBuffer& targetBuffer,
    ImageOption& images, LogSink& log);
static void emitStartupConfigDiagnostics(
    const std::vector<ConfigDiagnostic>& diagnostics, LogSink& log);
static void applyDisplayPresentationStartupChoice(const SceneConfig& sceneConfig,
    RandomSource& randomSource);

class VideoDirectorQuietObserver : public AutoChangeQuietObserver {
    VideoDirector& videoDirectorValue;

public:
    /**
     * Creates a quiet-audio observer over a video director.
     *
     * @param videoDirector_ Director that owns quiet-message policy.
     */
    explicit VideoDirectorQuietObserver(VideoDirector& videoDirector_)
        : videoDirectorValue(videoDirector_) { }

    /** Reports an ongoing quiet period to the video director. */
    virtual int observeQuiet(int quietLength) {
        return videoDirectorValue.observeQuiet(quietLength);
    }
};

static VideoFrameContext videoFrameContextFor(const AudioFrame& frame,
    const AcousticContext& acousticContext, const SceneSnapshot* sceneSnapshot,
    double frameNow, double frameDeltaT) {
    VideoFrameContext context;
    context.audioFrame = &frame;
    context.rawAudioData = frame.raw;
    context.processedWaveData = frame.processedWaveData;
    context.audioMetrics = &frame.metrics;
    context.acousticContext = &acousticContext;
    context.sceneSnapshot = sceneSnapshot;
    context.now = frameNow;
    context.deltaT = frameDeltaT;
    return context;
}

static void emitStartupConfigDiagnostics(
    const std::vector<ConfigDiagnostic>& diagnostics, LogSink& log) {
    for (std::vector<ConfigDiagnostic>::const_iterator it = diagnostics.begin();
         it != diagnostics.end(); ++it) {
        if (it->severity == ConfigDiagnosticError) {
            log.error("Configuration error from %s `%s': %s\n",
                it->source.c_str(), it->key.c_str(), it->message.c_str());
        } else if (it->severity == ConfigDiagnosticWarning) {
            log.warn("Configuration warning from %s `%s': %s\n",
                it->source.c_str(), it->key.c_str(), it->message.c_str());
        } else {
            log.info("Configuration note from %s `%s': %s\n",
                it->source.c_str(), it->key.c_str(), it->message.c_str());
        }
    }
}

static int loadEffectPolicyImages(const EffectPolicy& effectPolicy,
    const PathConfig& pathConfig, CthughaBuffer& targetBuffer,
    ImageOption& images, LogSink& log) {
    if (!effectPolicy.imageFilesEnabled)
        return 0;

    log.info("  loading image files...\n");
    int result = images.loadImages(pathConfig, targetBuffer.width(),
        targetBuffer.height());
    log.info("  number of loaded image files: %d\n", images.getNEntries());

    return result;
}

static void applyDisplayPresentationStartupChoice(const SceneConfig& sceneConfig,
    RandomSource& randomSource) {
    screen.change(sceneConfig.presentation.c_str(), randomSource, 0);
}

Application::Application(int argc, char* argv[])
    : argcValue(argc)
    , argvValue(argv)
    , displayArgv(argv, argv + argc)
    , framePacerValue(frameSleeperValue)
    , displayFrontendInitializer(&displayFrontendInitializerValue)
    , logSinkValue(loggingRuntimeValue)
    , exitStatusValue(1)
    , acousticContextValue(&logSinkValue)
    , videoDirectorValue(CthughaBuffer::buffer)
    , platformLifecycle(logSinkValue, PlatformLifecycleCallbacks(
          &Application::platformWillSuspend, &Application::platformDidResume, this))
    , startupInitialized(0)
    , shutdownComplete(0) {
    cthugha_install_logging_runtime(loggingRuntimeValue);
    videoDirectorValue.setRandomSource(randomSourceValue);
    videoDirectorValue.setTimerFactory(countdownTimerFactoryValue);
    sceneVisualCatalogFactoryValue
        = createLegacySceneVisualCatalogFactory(flame, flameGeneral, wave,
            waveScale, table, object, translation, palette, border,
            flashlight, videoDirectorValue.imageOption());
    sceneRuntimeValue.reset(new SceneRuntime(videoDirectorValue,
        *sceneVisualCatalogFactoryValue, randomSourceValue));
    interfaceRuntimeValue.reset(new InterfaceRuntime(millisecondClockValue));
    errorMessagesValue.reset(new ErrorMessages());
    registerDefaultInterfaces(*interfaceRuntimeValue, videoDirectorValue.imageOption());
}

Application::Application(int argc, char* argv[],
    DisplayFrontendInitializer& displayFrontendInitializer_)
    : argcValue(argc)
    , argvValue(argv)
    , displayArgv(argv, argv + argc)
    , framePacerValue(frameSleeperValue)
    , displayFrontendInitializer(&displayFrontendInitializer_)
    , logSinkValue(loggingRuntimeValue)
    , exitStatusValue(1)
    , acousticContextValue(&logSinkValue)
    , videoDirectorValue(CthughaBuffer::buffer)
    , platformLifecycle(logSinkValue, PlatformLifecycleCallbacks(
          &Application::platformWillSuspend, &Application::platformDidResume, this))
    , startupInitialized(0)
    , shutdownComplete(0) {
    cthugha_install_logging_runtime(loggingRuntimeValue);
    videoDirectorValue.setRandomSource(randomSourceValue);
    videoDirectorValue.setTimerFactory(countdownTimerFactoryValue);
    sceneVisualCatalogFactoryValue
        = createLegacySceneVisualCatalogFactory(flame, flameGeneral, wave,
            waveScale, table, object, translation, palette, border,
            flashlight, videoDirectorValue.imageOption());
    sceneRuntimeValue.reset(new SceneRuntime(videoDirectorValue,
        *sceneVisualCatalogFactoryValue, randomSourceValue));
    interfaceRuntimeValue.reset(new InterfaceRuntime(millisecondClockValue));
    errorMessagesValue.reset(new ErrorMessages());
    registerDefaultInterfaces(*interfaceRuntimeValue, videoDirectorValue.imageOption());
}

Application::~Application() {
    shutdown();
    cthugha_clear_logging_runtime(loggingRuntimeValue);
}

void Application::platformWillSuspend(void* context) {
    ((Application*)context)->willSuspend();
}

void Application::platformDidResume(void* context) {
    ((Application*)context)->didResume();
}

void Application::willSuspend() {
    shutdownAudioIngest();
}

void Application::didResume() {
    if (initAudioIngest() && runtimeShutdownValue.get() != NULL)
        runtimeShutdownValue->requestClose();
}

bool Application::closeRequested() const {
    return (runtimeShutdownValue.get() != NULL)
        && runtimeShutdownValue->closeRequested();
}

void Application::initSceneRuntime() {
    if (runtimeConfigRegistryValue.get() != NULL)
        return;

    videoDirectorValue.bindScene(sceneRuntimeValue->scene());
    runtimeConfigRegistryValue.reset(new RuntimeConfigRegistry(startupConfigValue));
    audioProcessorValue.reset(new AudioProcessor());
    audioProcessingStateValue.reset(new AudioProcessingState(randomSourceValue));
    audioProcessingSelectorValue.reset(
        new AudioProcessingSelector(*audioProcessingStateValue,
            *audioProcessorValue, logSinkValue));
    autoChangeSettingsValue.reset(
        new OwnedAutoChangeSettings(startupConfigValue.autoChange));
    autoChangeControlsValue.reset(
        new AutoChangeControls(*autoChangeSettingsValue, logSinkValue));
    runtimeConfigRegistryValue->addContributor(sceneRuntimeValue->serializer());
    runtimeConfigContributorValue.reset(
        new LegacyRuntimeConfigContributor(*autoChangeSettingsValue,
            *audioProcessingStateValue));
    runtimeConfigRegistryValue->addContributor(*runtimeConfigContributorValue);
    runtimePersistenceValue.reset(
        new IniRuntimePersistence(*runtimeConfigRegistryValue, logSinkValue));
    runtimeShutdownValue.reset(new RuntimeCloseState());
    runtimeDisplayControlsValue.reset(
        new DefaultRuntimeDisplayControls(randomSourceValue));
    runtimeAudioControlsValue.reset(
        new DefaultRuntimeAudioControls(*audioProcessingSelectorValue,
            mixerControlsValue.get()));
    runtimeAutoChangeControlsValue.reset(
        new DefaultRuntimeAutoChangeControls(*autoChangeControlsValue));
    runtimeEffectControlsValue.reset(
        new DefaultRuntimeEffectControls(randomSourceValue));
    interfaceRuntimeValue->setRuntimeConfigRegistry(runtimeConfigRegistryValue.get());
    interfaceRuntimeValue->setAudioProcessingSelector(audioProcessingSelectorValue.get());
    runtimeChangeMediatorValue.reset(new RuntimeChangeMediator(
        sceneRuntimeValue->commandTarget(), *runtimePersistenceValue,
        *runtimeShutdownValue, *runtimeDisplayControlsValue,
        *runtimeAudioControlsValue, *runtimeAutoChangeControlsValue,
        *runtimeEffectControlsValue));
    runtimeCommandRouterValue.reset(new RoutedRuntimeCommandTargetRouter(
        *runtimeChangeMediatorValue, sceneRuntimeValue->effectControlOwner(),
        *runtimeDisplayControlsValue, *runtimeAudioControlsValue,
        *runtimeAutoChangeControlsValue, *runtimeEffectControlsValue));
    interfaceRuntimeValue->setAutoChangeControls(autoChangeControlsValue.get());
}

void Application::shutdownSceneRuntime() {
    interfaceRuntimeValue->setAutoChangeControls(NULL);
    videoDirectorValue.unbindScene();
    interfaceRuntimeValue->setAudioProcessingSelector(NULL);
    interfaceRuntimeValue->setRuntimeConfigRegistry(NULL);
    runtimeCommandRouterValue.reset();
    runtimeChangeMediatorValue.reset();
    runtimeEffectControlsValue.reset();
    runtimeAutoChangeControlsValue.reset();
    runtimeAudioControlsValue.reset();
    runtimeDisplayControlsValue.reset();
    runtimePersistenceValue.reset();
    runtimeShutdownValue.reset();
    runtimeConfigContributorValue.reset();
    runtimeConfigRegistryValue.reset();
    autoChangeControlsValue.reset();
    autoChangeSettingsValue.reset();
    audioProcessingSelectorValue.reset();
    audioProcessingStateValue.reset();
    audioProcessorValue.reset();
}

int Application::initMixerRuntime() {
    if (mixerSessionValue.get() != NULL)
        return 0;

    if (startupConfigValue.audio.inputMode != AIM_DSPIn)
        return 0;

    logSinkValue.info("Initializing OSS mixer device...\n");
    mixerDeviceValue.reset(newMixerDevice());
    mixerSessionValue.reset(
        new MixerSession(*mixerDeviceValue, logSinkValue,
            startupConfigValue.audio));
    if (mixerSessionValue->initialize()) {
        mixerControlsValue.reset();
        mixerSessionValue.reset();
        mixerDeviceValue.reset();
        return 1;
    }

    mixerControlsValue.reset(new MixerControls(*mixerSessionValue, logSinkValue));
    Interface* mixerInterface = interfaceRuntimeValue->find("Mixer");
    if (mixerInterface == NULL) {
        logSinkValue.error("Mixer interface is not registered.\n");
        return 1;
    }
    mixerControlsValue->installInto(*mixerInterface);
    return 0;
}

void Application::shutdownMixerRuntime() {
    if (mixerControlsValue.get() != NULL) {
        Interface* mixerInterface = interfaceRuntimeValue->find("Mixer");
        if (mixerInterface != NULL)
            mixerControlsValue->clearInterface(*mixerInterface);
    }
    mixerControlsValue.reset();
    mixerSessionValue.reset();
    mixerDeviceValue.reset();
}

void Application::initVideoFilterchain() {
    if (videoFilterchain.get() != NULL)
        return;

    VideoFilterchainFactory factory;
    videoFilterchainSequence = videoDirectorValue.defaultFilterchainSequence();
    videoFilterchain.reset(factory.create(videoFilterchainSequence));

    if (displayRuntimeOwnership.get() != NULL)
        displayRuntimeOwnership->device().setFramePalette(
            framePaletteFromFilterchain(*videoFilterchain));
}

void Application::shutdownVideoFilterchain() {
    videoFilterchain.reset();
}

int Application::initAudioIngest() {
    if (audioIngestValue.get() != NULL)
        return 0;

    audioIngestValue.reset(new AudioIngest(startupConfigValue.audio,
        CthughaBuffer::buffer.maxDimension(), randomSourceValue,
        secondsClockValue, logSinkValue));
    if (audioIngestValue->start()) {
        audioIngestValue.reset();
        return 1;
    }

    return 0;
}

void Application::shutdownAudioIngest() {
    if (audioIngestValue.get() != NULL)
        audioIngestValue->stop();
    audioIngestValue.reset();
}

void Application::initAudioFramePipeline() {
    if (audioFramePipelineValue.get() == NULL) {
        audioFramePipelineValue.reset(new DefaultAudioFramePipeline(
            acousticContextValue, *audioProcessingSelectorValue,
            *audioProcessorValue, secondsClockValue, logSinkValue,
            startupConfigValue.audioAnalysis.minNoise));
    }
    if (autoChangeQuietObserverValue.get() == NULL)
        autoChangeQuietObserverValue.reset(
            new VideoDirectorQuietObserver(videoDirectorValue));
    if (sceneChangeSchedulerValue.get() == NULL
        && sceneRuntimeValue.get() != NULL
        && autoChangeSettingsValue.get() != NULL)
        sceneChangeSchedulerValue.reset(new SceneChangeScheduler(sceneRuntimeValue->commandTarget(),
            *autoChangeSettingsValue, acousticContextValue,
            millisecondClockValue, randomSourceValue,
            *autoChangeQuietObserverValue, logSinkValue));
    interfaceRuntimeValue->setSceneChangeStatusProvider(
        sceneChangeSchedulerValue.get());
}

void Application::shutdownAudioFramePipeline() {
    interfaceRuntimeValue->setSceneChangeStatusProvider(NULL);
    sceneChangeSchedulerValue.reset();
    autoChangeQuietObserverValue.reset();
    audioFramePipelineValue.reset();
}

void Application::runAudioFramePipeline(AudioFrame& frame) {
    initAudioFramePipeline();
    audioFramePipelineValue->processFrame(frame);
    if (sceneChangeSchedulerValue.get() != NULL)
        (*sceneChangeSchedulerValue)(frame.metrics);
}

const IndexedFrame* Application::runVideoFilterchain(
    AudioFrame& frame, const SceneSnapshot& sceneSnapshot) {
    initVideoFilterchain();

    // The filterchain receives a snapshot-like context for this visual frame.
    // Audio frame data and frame-local metrics are owned by AudioIngest; filters
    // borrow them only during run().
    VideoFrameContext context = videoFrameContextFor(frame, acousticContextValue,
        &sceneSnapshot, displayValue->currentFrameTimeSeconds(),
        displayValue->currentFrameDeltaSeconds());

    logSinkValue.trace("video runtime", "running filterchain=%p filters=%d\n",
        videoFilterchain.get(), videoFilterchain.get() ? videoFilterchain->size() : 0);
    CthughaBuffer* buffer = videoDirectorValue.configureFilterchain(*videoFilterchain);
    if (buffer != NULL) {
        videoFilterchain->run(*buffer, context);
        return &videoFilterchain->indexedFrame();
    }

    return NULL;
}

void Application::shutdown() {
    if (shutdownComplete)
        return;

    shutdownComplete = 1;

    if (startupInitialized && startupConfigValue.app.optionsSaveEnabled
        && runtimePersistenceValue.get() != NULL)
        runtimePersistenceValue->writeCurrentConfig();

    shutdownAudioFramePipeline();
    displayValue.reset();
    cthughaDisplay = NULL;
    if (displayRuntimeOwnership.get() != NULL)
        displayRuntimeOwnership->shutdown();
    displayRuntimeOwnership.reset();
    platformLifecycle.shutdown();
    shutdownAudioIngest();
    shutdownVideoFilterchain();
    shutdownSceneRuntime();
    shutdownMixerRuntime();
}

Scene& Application::scene() {
    return sceneRuntimeValue->scene();
}

int Application::initialize() {
    seteuid(getuid()); // give up root privileges

    ConfigBuildResult startupConfig = buildStartupConfig(argcValue, argvValue);
    startupConfigValue = startupConfig.config;
    startupConfigDiagnostics = startupConfig.diagnostics;
    loggingRuntimeValue.configure(startupConfigValue.logging);

    emitStartupConfigDiagnostics(startupConfigDiagnostics, logSinkValue);
    if (!startupConfig.ok()) {
        title();
        usage();
        return 0;
    }

    if (startupConfig.helpRequested) {
        title();
        usage();
        exitStatusValue = 0;
        return 0;
    }

    inputQueueValue.configure(startupConfigValue.input);
    configureCthughaDisplay(startupConfigValue.display);
#ifdef CTH_XWIN
    configureDisplayDeviceX11(startupConfigValue.x11);
#endif
    configureTranslationOptions(startupConfigValue.effectPolicy);
    configureWaveOptions(startupConfigValue.effectPolicy);
    configurePaletteOptions(startupConfigValue.effectPolicy);
    videoDirectorValue.configureTransitions(startupConfigValue.sceneTransition);
    videoDirectorValue.configureQuietMessages(startupConfigValue.messages);
    CthughaBuffer::buffer.setDimensions(startupConfigValue.display.bufferWidth,
        startupConfigValue.display.bufferHeight);

    remove_continuation_ini(startupConfigValue.paths, logSinkValue);

    if (initMixerRuntime())
        return 0;

    initSceneRuntime();

    title();
    videoDirectorValue.silenceMessages().initialize();

    init_imath();

    logSinkValue.info("Initializing the sound device...\n");
    if (initAudioIngest())
        return 0;

    // Visual catalogs depend on final buffer dimensions and must be available
    // before startup scene config can be matched to concrete catalog entries.
    logSinkValue.info("Initializing cthugha Buffer...\n");
    if (initializeVisualCatalogs(CthughaBuffer::buffer, startupConfigValue.paths,
            randomSourceValue))
        return 0;
    CthughaBuffer::buffer.allocatePixels();
    if (loadEffectPolicyImages(startupConfigValue.effectPolicy,
            startupConfigValue.paths, CthughaBuffer::buffer,
            videoDirectorValue.imageOption(), logSinkValue)) {
        exitStatusValue = 0;
        return 0;
    }
    init_border();
    init_flashlight();
    sceneRuntimeValue->configureEffectPolicy(startupConfigValue.effectPolicy);

    logSinkValue.info("Setting initial effect controls...\n");
    sceneRuntimeValue->applyStartupConfig(startupConfigValue.scene);
    applyDisplayPresentationStartupChoice(startupConfigValue.scene,
        randomSourceValue);
    audioProcessingSelectorValue->configureStartup(startupConfigValue.scene);

    // Interface/keymaps are available before display creation so early display
    // events and option panels can route input immediately.
    logSinkValue.info("Initializing interface...\n");
    interfaceRuntimeValue->set("main");

    logSinkValue.info("Registering key actions...\n");
    registerDefaultKeyActions(commandsValue);
    registerInterfaceKeyActions(commandsValue);

    logSinkValue.info("Initializing keymaps...\n");
    keymapsValue.init(startupConfigValue.input, commandsValue);

    logSinkValue.info("Initializing display...\n");
    int displayArgc = int(displayArgv.size());
    if (displayFrontendInitializer->initializeDisplayFrontend(
            &displayArgc, displayArgv.data()))
        return 0;
    displayRuntimeOwnership = newDisplayDevice(scene(),
        videoDirectorValue.imageOption(), *runtimeChangeMediatorValue,
        *runtimeCommandRouterValue, *runtimeConfigRegistryValue,
        startupConfigValue.display, secondsClockValue);
    if (displayRuntimeOwnership.get() == NULL)
        return 0;
    displayRuntimeOwnership->publishAliases();
    displayValue = newCthughaDisplay(displayRuntimeOwnership->device(),
        displayRuntimeOwnership->runtime(), secondsClockValue,
        *interfaceRuntimeValue, *errorMessagesValue);
    cthughaDisplay = displayValue.get();

    logSinkValue.info("Initializing the audio-visual bridge...\n");
    initAudioFramePipeline();

    // Install platform hooks last; callbacks assume audio/display state exists.
    platformLifecycle.install();

    startupInitialized = 1;
    exitStatusValue = 0;
    return 1;
}

void Application::run() {
    // Main loop shape:
    //   1. collect platform/window input;
    //   2. let the active interface react before frame generation;
    //   3. generate and optionally present one frame;
    //   4. let the interface draw/react again after frame-side changes.
    while (!closeRequested()) {
        int traceDisplayTiming = logSinkValue.traceEnabled();
        double loopStart = traceDisplayTiming ? secondsClockValue.nowSeconds() : 0.0;
        double eventsStart = loopStart;
        double eventsEnd = loopStart;
        double preInterfaceStart = 0.0;
        double preInterfaceEnd = 0.0;
        double frameStart = 0.0;
        double frameEnd = 0.0;
        double postInterfaceStart = 0.0;
        double postInterfaceEnd = 0.0;
        double pacingStart = 0.0;
        double pacingEnd = 0.0;
        int frameWasRun = 0;
        double visualFrameStart = 0.0;

        DisplayEventStats eventStats
            = displayRuntimeOwnership->runtime().processEvents(inputQueueValue);
        if (traceDisplayTiming)
            eventsEnd = secondsClockValue.nowSeconds();

        CommandContext commandContext(*interfaceRuntimeValue,
            runtimeChangeMediatorValue.get(), runtimeCommandRouterValue.get());

        if (traceDisplayTiming)
            preInterfaceStart = secondsClockValue.nowSeconds();
        interfaceRuntimeValue->runCurrent(inputQueueValue, keymapsValue,
            commandsValue, dispatcherValue, commandContext);
        if (traceDisplayTiming)
            preInterfaceEnd = secondsClockValue.nowSeconds();

        if (!closeRequested()) {
            if (traceDisplayTiming)
                frameStart = secondsClockValue.nowSeconds();
            runFrame(1);
            frameWasRun = 1;
            visualFrameStart = displayValue->currentFrameTimeSeconds();
            if (traceDisplayTiming)
                frameEnd = secondsClockValue.nowSeconds();
        }

        if (traceDisplayTiming)
            postInterfaceStart = secondsClockValue.nowSeconds();
        interfaceRuntimeValue->runCurrent(inputQueueValue, keymapsValue,
            commandsValue, dispatcherValue, commandContext);
        if (traceDisplayTiming)
            postInterfaceEnd = secondsClockValue.nowSeconds();

        if (frameWasRun && !closeRequested()) {
            if (traceDisplayTiming)
                pacingStart = secondsClockValue.nowSeconds();
            FramePacingResult pacing = framePacerValue.paceFrameEnd(visualFrameStart,
                secondsClockValue.nowSeconds(), int(maxFramesPerSecond));
            if (traceDisplayTiming) {
                pacingEnd = secondsClockValue.nowSeconds();
                logSinkValue.trace("frame pacing",
                    "pacing target-ms=%.3f requested-sleep-ms=%.3f actual-pacing-ms=%.3f maxfps=%d\n",
                    (pacing.targetFrameEndSeconds - pacing.frameStartSeconds)
                        * 1000.0,
                    pacing.requestedSleepSeconds * 1000.0,
                    (pacingEnd - pacingStart) * 1000.0,
                    pacing.maxFramesPerSecond);
            }
        }

        if (traceDisplayTiming) {
            double loopEnd = secondsClockValue.nowSeconds();
            logSinkValue.trace("display timing",
                "mainloop-ms=%.3f events-ms=%.3f pre-interface-ms=%.3f frame-ms=%.3f post-interface-ms=%.3f pacing-ms=%.3f events=%d resize-events=%d expose-events=%d\n",
                (loopEnd - loopStart) * 1000.0,
                (eventsEnd - eventsStart) * 1000.0,
                (preInterfaceEnd - preInterfaceStart) * 1000.0,
                (frameEnd - frameStart) * 1000.0,
                (postInterfaceEnd - postInterfaceStart) * 1000.0,
                (pacingEnd - pacingStart) * 1000.0,
                eventStats.eventCount, eventStats.resizeEvents,
                eventStats.exposeEvents);
        }
    }

    logSinkValue.info("Exiting cthugha...\n");
}

int Application::exitStatus() const {
    return exitStatusValue;
}

void Application::runFrame(int doDisplay) {
    double frameTiming[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int traceFrameTiming = logSinkValue.traceEnabled();
    if (traceFrameTiming)
        frameTiming[0] = secondsClockValue.nowSeconds();

    // Advance display timing first so owned frame time describes this visual frame for
    // audio analysis, scene-change policy, and all visual filters.
    displayValue->nextFrame();
    if (traceFrameTiming)
        frameTiming[1] = secondsClockValue.nowSeconds();

    audioIngestValue->tick();
    AudioFrame& audioFrame = audioIngestValue->currentFrame();
    if (audioIngestValue->complete() && runtimeShutdownValue.get() != NULL) {
        logSinkValue.info("Stopping...\n");
        runtimeShutdownValue->requestClose();
    }
    if (traceFrameTiming)
        frameTiming[2] = secondsClockValue.nowSeconds();

    // Analyze audio and run option-changing policy before visual filters read
    // SceneSettings.
    runAudioFramePipeline(audioFrame);
    if (traceFrameTiming)
        frameTiming[3] = secondsClockValue.nowSeconds();

    // Mutate Cthugha's indexed active/passive buffers and publish a frame view.
    SceneSnapshot sceneSnapshot = sceneRuntimeValue->snapshot();
    const IndexedFrame* indexedFrame = runVideoFilterchain(audioFrame, sceneSnapshot);
    VideoFrameContext presentationContext = videoFrameContextFor(audioFrame,
        acousticContextValue, &sceneSnapshot, displayValue->currentFrameTimeSeconds(),
        displayValue->currentFrameDeltaSeconds());
    if (traceFrameTiming)
        frameTiming[4] = secondsClockValue.nowSeconds();

    // Prefer the modern IndexedFrame path. Fall back to the legacy screen()
    // function path when no filterchain frame was published.
    double visualStart = secondsClockValue.nowSeconds();
    if (doDisplay) {
        if (indexedFrame != NULL && indexedFrame->valid())
            displayValue->present(*indexedFrame, presentationContext);
        else
            displayValue->presentCurrent(presentationContext);
    }
    double visualEnd = secondsClockValue.nowSeconds();
    if (traceFrameTiming)
        frameTiming[5] = visualEnd;
    if (displayValue.get() != NULL)
        displayValue->observeVisualLatency(visualEnd - visualStart);

    if (traceFrameTiming) {
        frameTiming[6] = secondsClockValue.nowSeconds();
        logSinkValue.trace("frame timing",
            "total-ms=%.3f next-frame=%.3f audio=%.3f bridge=%.3f buffer=%.3f display=%.3f do-display=%d\n",
            (frameTiming[6] - frameTiming[0]) * 1000.0,
            (frameTiming[1] - frameTiming[0]) * 1000.0,
            (frameTiming[2] - frameTiming[1]) * 1000.0,
            (frameTiming[3] - frameTiming[2]) * 1000.0,
            (frameTiming[4] - frameTiming[3]) * 1000.0,
            (frameTiming[5] - frameTiming[4]) * 1000.0,
            doDisplay);
    }

    // Service lifecycle requests only between frame stages.
    platformLifecycle.serviceFrameBoundary();
}

static int initializeVisualCatalogs(const CthughaBuffer& buffer,
    const PathConfig& pathConfig, RandomSource& randomSource) {
    // Built-in visual choices and file-backed catalogs are application startup
    // state, not pixel-buffer state. They live here because option parsing can
    // change buffer dimensions and stage initial option names before startup.
    flame.add(_flames, _nFlames);

    if (init_flames())
        return 1;

    if (init_translate(buffer, randomSource))
        return 1;

    if (init_wave(pathConfig))
        return 1;

    if (load_palettes(pathConfig))
        return 1;

    return 0;
}
