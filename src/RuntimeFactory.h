// Startup-time composition helpers.

#ifndef __RUNTIME_FACTORY_H
#define __RUNTIME_FACTORY_H

#include "PcmSourceFactory.h"
#include "Settings.h"

class Environment {
public:
    int ossInputAvailable;
    int ossOutputAvailable;
    int pulseOutputAvailable;

    Environment();

    static Environment detect();
};

class RuntimeFactory {
    Settings settings;
    Environment environment;
    PcmSourceFactory pcmSourceFactory;

public:
    RuntimeFactory(const Settings& settings, const Environment& environment);

    AudioInput* createAudioInput() const;
    AudioOutput* createAudioOutput() const;
    AudioInputProcessor* createAudioProcessor() const;
    AudioSourceStrategy selectAudioSourceStrategy() const;
};

#endif
