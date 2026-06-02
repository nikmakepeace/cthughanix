// Private helpers shared by the audio implementation units.

#ifndef CTHUGHA_AUDIO_INTERNAL_H
#define CTHUGHA_AUDIO_INTERNAL_H

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
 * @param scratch PCM bytes submitted to output.
 * @param samples Number of sample frames represented in scratch.
 * @param bytes Number of bytes represented in scratch.
 * @param written Number of bytes accepted by the backend.
 * @param queuedSamples Remaining queued sample frames after submission.
 * @param submittedEndSample Absolute sample-frame position after submission.
 */
void audioDebugSubmittedPcm(const char* scratch, int samples, int bytes, int written,
    int queuedSamples, long long submittedEndSample);

/**
 * Optionally appends submitted PCM to the configured WAV dump.
 *
 * @param data PCM bytes in the current output sound format.
 * @param bytes Number of bytes available at data.
 */
void audioOutputDumpSubmittedPcm(const char* data, int bytes);

#endif
