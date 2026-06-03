#include "VideoFrameBudget.h"

#include <assert.h>

static void testConfiguredMaxFramesPerSecondWins() {
    assert(videoFrameBudgetFramesPerSecond(25, 59.8) == 25);
}

static void testRollingFramesPerSecondFeedsUncappedBudgets() {
    assert(videoFrameBudgetFramesPerSecond(0, 48.9) == 48);
}

static void testMissingRollingFramesPerSecondFallsBackToSixty() {
    assert(videoFrameBudgetFramesPerSecond(0, 0.0) == 60);
}

int main() {
    testConfiguredMaxFramesPerSecondWins();
    testRollingFramesPerSecondFeedsUncappedBudgets();
    testMissingRollingFramesPerSecondFallsBackToSixty();
    return 0;
}
