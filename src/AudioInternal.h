// Private helpers shared by the audio implementation units.

#ifndef CTHUGHA_AUDIO_INTERNAL_H
#define CTHUGHA_AUDIO_INTERNAL_H

struct PcmFormat;

/**
 * Converts the largest logical visual dimension into the audio sample window.
 *
 * @param visualMaxDimension Maximum logical visual-buffer dimension, in pixels,
 *        before display zoom.
 * @return Power-of-two sample window used by OSS/DSP and input processors.
 */
int audioSampleWindowForVisualMaxDimension(int visualMaxDimension);

/**
 * Emits bounded debug detail for PCM submitted to an output backend.
 *
 * @param format PCM format represented by scratch.
 * @param scratch PCM bytes submitted to output.
 * @param samples Number of sample frames represented in scratch.
 * @param bytes Number of bytes represented in scratch.
 * @param written Number of bytes accepted by the backend.
 * @param queuedSamples Remaining queued sample frames after submission.
 * @param submittedEndSample Absolute sample-frame position after submission.
 */
void audioDebugSubmittedPcm(const PcmFormat& format, const char* scratch,
    int samples, int bytes, int written, int queuedSamples,
    long long submittedEndSample);

#endif
