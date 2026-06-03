#ifndef __FRAME_RATE_METER_H
#define __FRAME_RATE_METER_H

#include <vector>

class FrameRateMeter {
    std::vector<double> frameDurations;
    int nextFrameDuration;
    int frameDurationCount;
    double rollingDurationSeconds;
    double framesPerSecondValue;

public:
    explicit FrameRateMeter(int rollingFrameCapacity = 30);

    void observeFrameDuration(double seconds);
    double framesPerSecond() const;
    double rollingFramesPerSecond() const;
};

#endif
