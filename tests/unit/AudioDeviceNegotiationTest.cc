/** @file
 * Unit coverage for source/output-local PCM format negotiation.
 */

#include "Audio.h"
#include "AudioSettings.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <unistd.h>

int cth_log_enabled(int) { return 0; }
int cth_log(int, const char*, ...) { return 0; }
int cth_log_context(int, const char*, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }
int cth_log_errno(int, const char*, ...) { return 0; }

double getTime() { return 0.0; }
const char* audioSampleFormatText() { return "test-format"; }
const char* audioSampleFormatText(int) { return "test-format"; }

int init_mixer() { return 0; }

static std::string fixturePath(const char* fileName) {
    return std::string(CTH_AUDIO_FIXTURE_DIR) + "/" + fileName;
}

static PcmFormat formatFor(int sampleRate, int channels, int sampleFormat) {
    PcmFormat format;
    format.sampleRate = sampleRate;
    format.channels = channels;
    format.sampleFormat = sampleFormat;
    return format;
}

static void assertFormatEquals(const PcmFormat& actual,
    const PcmFormat& expected) {
    assert(actual.sampleRate == expected.sampleRate);
    assert(actual.channels == expected.channels);
    assert(actual.sampleFormat == expected.sampleFormat);
}

static void testWavSourcePublishesHeaderFormat() {
    WavPcmSource source(fixturePath("sine-50-1600-doubling-4s.wav").c_str());

    assert(!source.hasError());
    assertFormatEquals(source.format(), formatFor(44100, 2, SF_s16_le));
}

static void testMp3SourcePublishesDecoderFormatWhenAvailable() {
    Minimp3PcmSource source(fixturePath("prism.mp3").c_str());

    if (source.hasError())
        return;

    assertFormatEquals(source.format(), formatFor(48000, 2, SF_s16_le));
}

static void testRawSourceUsesExplicitFormat() {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "/tmp/cthughanix-raw-format-%ld.raw",
        (long)getpid());

    FILE* file = fopen(path, "wb");
    assert(file != NULL);
    const unsigned char data[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    assert(fwrite(data, 1, sizeof(data), file) == sizeof(data));
    fclose(file);

    PcmFormat requested = formatFor(22050, 1, SF_u8);
    RawPcmSource source(path, requested);
    unlink(path);

    assert(!source.hasError());
    assertFormatEquals(source.format(), requested);
}

static void testRandomNoiseKeepsSessionRateAndOwnSampleShape() {
    RandomNoisePcmSource source(formatFor(32000, 1, SF_s16_le));

    assert(!source.hasError());
    assertFormatEquals(source.format(), formatFor(32000, 2, SF_u8));
}

static void testDspSourceKeepsRequestedFormatLocallyWhenOpenFails() {
    AudioSettings settings;
    settings.pcmFormat = formatFor(48000, 1, SF_s16_le);
    settings.soundDSPMethod = 0;
    settings.dspFragments = 8;
    settings.dspFragmentSize = 9;
    settings.dspSyncEnabled = 1;
    strncpy(settings.dspDevicePath, "/tmp/cthughanix-no-such-dsp", PATH_MAX);
    settings.dspDevicePath[PATH_MAX - 1] = '\0';

    DspPcmSource source(settings, 256);

    assert(source.hasError());
    assertFormatEquals(source.format(), settings.pcmFormat);
}

static void testDspOutputNegotiatesWithoutGlobalSetters() {
    AudioSettings settings;
    settings.pcmFormat = formatFor(48000, 2, SF_s16_le);
    settings.soundDSPMethod = 0;
    settings.dspFragments = 8;
    settings.dspFragmentSize = 9;
    strncpy(settings.dspDevicePath, "/tmp/cthughanix-no-such-dsp", PATH_MAX);
    settings.dspDevicePath[PATH_MAX - 1] = '\0';

    AudioDSPOutput output(settings, 256);

    assert(!output.isOpen());
}

int main() {
    testWavSourcePublishesHeaderFormat();
    testMp3SourcePublishesDecoderFormatWhenAvailable();
    testRawSourceUsesExplicitFormat();
    testRandomNoiseKeepsSessionRateAndOwnSampleShape();
    testDspSourceKeepsRequestedFormatLocallyWhenOpenFails();
    testDspOutputNegotiatesWithoutGlobalSetters();
    return 0;
}
