#ifndef __FRAME_CLOCK_H
#define __FRAME_CLOCK_H

#include "FrameRateMeter.h"

class FrameTimeSource {
public:
    virtual ~FrameTimeSource() { }
    virtual double seconds() = 0;
};

class SystemFrameTimeSource : public FrameTimeSource {
public:
    virtual double seconds();
};

class FrameClock {
    FrameTimeSource& timeSource;
    double nowValue;
    double deltaValue;
    int hasFrameValue;
    FrameRateMeter frameRateMeter;

public:
    explicit FrameClock(FrameTimeSource& timeSource_);

    void beginFrame();
    double sample();
    double now() const;
    double deltaT() const;
    double framesPerSecond() const;
    double rollingFramesPerSecond() const;
    void publish(double& nowAlias, double& deltaAlias) const;
};

#endif
