// Application lifecycle, shutdown handling, and one-frame runtime dispatcher.

#include "Application.h"

#include "cthugha.h"
#include "information.h"
#include "display.h"
#include "AudioFrame.h"
#include "AudioRuntime.h"
#include "AudioSystem.h"
#include "AudioAnalyzer.h"
#include "AudioProcessor.h"
#include "AudioVisualBridge.h"
#include "Border.h"
#include "CoreOption.h"
#include "CthughaBuffer.h"
#include "CthughaDisplay.h"
#include "DisplayDevice.h"
#include "Flashlight.h"
#include "IndexedFrame.h"
#include "Interface.h"
#include "Scene.h"
#include "VideoDirector.h"
#include "VideoFilterchain.h"
#include "VideoFilterchainFactory.h"
#include "VideoFilters.h"
#include "imath.h"
#include "keymap.h"
#include "options.h"

#include <signal.h>
#include <unistd.h>

static Application* application = NULL;

static void configureTerminalTextMode();
void sig_tty_cont(int);
void sig_tty_stop(int);
void deleter();

Application::Application(int argc, char* argv[])
    : argcValue(argc)
    , argvValue(argv)
    , displayArgv(argv, argv + argc)
    , exitStatusValue(1)
    , exitHandlersRegistered(0)
    , shutdownComplete(0) { }

Application::~Application() {
    shutdown();
}

void Application::initSceneRuntime() {
    if (sceneValue.get() != NULL)
        return;

    sceneValue.reset(new Scene);
    videoDirector().bindScene(*sceneValue);
    sceneCommandsValue.reset(new SceneCommands(*sceneValue, CthughaBuffer::buffer,
        videoDirector().imageOption()));
    bindSceneCommandsForLegacyCallbacks(sceneCommandsValue.get());
}

void Application::shutdownSceneRuntime() {
    bindSceneCommandsForLegacyCallbacks(NULL);
    videoDirector().unbindScene();
    sceneCommandsValue.reset();
    sceneValue.reset();
}

void Application::initVideoFilterchain() {
    if (videoFilterchain.get() != NULL)
        return;

    VideoFilterchainFactory factory;
    videoFilterchainSequence = videoDirector().defaultFilterchainSequence();
    videoFilterchain.reset(factory.create(videoFilterchainSequence));

    if (displayDevice != NULL)
        displayDevice->setFramePalette(framePaletteFromFilterchain(*videoFilterchain));
}

void Application::shutdownVideoFilterchain() {
    videoFilterchain.reset();
}

void Application::initAudioVisualBridge() {
    if (audioVisualBridge.get() == NULL)
        audioVisualBridge.reset(new AudioVisualBridge(sceneCommandsValue.get()));
}

void Application::shutdownAudioVisualBridge() {
    audioVisualBridge.reset();
}

void Application::runAudioVisualBridge() {
    initAudioVisualBridge();
    audioVisualBridge->runFrame();

    if (audioVisualBridge->filterchainRefreshRequested()) {
        initVideoFilterchain();
        VideoFilterchainFactory factory;
        factory.refresh(*videoFilterchain, videoFilterchainSequence);
        audioVisualBridge->clearFilterchainRefreshRequest();
    }
}

const IndexedFrame* Application::runVideoFilterchain() {
    initVideoFilterchain();

    VideoFrameContext context;
    context.audioFrame = audioFrameCurrent();
    context.rawAudioData = audioFrameRawData();
    context.processedWaveData = audioFrameProcessedWaveData();
    context.audioMetrics = &audioMetrics;
    context.acousticContext = &acousticContext;
    context.now = now;
    context.deltaT = deltaT;

    CTH_TRACE("running filterchain=%p filters=%d\n", "video runtime",
        videoFilterchain.get(), videoFilterchain.get() ? videoFilterchain->size() : 0);
    CthughaBuffer* buffer = videoDirector().configureFilterchain(*videoFilterchain);
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

    // AutoChanger owns final option persistence, so destroy the bridge first.
    shutdownAudioVisualBridge();
    delete cthughaDisplay;
    cthughaDisplay = NULL;
    delete displayDevice;
    displayDevice = NULL;
    audioRuntimeShutdown();
    shutdownVideoFilterchain();
    shutdownSceneRuntime();
}

Scene& Application::scene() {
    return *sceneValue;
}

SceneCommands& Application::sceneCommands() {
    return *sceneCommandsValue;
}

