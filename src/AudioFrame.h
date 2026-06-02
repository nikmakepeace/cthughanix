// Current audio-frame facade.
//
// This keeps visual/audio consumers on the frame facade instead of runtime
// implementation details.

#ifndef __AUDIO_FRAME_H
#define __AUDIO_FRAME_H

#include "AudioTypes.h"

class AudioFrame {
public:
    /** Absolute sample-frame position at the center of raw/processedWaveData. */
    long long centerSample;

    /** Number of valid stereo sample frames in raw and processedWaveData. */
    int samples;

    /** Signed 8-bit stereo samples used by visualization and analysis. */
    char2 raw[1024];

    /** Signed 8-bit stereo samples after the selected AudioProcessingOption. */
    char2 processedWaveData[1024];

    AudioFrame();

    /** Resets sample positions and zeroes raw/processed audio data. */
    void clear();
};

/**
 * Advances the published audio frame to the current visual-frame time.
 *
 * Services AudioRuntime and updates the AudioFrame returned by audioFrameCurrent().
 * Called once per visual frame before AudioVisualBridge::runFrame().
 */
void audioFrameTick();

/**
 * Reconfigures audio frame processing after audio options change.
 *
 * Used by option setters and sound reset actions to rebuild processor buffers
 * without tearing down the whole application.
 */
void audioFrameChange();

/**
 * @return Current audio frame, or NULL when the runtime has not published one.
 */
AudioFrame* audioFrameCurrent();

/**
 * @return Current raw signed 8-bit stereo frame data. Falls back to legacy
 *         processor storage when no AudioFrame facade is available.
 */
char2* audioFrameRawData();

/**
 * @return Current processed signed 8-bit stereo frame data. Falls back to legacy
 *         processor storage when no AudioFrame facade is available.
 */
char2* audioFrameProcessedWaveData();

/**
 * @return Number of bytes in the current broadcast audio frame.
 */
int audioFrameBroadcastBytes();

#ifdef CTH_AUDIO_FRAME_TEST_OVERRIDE
/**
 * Installs a test-only current-frame override.
 *
 * @param frame Frame pointer to publish during tests, or NULL to clear.
 */
void audioFrameSetTestOverride(AudioFrame* frame);
#endif

#endif
