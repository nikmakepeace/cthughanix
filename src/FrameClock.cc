#include "FrameClock.h"
#include "cthugha.h"

double SystemFrameTimeSource::seconds() {
    return getTime();
}

FrameClock::FrameClock(FrameTimeSource& timeSource_)
    : timeSource(timeSource_)
    , nowValue(0.0)
    , deltaValue(0.0)
    , hasFrameValue(0)
    , frameRateMeter() {
}

void FrameClock::beginFrame() {
    double previous = nowValue;
    nowValue = timeSource.seconds();
    if (hasFrameValue) {
        deltaValue = nowValue - previous;
        frameRateMeter.observeFrameDuration(deltaValue);
    } else {
        deltaValue = 0.0;
        hasFrameValue = 1;
    }
}

double FrameClock::sample() {
    return timeSource.seconds();
}

double FrameClock::now() const {
    return nowValue;
}

double FrameClock::deltaT() const {
    return deltaValue;
}

double FrameClock::framesPerSecond() const {
    return frameRateMeter.framesPerSecond();
}

double FrameClock::rollingFramesPerSecond() const {
    return frameRateMeter.rollingFramesPerSecond();
}

void FrameClock::publish(double& nowAlias, double& deltaAlias) const {
    nowAlias = nowValue;
    deltaAlias = deltaValue;
}