int Application::initialize() {
    srand(time(0));
    seteuid(getuid()); // give up root privileges

    if (get_pre_params(argcValue, argvValue))
        return 0;

    if (params_request_help(argcValue, argvValue)) {
        title();
        usage();
        exitStatusValue = 0;
        return 0;
    }

    initSceneRuntime();

    if (get_params(argcValue, argvValue))
        return 0;

    title();

    init_imath();

    atexit(deleter);
    exitHandlersRegistered = 1;

    configureTerminalTextMode();
    if (ncurses_use) {
        init_ncurses();
        atexit(exit_ncurses);
    }

    CTH_INFO("Initializing the sound device...\n");
    init_sound(CthughaBuffer::buffer.maxDimension());

    CTH_INFO("Initializing cthugha Buffer...\n");
    CthughaBuffer::initAll();
    if (videoDirector().loadImages()) {
        exitStatusValue = 0;
        return 0;
    }
    init_border();
    init_flashlight();

    CTH_INFO("Setting initial core options...\n");
    CoreOption::changeToInitial();
    audioProcessing.changeToInitial();
    sceneCommands().initializeFromOptions();

    CTH_INFO("Initializing interface...\n");
    Interface::set("main");

    CTH_INFO("Initializing keymaps...\n");
    Keymap::init();

    CTH_INFO("Initializing display...\n");
    int displayArgc = int(displayArgv.size());
    if (cth_init(&displayArgc, displayArgv.data()))
        return 0;
    newDisplayDevice(scene(), sceneCommands());
    newCthughaDisplay();

    CTH_INFO("Initializing the audio-visual bridge...\n");
    initAudioVisualBridge();

    signal(SIGTSTP, sig_tty_stop);

    exitStatusValue = 0;
    return 1;
}

void Application::run() {
    displayDevice->mainLoop();

    CTH_INFO("Exiting cthugha...\n");
}

int Application::exitStatus() const {
    return exitStatusValue;
}

int Application::hasExitHandlers() const {
    return exitHandlersRegistered;
}

void Application::runFrame(int doDisplay) {
    double frameTiming[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int traceFrameTiming = CTH_LOG_ENABLED(CTH_LOG_TRACE);
    if (traceFrameTiming)
        frameTiming[0] = getTime();

    cthughaDisplay->nextFrame();
    if (traceFrameTiming)
        frameTiming[1] = getTime();

    audioFrameTick();
    if (traceFrameTiming)
        frameTiming[2] = getTime();

    runAudioVisualBridge();
    if (traceFrameTiming)
        frameTiming[3] = getTime();

    const IndexedFrame* indexedFrame = runVideoFilterchain();
    if (traceFrameTiming)
        frameTiming[4] = getTime();

    double visualStart = getTime();
    if (doDisplay) {
        if (indexedFrame != NULL && indexedFrame->valid())
            cthughaDisplay->present(*indexedFrame);
        else
            (*cthughaDisplay)();
    }
    double visualEnd = getTime();
    if (traceFrameTiming)
        frameTiming[5] = visualEnd;
    if (cthughaDisplay)
        cthughaDisplay->observeVisualLatency(visualEnd - visualStart);

    if (traceFrameTiming) {
        frameTiming[6] = getTime();
        CTH_TRACE("total-ms=%.3f next-frame=%.3f audio=%.3f bridge=%.3f buffer=%.3f display=%.3f do-display=%d\n",
            "frame timing",
            (frameTiming[6] - frameTiming[0]) * 1000.0,
            (frameTiming[1] - frameTiming[0]) * 1000.0,
            (frameTiming[2] - frameTiming[1]) * 1000.0,
            (frameTiming[3] - frameTiming[2]) * 1000.0,
            (frameTiming[4] - frameTiming[3]) * 1000.0,
            (frameTiming[5] - frameTiming[4]) * 1000.0,
            doDisplay);
    }

    // Suspend only between frame stages, after graphics operations are done.
    if (cthugha_pause) {
        cthugha_pause = 0;

        exit_sound();

        raise(SIGTSTP);
    }
}

Application* createApplication(int argc, char* argv[]) {
    if (application == NULL)
        application = new Application(argc, argv);
    return application;
}

void destroyApplication() {
    delete application;
    application = NULL;
}

static void configureTerminalTextMode() {
#if HAVE_NCURSES == 1
    ncurses_use = DisplayDevice::text_on_term;
#else
    ncurses_use = 0;
    DisplayDevice::text_on_term = 0;
#endif
}

void sig_tty_cont(int);
void sig_tty_stop(int) {
    CTH_INFO("Stopping...\n");

    signal(SIGCONT, sig_tty_cont);

    // Defer suspension until run() reaches a point outside graphics work.
    cthugha_pause = 1;
}
void sig_tty_cont(int) {
    CTH_INFO("Continuing...\n");

    init_sound(CthughaBuffer::buffer.maxDimension());

    signal(SIGTSTP, sig_tty_stop);

    raise(SIGCONT);
}

void deleter() {
    destroyApplication();
}

void run(int doDisplay) {
    if (application != NULL)
        application->runFrame(doDisplay);
}
