// Platform lifecycle hooks.

#include "PlatformLifecycle.h"

#include "cthugha.h"

#include <signal.h>

#if defined(SIGTSTP) && !defined(_WIN32)
#define CTH_HAVE_JOB_CONTROL 1
#include <string.h>

static volatile sig_atomic_t suspendRequested = 0;
static int suspendHandlerInstalled = 0;
static int previousSuspendActionValid = 0;
static struct sigaction previousSuspendAction;

static void suspendSignalHandler(int) {
    suspendRequested = 1;
}

static int installSuspendSignalHandler() {
    if (suspendHandlerInstalled)
        return 1;

    struct sigaction action;
    memset(&action, 0, sizeof(action));
    sigemptyset(&action.sa_mask);
    action.sa_handler = suspendSignalHandler;

    if (!previousSuspendActionValid) {
        struct sigaction currentAction;
        if (sigaction(SIGTSTP, 0, &currentAction) != 0) {
            CTH_WARN("Could not inspect SIGTSTP handler; terminal suspend disabled.\n");
            return 0;
        }

        if (currentAction.sa_handler == SIG_IGN)
            return 0;

        if (sigaction(SIGTSTP, &action, &previousSuspendAction) != 0) {
            CTH_WARN("Could not install SIGTSTP handler; terminal suspend disabled.\n");
            return 0;
        }

        previousSuspendActionValid = 1;
    } else {
        if (sigaction(SIGTSTP, &action, 0) != 0) {
            CTH_WARN("Could not reinstall SIGTSTP handler; terminal suspend disabled.\n");
            return 0;
        }
    }

    suspendHandlerInstalled = 1;
    return 1;
}

static void restoreSuspendSignalHandler() {
    if (!suspendHandlerInstalled)
        return;

    if (previousSuspendActionValid)
        sigaction(SIGTSTP, &previousSuspendAction, 0);

    previousSuspendActionValid = 0;
    suspendHandlerInstalled = 0;
}

static void useDefaultSuspendSignalHandler() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    sigemptyset(&action.sa_mask);
    action.sa_handler = SIG_DFL;
    sigaction(SIGTSTP, &action, 0);
    suspendHandlerInstalled = 0;
}

static void suspendCurrentProcess() {
    useDefaultSuspendSignalHandler();
    raise(SIGTSTP);
    installSuspendSignalHandler();
}
#else
#define CTH_HAVE_JOB_CONTROL 0

static volatile int suspendRequested = 0;

static int installSuspendSignalHandler() { return 0; }
static void restoreSuspendSignalHandler() { }
static void suspendCurrentProcess() { }
#endif

static int consumeSuspendRequest() {
    if (!suspendRequested)
        return 0;

    suspendRequested = 0;
    return 1;
}

PlatformLifecycle::PlatformLifecycle(const PlatformLifecycleCallbacks& callbacks_)
    : callbacks(callbacks_)
    , installed(0) { }

PlatformLifecycle::~PlatformLifecycle() {
    shutdown();
}

void PlatformLifecycle::install() {
    installed = installSuspendSignalHandler();
}

void PlatformLifecycle::shutdown() {
    if (!installed)
        return;

    restoreSuspendSignalHandler();
    installed = 0;
}

void PlatformLifecycle::requestSuspend() {
    requestApplicationSuspend();
}

void PlatformLifecycle::serviceFrameBoundary() {
    if (!consumeSuspendRequest())
        return;

    if (!installed) {
        CTH_DEBUG("Suspend requested, but this platform has no active job-control suspend hook.\n");
        return;
    }

#if CTH_HAVE_JOB_CONTROL
    CTH_INFO("Stopping...\n");
    if (callbacks.willSuspend != 0)
        callbacks.willSuspend(callbacks.context);

    suspendCurrentProcess();

    CTH_INFO("Continuing...\n");
    if (callbacks.didResume != 0)
        callbacks.didResume(callbacks.context);
#endif
}

void requestApplicationSuspend() {
    suspendRequested = 1;
}
