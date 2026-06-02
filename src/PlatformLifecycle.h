// Platform lifecycle hooks.
//
// The application owns frame boundaries and resource state.  This layer only
// translates platform-specific lifecycle requests, such as POSIX job-control
// suspend, into safe application callbacks.

#ifndef __PLATFORM_LIFECYCLE_H
#define __PLATFORM_LIFECYCLE_H

struct PlatformLifecycleCallbacks {
    /** Called at a safe frame boundary immediately before process suspension. */
    void (*willSuspend)(void*);

    /** Called after the process resumes and before frame processing continues. */
    void (*didResume)(void*);

    /** Opaque pointer passed to callbacks, usually Application*. */
    void* context;

    PlatformLifecycleCallbacks()
        : willSuspend(0)
        , didResume(0)
        , context(0) { }

    PlatformLifecycleCallbacks(void (*willSuspend_)(void*),
        void (*didResume_)(void*), void* context_)
        : willSuspend(willSuspend_)
        , didResume(didResume_)
        , context(context_) { }
};

class PlatformLifecycle {
    PlatformLifecycleCallbacks callbacks;
    int installed;

public:
    explicit PlatformLifecycle(const PlatformLifecycleCallbacks& callbacks_ =
            PlatformLifecycleCallbacks());
    ~PlatformLifecycle();

    /** Installs supported platform hooks, such as POSIX SIGTSTP handling. */
    void install();

    /** Removes installed platform hooks. Safe to call more than once. */
    void shutdown();

    /** Requests a suspend to be serviced at the next frame boundary. */
    void requestSuspend();

    /**
     * Services pending lifecycle work between frame stages.
     *
     * Application calls this after runFrame() has finished mutating audio,
     * video, and display state so suspend/resume callbacks cannot interrupt
     * partially updated runtime objects.
     */
    void serviceFrameBoundary();
};

/** Requests application suspension through the process-wide lifecycle hook. */
void requestApplicationSuspend();

#endif
