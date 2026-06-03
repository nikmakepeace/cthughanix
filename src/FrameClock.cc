#include "FrameClock.h"
#include "cthugha.h"

double SystemFrameTimeSource::seconds() {
    return getTime();
}

FrameClock::FrameClock(FrameTimeSource& timeSource_)
    : timeSource(timeSource_)
    , nowValue(0.0)
    , deltaValue(0.0) {
}

void FrameClock::beginFrame() {
    double previous = nowValue;
    nowValue = timeSource.seconds();
    deltaValue = nowValue - previous;
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

void FrameClock::publish(double& nowAlias, double& deltaAlias) const {
    nowAlias = nowValue;
    deltaAlias = deltaValue;
}
