#include "FrameClock.h"

#include <assert.h>
#include <math.h>

static void assertNear(double actual, double expected) {
    assert(fabs(actual - expected) < 0.000001);
}

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
    assert(clock.deltaT() == 0.0);

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
    assert(deltaAlias == 0.0);
}

static void testSampleUsesInjectedTimeSourceWithoutAdvancingFrame() {
    FakeTimeSource timeSource(30.0);
    FrameClock clock(timeSource);

    assert(clock.sample() == 30.0);
    assert(clock.now() == 0.0);
    assert(clock.deltaT() == 0.0);
}

static void testFrameRateReadoutsComeFromCompletedFrameDurations() {
    FakeTimeSource timeSource(40.0);
    FrameClock clock(timeSource);

    clock.beginFrame();
    assertNear(clock.framesPerSecond(), 0.0);
    assertNear(clock.rollingFramesPerSecond(), 0.0);

    timeSource.value = 40.040;
    clock.beginFrame();
    assertNear(clock.framesPerSecond(), 25.0);
    assertNear(clock.rollingFramesPerSecond(), 25.0);

    timeSource.value = 40.090;
    clock.beginFrame();
    assertNear(clock.framesPerSecond(), 20.0);
    assertNear(clock.rollingFramesPerSecond(), 2.0 / 0.090);
}

int main() {
    testBeginFramePublishesNowAndDelta();
    testPublishUpdatesLegacyAliases();
    testSampleUsesInjectedTimeSourceWithoutAdvancingFrame();
    testFrameRateReadoutsComeFromCompletedFrameDurations();
    return 0;
}
