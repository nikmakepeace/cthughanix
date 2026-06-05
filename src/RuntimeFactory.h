/** @file
 * Startup-time runtime composition helpers.
 */

#ifndef __RUNTIME_FACTORY_H
#define __RUNTIME_FACTORY_H

#include "PcmSourceFactory.h"
#include "AudioSettings.h"

class Environment {
public:
    int ossInputAvailable;
    int ossOutputAvailable;
    int pulseOutputAvailable;

    /** Creates an environment with no detected backend availability. */
    Environment();

    /**
     * Detects audio backend availability for this process.
     *
     * @param settings Startup/session settings whose device paths should be
     *        checked.
     * @return Environment describing available input/output backends.
     */
    static Environment detect(const AudioSettings& settings);
};

class RuntimeFactory {
    AudioSettings settings;
    Environment environment;
    int visualMaxDimension;
    PcmSourceFactory pcmSourceFactory;

public:
    /**
     * Creates a factory for one startup/runtime-selection pass.
     *
     * @param settings Snapshot of audio options used to select input/output.
     * @param environment Detected backend availability for this process.
     * @param visualMaxDimension Maximum logical visual-buffer dimension, in pixels,
     *        before display zoom. Passed to DSP/audio-window constructors.
     */
    RuntimeFactory(const AudioSettings& settings, const Environment& environment,
        int visualMaxDimension);

    /**
     * @return Newly allocated audio input wrapper. Caller owns the returned pointer.
     */
    AudioInput* createAudioInput() const;

    /**
     * @param format PCM format produced by the selected input/session.
     * @return Newly allocated audio output backend. Caller owns the returned pointer.
     */
    AudioOutput* createAudioOutput(const PcmFormat& format) const;

    /**
     * @return Strategy selected from the current settings and environment.
     */
    AudioSourceStrategy selectAudioSourceStrategy() const;
};

#endif
