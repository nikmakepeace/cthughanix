#include "FrameRateMeter.h"

FrameRateMeter::FrameRateMeter(int rollingFrameCapacity)
    : frameDurations(rollingFrameCapacity > 0 ? rollingFrameCapacity : 1, 0.0)
    , nextFrameDuration(0)
    , frameDurationCount(0)
    , rollingDurationSeconds(0.0)
    , framesPerSecondValue(0.0) {
}

void FrameRateMeter::observeFrameDuration(double seconds) {
    if (seconds <= 0.0) {
        framesPerSecondValue = 0.0;
        return;
    }

    framesPerSecondValue = 1.0 / seconds;

    if (frameDurationCount < int(frameDurations.size())) {
        frameDurations[frameDurationCount] = seconds;
        frameDurationCount++;
    } else {
        rollingDurationSeconds -= frameDurations[nextFrameDuration];
        frameDurations[nextFrameDuration] = seconds;
    }

    rollingDurationSeconds += seconds;
    nextFrameDuration = (nextFrameDuration + 1) % int(frameDurations.size());
}

double FrameRateMeter::framesPerSecond() const {
    return framesPerSecondValue;
}

double FrameRateMeter::rollingFramesPerSecond() const {
    if (rollingDurationSeconds <= 0.0 || frameDurationCount <= 0)
        return 0.0;

    return double(frameDurationCount) / rollingDurationSeconds;
}
