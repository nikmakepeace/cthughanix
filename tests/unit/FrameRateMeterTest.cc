#include "FrameRateMeter.h"

#include <assert.h>
#include <math.h>

static void assertNear(double actual, double expected) {
    assert(fabs(actual - expected) < 0.000001);
}

static void testStartsWithNoFrameRate() {
    FrameRateMeter meter(3);

    assertNear(meter.framesPerSecond(), 0.0);
    assertNear(meter.rollingFramesPerSecond(), 0.0);
}

static void testReportsInstantFrameRateFromLatestFrameDuration() {
    FrameRateMeter meter(3);

    meter.observeFrameDuration(0.040);
    assertNear(meter.framesPerSecond(), 25.0);

    meter.observeFrameDuration(0.050);
    assertNear(meter.framesPerSecond(), 20.0);
}

static void testReportsRollingFrameRateAcrossRecentFrameDurations() {
    FrameRateMeter meter(3);

    meter.observeFrameDuration(0.040);
    meter.observeFrameDuration(0.050);
    meter.observeFrameDuration(0.100);
    assertNear(meter.rollingFramesPerSecond(), 3.0 / 0.190);

    meter.observeFrameDuration(0.025);
    assertNear(meter.rollingFramesPerSecond(), 3.0 / 0.175);
}

static void testInvalidFrameDurationsClearInstantRateButDoNotPoisonRollingRate() {
    FrameRateMeter meter(3);

    meter.observeFrameDuration(0.040);
    meter.observeFrameDuration(0.0);

    assertNear(meter.framesPerSecond(), 0.0);
    assertNear(meter.rollingFramesPerSecond(), 25.0);
}

int main() {
    testStartsWithNoFrameRate();
    testReportsInstantFrameRateFromLatestFrameDuration();
    testReportsRollingFrameRateAcrossRecentFrameDurations();
    testInvalidFrameDurationsClearInstantRateButDoNotPoisonRollingRate();
    return 0;
}
