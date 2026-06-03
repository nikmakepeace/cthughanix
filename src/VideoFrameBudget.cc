#include "VideoFrameBudget.h"

int videoFrameBudgetFramesPerSecond(int configuredMaxFramesPerSecond,
    double rollingFramesPerSecond) {
    if (configuredMaxFramesPerSecond > 0)
        return configuredMaxFramesPerSecond;

    if (rollingFramesPerSecond > 0.0)
        return int(rollingFramesPerSecond);

    return 60;
}
