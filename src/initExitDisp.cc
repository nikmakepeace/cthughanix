// Program entry point, shutdown handling, and one-frame runtime dispatcher.
// Most subsystems own their detailed setup; this file defines their startup
// order and the per-frame call sequence used by the display main loop.

#include "cthugha.h"
#include "information.h"
#include "display.h"
#include "Sound.h"
#include "AudioFrame.h"
#include "AudioRuntime.h"
#include "translate.h"
#include "options.h"
#include "keys.h"
#include "imath.h"
#include "waves.h"
#include "Option.h"
#include "AudioAnalyzer.h"
#include "AutoChanger.h"
#include "CthughaBuffer.h"
#include "SoundServer.h"
#include "CthughaDisplay.h"
#include "CDPlayer.h"
#include "DisplayDevice.h"
#include "Interface.h"
#include "keymap.h"

#include <unistd.h>
#include <signal.h>

void sig_tty_cont(int);
void sig_tty_stop(int) {
    CTH_INFO("Stopping...\n");

    signal(SIGCONT, sig_tty_cont);

    // Defer suspension until run() reaches a point outside graphics work.
    cthugha_pause = 1;
}
void sig_tty_cont(int) {
    CTH_INFO("Continuing...\n");

    init_sound();

    signal(SIGTSTP, sig_tty_stop);

    raise(SIGCONT);
}

void deleter() {
    // autoChanger owns final option persistence, so destroy it first.
    delete autoChanger;
    delete cthughaDisplay;
    delete soundServer;
    audioRuntimeShutdown();
    delete cdPlayer;
}

int main(int argc, char* argv[]) {

    srand(time(0));
    seteuid(getuid()); // give up root privileges

    if (get_pre_params(argc, argv))
        return 1;

    if (cth_init(&argc, argv))
        return 1;

    if (get_params(argc, argv))
        return 1;

    title();

    init_imath();

    atexit(deleter);

    if (ncurses_use) {
        init_ncurses();
        atexit(exit_ncurses);
    }

    CTH_INFO("Initializing the sound device...\n");
    audioRuntimeInit(RSIC_MainProcess, 1);

    CTH_INFO("Initializing the sound server...\n");
    soundServer = new SoundServer;

    CTH_INFO("Initializing CD player...\n");
    cdPlayer = new CDPlayer;

    CTH_INFO("Initializing cthugha Buffer...\n");
    CthughaBuffer::initAll();

    CTH_INFO("Initializing display...\n");
    newDisplayDevice();
    newCthughaDisplay();

    CTH_INFO("Setting initial core options...\n");
    CoreOption::changeToInitial();

    CTH_INFO("Initializing interface...\n");
    Interface::set("main");

    CTH_INFO("Initializing keymaps...\n");
    Keymap::init();

    CTH_INFO("Initializing the automatic changing...\n");
    autoChanger = new AutoChanger;

    signal(SIGTSTP, sig_tty_stop);

    displayDevice->mainLoop();

    CTH_INFO("Exiting cthugha...\n");

    return 0;
}

// Ad hoc profiling of the main loop.
// This is not very exact, but it gives a good impression of where the time is spent.
// To enable it, define PROF in the Makefile and recompile.
//
// The timing is accumulated and output is printed every 25 frames
// It shows the time spent in each module (sound reading, sound analyzing, display, ...)
// and the total time for 25 frames.


#undef PROF
//#define PROF // uncomment to enable ad hoc profiling of the main loop


#ifdef PROF
#define PROFILING() T[profilingIndex++] = getTime()
#else
#define PROFILING()
#endif

void run(int doDisplay) {
#ifdef PROF
    double T[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int profilingIndex = 0;
#endif
    double frameTiming[9] = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    int traceFrameTiming = CTH_LOG_ENABLED(CTH_LOG_TRACE);
    if (traceFrameTiming)
        frameTiming[0] = getTime();

    PROFILING();
    cthughaDisplay->nextFrame();
    if (traceFrameTiming)
        frameTiming[1] = getTime();

    PROFILING();
    audioFrameTick();
    if (traceFrameTiming)
        frameTiming[2] = getTime();

    PROFILING();
    audioAnalyzer();
    if (traceFrameTiming)
        frameTiming[3] = getTime();

    PROFILING();
    (*autoChanger)();
    if (traceFrameTiming)
        frameTiming[4] = getTime();

    PROFILING();
    (*soundServer)();
    if (traceFrameTiming)
        frameTiming[5] = getTime();

    PROFILING();
    CthughaBuffer::run();
    if (traceFrameTiming)
        frameTiming[6] = getTime();

    PROFILING();
    double visualStart = getTime();
    if (doDisplay)
        (*cthughaDisplay)();
    double visualEnd = getTime();
    if (traceFrameTiming)
        frameTiming[7] = visualEnd;
    if (cthughaDisplay)
        cthughaDisplay->observeVisualLatency(visualEnd - visualStart);

    PROFILING();
    (*cdPlayer)();
    if (traceFrameTiming) {
        frameTiming[8] = getTime();
        CTH_TRACE("total-ms=%.3f next-frame=%.3f audio=%.3f analyze=%.3f auto=%.3f server=%.3f buffer=%.3f display=%.3f cd=%.3f do-display=%d\n",
            "frame timing",
            (frameTiming[8] - frameTiming[0]) * 1000.0,
            (frameTiming[1] - frameTiming[0]) * 1000.0,
            (frameTiming[2] - frameTiming[1]) * 1000.0,
            (frameTiming[3] - frameTiming[2]) * 1000.0,
            (frameTiming[4] - frameTiming[3]) * 1000.0,
            (frameTiming[5] - frameTiming[4]) * 1000.0,
            (frameTiming[6] - frameTiming[5]) * 1000.0,
            (frameTiming[7] - frameTiming[6]) * 1000.0,
            (frameTiming[8] - frameTiming[7]) * 1000.0,
            doDisplay);
    }

    PROFILING();
    // Suspend only between frame stages, after graphics operations are done.
    if (cthugha_pause) {
        cthugha_pause = 0;

        exit_sound();

        raise(SIGTSTP);
    }

    PROFILING();

#ifdef PROF
    static double Ts[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    for (int i = 0; i < 8; i++)
        Ts[i] = Ts[i] + T[i + 1] - T[i];

    static int n = 24;
    n++;
    if (n == 25) {
        static double To = 0;
        double to = getTime();

        printf("%6.4f: ", to - To);
        To = to;

        for (int i = 0; i < 8; i++) {
            printf("%6.4f ", Ts[i]);
            Ts[i] = 0;
        }
        printf("\n");
        n = 0;
    }
#endif
}
