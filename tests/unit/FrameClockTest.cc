#include "FrameClock.h"

#include <assert.h>

double getTime() {
    return -1.0;
}

class FakeTimeSource : public FrameTimeSource {
public:
    double value;
    int calls;

    explicit FakeTimeSource(double value_)
        : value(value_)
        , calls(0) {
    }

    virtual double seconds() {
        calls++;
        return value;
    }
};

static void testBeginFramePublishesNowAndDelta() {
    FakeTimeSource timeSource(10.0);
    FrameClock clock(timeSource);

    clock.beginFrame();

    assert(timeSource.calls == 1);
    assert(clock.now() == 10.0);
    assert(clock.deltaT() == 10.0);

    timeSource.value = 10.25;
    clock.beginFrame();

    assert(timeSource.calls == 2);
    assert(clock.now() == 10.25);
    assert(clock.deltaT() == 0.25);
}

static void testPublishUpdatesLegacyAliases() {
    FakeTimeSource timeSource(20.0);
    FrameClock clock(timeSource);
    double nowAlias = -1.0;
    double deltaAlias = -1.0;

    clock.beginFrame();
    clock.publish(nowAlias, deltaAlias);

    assert(nowAlias == 20.0);
    assert(deltaAlias == 20.0);
}

static void testSampleUsesInjectedTimeSourceWithoutAdvancingFrame() {
    FakeTimeSource timeSource(30.0);
    FrameClock clock(timeSource);

    assert(clock.sample() == 30.0);
    assert(clock.now() == 0.0);
    assert(clock.deltaT() == 0.0);
}

int main() {
    testBeginFramePublishesNowAndDelta();
    testPublishUpdatesLegacyAliases();
    testSampleUsesInjectedTimeSourceWithoutAdvancingFrame();
    return 0;
}
