// Program entry point, shutdown handling, and one-frame runtime dispatcher.
// Most subsystems own their detailed setup; this file defines their startup
// order and the per-frame call sequence used by the display main loop.

#include "cthugha.h"
#include "cth_buffer.h"
#include "information.h"
#include "display.h"
#include "AudioSystem.h"
#include "AudioFrame.h"
#include "AudioRuntime.h"
#include "AudioVisualBridge.h"
#include "AudioProcessor.h"
#include "Border.h"
#include "translate.h"
#include "options.h"
#include "keys.h"
#include "imath.h"
#include "waves.h"
#include "Option.h"
#include "CthughaBuffer.h"
#include "CthughaDisplay.h"
#include "DisplayDevice.h"
#include "Flashlight.h"
#include "Interface.h"
#include "VideoFilters.h"
#include "Scene.h"
#include "VideoDirector.h"
#include "VideoFilterchain.h"
#include "VideoFilterchainFactory.h"
#include "keymap.h"

#include <memory>
#include <unistd.h>
#include <signal.h>

class Application {
    std::unique_ptr<VideoFilterchain> videoFilterchain;
    VideoFilterchainSequence videoFilterchainSequence;
    std::unique_ptr<AudioVisualBridge> audioVisualBridge;
    std::unique_ptr<Scene> sceneValue;
    std::unique_ptr<SceneCommands> sceneCommandsValue;
    int shutdownComplete;

    void shutdownSceneRuntime();
    void shutdownVideoFilterchain();
    void shutdownAudioVisualBridge();
    void runAudioVisualBridge();
    const IndexedFrame* runVideoFilterchain();

public:
    Application();
    ~Application();

    void initSceneRuntime();
    void initVideoFilterchain();
    void initAudioVisualBridge();
    void shutdown();
    void runFrame(int doDisplay);

    Scene& scene();
    SceneCommands& sceneCommands();
};

static Application* application = NULL;

Application::Application()
    : shutdownComplete(0) { }

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

static void destroyApplication() {
    delete application;
    application = NULL;
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

int main(int argc, char* argv[]) {

    srand(time(0));
    seteuid(getuid()); // give up root privileges

    application = new Application;

    if (get_pre_params(argc, argv)) {
        destroyApplication();
        return 1;
    }

    application->initSceneRuntime();

    if (cth_init(&argc, argv)) {
        destroyApplication();
        return 1;
    }

    if (get_params(argc, argv)) {
        destroyApplication();
        return 1;
    }

    title();

    init_imath();

    atexit(deleter);

    if (ncurses_use) {
        init_ncurses();
        atexit(exit_ncurses);
    }

    CTH_INFO("Initializing the sound device...\n");
    init_sound(CthughaBuffer::buffer.maxDimension());

    CTH_INFO("Initializing cthugha Buffer...\n");
    CthughaBuffer::initAll();
    if (videoDirector().loadImages())
        exit(0);
    init_border();
    init_flashlight();

    CTH_INFO("Initializing display...\n");
    newDisplayDevice(application->scene(), application->sceneCommands());
    newCthughaDisplay();

    CTH_INFO("Setting initial core options...\n");
    CoreOption::changeToInitial();
    audioProcessing.changeToInitial();
    application->sceneCommands().initializeFromOptions();

    CTH_INFO("Initializing interface...\n");
    Interface::set("main");

    CTH_INFO("Initializing keymaps...\n");
    Keymap::init();

    CTH_INFO("Initializing the audio-visual bridge...\n");
    application->initAudioVisualBridge();

    signal(SIGTSTP, sig_tty_stop);

    displayDevice->mainLoop();

    CTH_INFO("Exiting cthugha...\n");

    return 0;
}

void run(int doDisplay) {
    if (application != NULL)
        application->runFrame(doDisplay);
}
