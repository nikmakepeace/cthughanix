// -*- c++ -*-

#ifndef __SOUND_ANALYZE_H
#define __SOUND_ANALYZE_H

#include "sound.h"

#include "CoreOption.h"

extern OptionInt sound_minnoise;		/* quiet is below this */

class SoundAnalyze {
   
public:
    SoundAnalyze();

    void operator()();		// does noiselvel checking, massage, fft, ...
    
    int amplitude;		// sound amplitude (variance)
    int amplitudeLeft;
    int amplitudeRight;
    int noisy;			// there is some sound
    int attackLevel;		// attack level
    int fire;			// fired now
    int fireLevel;		// accumulated fire (for change)

    double intensity;		// smoothed and normalized amplitude

    double speed;
};

extern SoundAnalyze soundAnalyze;

#endif

