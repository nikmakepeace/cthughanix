// Private helper implementations shared across the audio split units.

#include "cthugha.h"
#include "AudioInternal.h"
#include "Audio.h"
#include "imath.h"

static int readSigned16Le(const unsigned char* p) {
    unsigned int value = (unsigned int)p[0] | ((unsigned int)p[1] << 8);
    return (value & 0x8000) ? (int)value - 0x10000 : (int)value;
}

static int readSigned16Be(const unsigned char* p) {
    unsigned int value = ((unsigned int)p[0] << 8) | (unsigned int)p[1];
    return (value & 0x8000) ? (int)value - 0x10000 : (int)value;
}

static unsigned int readUnsigned16Le(const unsigned char* p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

static unsigned int readUnsigned16Be(const unsigned char* p) {
    return ((unsigned int)p[0] << 8) | (unsigned int)p[1];
}

int audioSampleWindowForVisualMaxDimension(int visualMaxDimension) {
    if (visualMaxDimension < 1)
        visualMaxDimension = 160;

    return 1 << ilog2(visualMaxDimension);
}

static int audioPcmPeak(const PcmFormat& format, const char* data, int samples) {
    const unsigned char* bytes = (const unsigned char*)data;
    int peak = 0;
    int channels = format.channels;
    if ((data == NULL) || (samples <= 0) || (channels <= 0))
        return 0;

    switch (format.sampleFormat) {
    case SF_u8:
        for (int i = 0; i < samples * channels; i++) {
            int sample = (int)bytes[i] - 128;
            peak = max(peak, abs(sample));
        }
        break;
    case SF_s8:
        for (int i = 0; i < samples * channels; i++)
            peak = max(peak, abs((int)((const signed char*)data)[i]));
        break;
    case SF_u16_le:
        for (int i = 0; i < samples * channels; i++) {
            int sample = (int)readUnsigned16Le(bytes + i * 2) - 32768;
            peak = max(peak, abs(sample));
        }
        break;
    case SF_s16_le:
        for (int i = 0; i < samples * channels; i++)
            peak = max(peak, abs(readSigned16Le(bytes + i * 2)));
        break;
    case SF_u16_be:
        for (int i = 0; i < samples * channels; i++) {
            int sample = (int)readUnsigned16Be(bytes + i * 2) - 32768;
            peak = max(peak, abs(sample));
        }
        break;
    case SF_s16_be:
        for (int i = 0; i < samples * channels; i++)
            peak = max(peak, abs(readSigned16Be(bytes + i * 2)));
        break;
    default:
        break;
    }

    return peak;
}

void audioDebugSubmittedPcm(const PcmFormat& format, const char* scratch,
    int samples, int bytes, int written, int queuedSamples,
    long long submittedEndSample) {
    static int reports = 0;

    if (!CTH_LOG_ENABLED(CTH_LOG_DEBUG) || (reports >= 8))
        return;

    reports++;

    CTH_DEBUG("    audio output: submitted samples=%d bytes=%d written=%d peak=%d queued-samples=%d submitted-end-sample=%lld\n",
        samples, bytes, written, audioPcmPeak(format, scratch, samples), queuedSamples,
        submittedEndSample);
}
