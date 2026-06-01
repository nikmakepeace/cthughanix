// -*- c++ -*-

#ifndef __AUDIO_ANALYZER_H
#define __AUDIO_ANALYZER_H

#include "cthugha.h"
#include "AudioTypes.h"

#include "CoreOption.h"

extern OptionInt sound_minnoise; /* quiet is below this */

struct AudioMetrics {
    int amplitude;
    int amplitudeLeft;
    int amplitudeRight;
    int noisy;

    AudioMetrics();
};

class AcousticContext {
    double intensityValue;
    int lastAmplitudeValue;
    int attackLevelValue;
    int fireValue;
    int cumulativeFireLevelValue;

public:
    AcousticContext();

    void update(const AudioMetrics& metrics);
    double intensity() const;
    int fire() const;
    int cumulativeFireLevel() const;
    void resetCumulativeFireLevel();
};

class AudioAnalyzer {
public:
    AudioAnalyzer();

    AudioMetrics analyze(const char2* frame);
    void operator()();
};

extern AudioAnalyzer audioAnalyzer;
extern AudioMetrics audioMetrics;
extern AcousticContext acousticContext;

#endif
