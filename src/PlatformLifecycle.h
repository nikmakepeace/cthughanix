// Platform lifecycle hooks.
//
// The application owns frame boundaries and resource state.  This layer only
// translates platform-specific lifecycle requests, such as POSIX job-control
// suspend, into safe application callbacks.

#ifndef __PLATFORM_LIFECYCLE_H
#define __PLATFORM_LIFECYCLE_H

struct PlatformLifecycleCallbacks {
    void (*willSuspend)(void*);
    void (*didResume)(void*);
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

    void install();
    void shutdown();
    void requestSuspend();
    void serviceFrameBoundary();
};

void requestApplicationSuspend();

#endif
