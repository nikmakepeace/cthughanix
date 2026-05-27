#include "cthugha.h"
#include "Audio.h"
#include "Sound.h"
#include "imath.h"
#include "cth_buffer.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifndef WITH_MINIMP3
#define WITH_MINIMP3 1
#endif

#if WITH_PULSE == 1
#include <pulse/error.h>
#include <pulse/simple.h>
#endif

#if WITH_MINIMP3 == 1
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#ifndef UINT64_MAX
#define UINT64_MAX ((uint64_t)-1)
#endif
#define MINIMP3_NO_SIMD
#define MINIMP3_IMPLEMENTATION
#include "../external/minimp3/minimp3_ex.h"
#endif

#if WITH_DSP == 1
#ifdef HAVE_LINUX_SOUNDCARD_H
#include <linux/soundcard.h>
#else
#include <sys/soundcard.h>
#endif
#include <sys/mman.h>
#ifndef MAP_FILE
#define MAP_FILE 0
#endif
#endif

char pulse_server[PATH_MAX] = "";

const char* pulse_server_name() {
    return (pulse_server[0] != '\0') ? pulse_server : NULL;
}

AudioOutput::AudioOutput()
    : outputSamplesPerSecond(0)
    , outputBytesPerSample(1)
    , outputTargetDelaySamples(0)
    , outputScratchSamples(0) { }

AudioOutput::~AudioOutput() { }

int AudioOutput::timingScratchSamples(int inputChunkSamples, int targetDelaySamples) const {
    return isRealtime() ? targetDelaySamples : inputChunkSamples;
}

void AudioOutput::configureTiming(int samplesPerSecond, int bytesPerSample, int inputChunkSamples) {
    outputSamplesPerSecond = samplesPerSecond;
    outputBytesPerSample = bytesPerSample;
    int targetLatencyMs = defaultTargetLatencyMs();
    outputScratchSamples = inputChunkSamples;

    if (outputBytesPerSample < 1)
        outputBytesPerSample = 1;
    if (outputScratchSamples < 1)
        outputScratchSamples = 1;
    if (targetLatencyMs < 0)
        targetLatencyMs = 0;

    outputTargetDelaySamples = (outputSamplesPerSecond * targetLatencyMs) / 1000;
    if (outputTargetDelaySamples < outputScratchSamples)
        outputTargetDelaySamples = outputScratchSamples;

    outputScratchSamples = timingScratchSamples(inputChunkSamples, outputTargetDelaySamples);

    if (outputScratchSamples < 1)
        outputScratchSamples = inputChunkSamples;
    if (outputTargetDelaySamples < 1)
        outputTargetDelaySamples = outputScratchSamples;

    CTH_TRACE("configured timing realtime=%d samples-per-second=%d bytes-per-sample=%d input-chunk-samples=%d target-latency-ms=%d target-delay-samples=%d scratch-samples=%d\n",
        "audio output", isRealtime(), outputSamplesPerSecond, outputBytesPerSample,
        inputChunkSamples, targetLatencyMs, outputTargetDelaySamples, outputScratchSamples);
}

int AudioOutput::outputDelaySamples() const {
    int delayBytes = outputDelayBytes();
    return (outputBytesPerSample > 0) ? delayBytes / outputBytesPerSample : 0;
}

int AudioOutput::queuedTargetSamples() const {
    return isRealtime() ? outputTargetDelaySamples : outputScratchSamples;
}

long long AudioOutput::audibleSamplePosition(const AudioBuffer& buffer) const {
    long long submittedEndSample = buffer.submittedEndPosition();
    int delay = outputDelaySamples();

    if (delay > submittedEndSample)
        return 0;

    return submittedEndSample - delay;
}

int AudioOutput::playbackComplete(const AudioBuffer& buffer, int inputFinished) const {
    return inputFinished && (buffer.queuedForOutputSamples() == 0)
        && (!isRealtime() || (outputDelayBytes() == 0));
}

int AudioNullOutput::defaultTargetLatencyMs() const { return 0; }
int AudioNullOutput::write(const void*, int size) { return size; }
int AudioNullOutput::outputDelayBytes() const { return 0; }
int AudioNullOutput::isOpen() const { return 1; }
int AudioNullOutput::isRealtime() const { return 0; }

#if WITH_PULSE == 1

static pa_sample_format_t audioPulseSampleFormat() {
    switch (soundFormat) {
    case SF_u8:
        return PA_SAMPLE_U8;
    case SF_s16_le:
        return PA_SAMPLE_S16LE;
    case SF_s16_be:
        return PA_SAMPLE_S16BE;
    default:
        return PA_SAMPLE_INVALID;
    }
}

AudioPulseOutput::AudioPulseOutput()
    : pulse(NULL)
    , bytesPerSecondValue(0) {
    update();
}

AudioPulseOutput::~AudioPulseOutput() {
    if (pulse)
        pa_simple_free((pa_simple*)pulse);
    pulse = NULL;
}

int AudioPulseOutput::defaultTargetLatencyMs() const {
    // pa_simple_write() may block until the server accepts more data, so keep
    // a deeper sink target than the frame-sized driver chunks.
    return 250;
}

int AudioPulseOutput::timingScratchSamples(int, int targetDelaySamples) const {
    return targetDelaySamples;
}

void AudioPulseOutput::update() {
    int error;
    pa_sample_spec sampleSpec;

    if (pulse) {
        pa_simple_free((pa_simple*)pulse);
        pulse = NULL;
    }
    bytesPerSecondValue = 0;

    sampleSpec.format = audioPulseSampleFormat();
    sampleSpec.rate = int(soundSampleRate);
    sampleSpec.channels = int(soundChannels);

    if (sampleSpec.format == PA_SAMPLE_INVALID) {
        CTH_DEBUG("    audio output strategy: Pulse unavailable for format `%s'\n",
            soundFormat.text());
        return;
    }

    pulse = pa_simple_new(pulse_server_name(), "Cthughanix", PA_STREAM_PLAYBACK, NULL,
        "Audio passthrough", &sampleSpec, NULL, NULL, &error);
    if (pulse == NULL) {
        CTH_DEBUG("    audio output strategy: Pulse failed to open: %s\n", pa_strerror(error));
        return;
    }

    bytesPerSecondValue = pa_bytes_per_second(&sampleSpec);
    CTH_TRACE("opened server=`%s' rate=%d channels=%d format=%d bytes-per-second=%d\n",
        "audio pulse output", pulse_server_name() ? pulse_server_name() : "default",
        sampleSpec.rate, sampleSpec.channels, sampleSpec.format, bytesPerSecondValue);
}

int AudioPulseOutput::write(const void* buffer, int size) {
    int error;

    if (pulse == NULL)
        return 0;

    if (pa_simple_write((pa_simple*)pulse, buffer, size, &error) < 0) {
        CTH_ERROR("Pulse passthrough write failed: %s\n", pa_strerror(error));
        return 0;
    }

    return size;
}

int AudioPulseOutput::outputDelayBytes() const {
    int error;
    pa_usec_t latency;

    if ((pulse == NULL) || (bytesPerSecondValue <= 0))
        return 0;

    latency = pa_simple_get_latency((pa_simple*)pulse, &error);
    if (latency == (pa_usec_t)-1)
        return 0;

    return (int)((latency * bytesPerSecondValue) / 1000000);
}

int AudioPulseOutput::isOpen() const { return pulse != NULL; }
int AudioPulseOutput::isRealtime() const { return 1; }

#else

AudioPulseOutput::AudioPulseOutput()
    : pulse(NULL)
    , bytesPerSecondValue(0) {
    CTH_DEBUG("    audio output strategy: Pulse unavailable because support is not compiled in\n");
}

AudioPulseOutput::~AudioPulseOutput() { }
int AudioPulseOutput::defaultTargetLatencyMs() const { return 250; }
int AudioPulseOutput::timingScratchSamples(int, int targetDelaySamples) const { return targetDelaySamples; }
int AudioPulseOutput::write(const void*, int) { return 0; }
int AudioPulseOutput::outputDelayBytes() const { return 0; }
int AudioPulseOutput::isOpen() const { return 0; }
int AudioPulseOutput::isRealtime() const { return 1; }
void AudioPulseOutput::update() { }

#endif

#if WITH_DSP == 1

AudioDSPOutput::AudioDSPOutput(int method_)
    : handle(-1)
    , method(method_) {
    init();
}

int AudioDSPOutput::defaultTargetLatencyMs() const {
    // OSS latency behaviour varies with the selected snd-method and driver.
    // Keep the conservative passthrough target here, owned by the OSS output
    // path rather than by AudioRuntime.
    switch (method) {
    case 0:
    case 1:
    case 2:
    case 3:
    default:
        return 250;
    }
}

int AudioDSPOutput::timingScratchSamples(int, int targetDelaySamples) const {
    return targetDelaySamples;
}

void AudioDSPOutput::setFragment() {
    int soundDSPFragment = (int(soundDSPFragments) << 16) | int(soundDSPFragmentSize);
    if (ioctl(handle, SNDCTL_DSP_SETFRAGMENT, &soundDSPFragment) < 0)
        CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SETFRAGMENT failed.");

    soundDSPFragments.setValue(soundDSPFragment >> 16);
    soundDSPFragmentSize.setValue(soundDSPFragment & 0x7fff);
}

void AudioDSPOutput::setChannels() {
    int channels = int(soundChannels) - 1;
    if (ioctl(handle, SNDCTL_DSP_STEREO, &channels) < 0)
        CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_STEREO failed");
    soundChannels.setValue(channels + 1);
}

void AudioDSPOutput::setSampleRate() {
    if (ioctl(handle, SNDCTL_DSP_SPEED, &(soundSampleRate.value)) < 0)
        CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SPEED failed");
}

void AudioDSPOutput::setFormat() {
    int sound_format;

    switch (soundFormat) {
    case SF_u8:
        sound_format = AFMT_U8;
        break;
    case SF_s8:
        sound_format = AFMT_S8;
        break;
    case SF_u16_le:
        sound_format = AFMT_U16_LE;
        break;
    case SF_s16_le:
        sound_format = AFMT_S16_LE;
        break;
    case SF_u16_be:
        sound_format = AFMT_U16_BE;
        break;
    case SF_s16_be:
        sound_format = AFMT_S16_BE;
        break;
    default:
        CTH_ERROR("Internal error: unknown sound format.\n");
        sound_format = AFMT_U8;
    }

    int requested_sound_format = sound_format;
    if (ioctl(handle, SNDCTL_DSP_SETFMT, &sound_format) < 0) {
        CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SETFMT failed. Trying 8bit unsigned");
        sound_format = AFMT_U8;
        if (ioctl(handle, SNDCTL_DSP_SETFMT, &sound_format) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SETFMT failed.");
    }
    CTH_TRACE("set sound format requested=%d returned=%d\n", "audio dsp output",
        requested_sound_format, sound_format);

    switch (sound_format) {
    case AFMT_U8:
        soundFormat.setValue(SF_u8);
        break;
    case AFMT_S8:
        soundFormat.setValue(SF_s8);
        break;
    case AFMT_U16_LE:
        soundFormat.setValue(SF_u16_le);
        break;
    case AFMT_S16_LE:
        soundFormat.setValue(SF_s16_le);
        break;
    case AFMT_U16_BE:
        soundFormat.setValue(SF_u16_be);
        break;
    case AFMT_S16_BE:
        soundFormat.setValue(SF_s16_be);
        break;
    default:
        CTH_ERROR("Unknown sound format returned by SNDCTL_DSP_SETFMT %d.\n", sound_format);
    }
}

void AudioDSPOutput::init() {
    CTH_DEBUG("  setting %s for writing...\n", SoundDeviceDSP::dev_dsp);
    CTH_TRACE("init device=`%s' method=%d\n", "audio dsp output", SoundDeviceDSP::dev_dsp,
        method);

    if (handle >= 0)
        close(handle);
    handle = -1;

    if ((handle = open(SoundDeviceDSP::dev_dsp, O_WRONLY)) < 0) {
        CTH_ERRNO(errno, "Can't open `%s' for writing.", SoundDeviceDSP::dev_dsp);
        return;
    }

    switch (method) {
    case 0: {
        int sampleWindow = 1 << ilog2(max(BUFF_WIDTH, BUFF_HEIGHT));
        CTH_INFO("   Using sound method 0 - optimal fragment size\n");
        soundDSPFragmentSize.setValue(ilog2(sampleWindow) - 1);
        setFragment();
        setChannels();
        setSampleRate();
        setFormat();
        break;
    }

    case 1:
        CTH_INFO("   Using sound method 1 - small fragment size\n");
        soundDSPFragments.setValue(2);
        soundDSPFragmentSize.setValue(4);
        setFragment();
        setChannels();
        setSampleRate();
        setFormat();
        break;

    case 2: {
        CTH_INFO("   Using sound method 2 - old version\n");
        int sound_div = 4;
        if (ioctl(handle, SNDCTL_DSP_SUBDIVIDE, &sound_div) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SUBDIVIDE failed.");

        int dummy = 0;
        if (ioctl(handle, SNDCTL_DSP_STEREO, &dummy) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_STEREO failed.");

        dummy = 0;
        if (ioctl(handle, SNDCTL_DSP_SPEED, &dummy) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SPEED failed.");

        int sound_blkSize;
        if (ioctl(handle, SNDCTL_DSP_GETBLKSIZE, &sound_blkSize) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_GETBLKSIZE failed.");

        setChannels();
        setSampleRate();
        setFormat();

        if (ioctl(handle, SNDCTL_DSP_GETBLKSIZE, &sound_blkSize) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_GETBLKSIZE");
        break;
    }

    case 3:
        CTH_INFO("   Using sound method 3 - primitiv version\n");
        setFormat();
        setChannels();
        setSampleRate();
        break;

    case 4:
        CTH_ERROR("Sound method 4 is only available for reading sound data.\n"
                "Please use a different sound method.\n");
        close(handle);
        handle = -1;
        break;

    default:
        CTH_ERROR("Unknown sound method %d.", method);
        CTH_ERROR("   available sound methods:\n"
                "   0: sophisticated 1 (optimal fragment size)\n"
                "   1: sophisticated 2 (small fragments)\n"
                "   2: simple (small DMA buffer)\n"
                "   3: primitiv\n"
                "   4: directly use DMA buffer\n");
        close(handle);
        handle = -1;
        break;
    }
}

int AudioDSPOutput::write(const void* buffer, int size) {
    if (handle < 0)
        return 0;
    return ::write(handle, buffer, size);
}

int AudioDSPOutput::outputDelayBytes() const {
    int delay = 0;

    if (handle < 0)
        return 0;

#ifdef SNDCTL_DSP_GETODELAY
    if (ioctl(handle, SNDCTL_DSP_GETODELAY, &delay) == 0)
        return max(0, delay);
#endif

#ifdef SNDCTL_DSP_GETOSPACE
    {
        audio_buf_info bi;
        if (ioctl(handle, SNDCTL_DSP_GETOSPACE, &bi) == 0)
            return max(0, bi.fragstotal * bi.fragsize - bi.bytes);
    }
#endif

    return 0;
}

int AudioDSPOutput::getHandle() const { return handle; }
int AudioDSPOutput::isOpen() const { return handle >= 0; }
void AudioDSPOutput::update() { init(); }

AudioDSPOutput::~AudioDSPOutput() {
    if (handle >= 0)
        close(handle);
    handle = -1;
}

#else

AudioDSPOutput::AudioDSPOutput(int method_)
    : handle(-1)
    , method(method_) {
    CTH_DEBUG("    audio output strategy: OSS DSP unavailable because support is not compiled in\n");
    CTH_TRACE("unavailable method=%d\n", "audio dsp output", method);
}

AudioDSPOutput::~AudioDSPOutput() { }
int AudioDSPOutput::defaultTargetLatencyMs() const { return 250; }
int AudioDSPOutput::timingScratchSamples(int, int targetDelaySamples) const { return targetDelaySamples; }
void AudioDSPOutput::setFragment() { }
void AudioDSPOutput::setChannels() { }
void AudioDSPOutput::setSampleRate() { }
void AudioDSPOutput::setFormat() { }
void AudioDSPOutput::init() { }
int AudioDSPOutput::write(const void*, int) { return 0; }
int AudioDSPOutput::outputDelayBytes() const { return 0; }
int AudioDSPOutput::getHandle() const { return -1; }
int AudioDSPOutput::isOpen() const { return 0; }
void AudioDSPOutput::update() { }

#endif

AudioBuffer::AudioBuffer(int capacitySamples_, int bytesPerSample_, int protectedHistorySamples_)
    : data(NULL)
    , bytesPerSampleValue(bytesPerSample_)
    , capacitySamples(capacitySamples_)
    , protectedHistorySamples(protectedHistorySamples_)
    , decodedEndSample(0)
    , submittedEndSample(0) {
    if (bytesPerSampleValue < 1)
        bytesPerSampleValue = 1;
    if (capacitySamples < 1)
        capacitySamples = 1;
    if (protectedHistorySamples < 0)
        protectedHistorySamples = 0;
    if (protectedHistorySamples > capacitySamples)
        protectedHistorySamples = capacitySamples;
    data = new char[pcmBytesForSamples(capacitySamples, bytesPerSampleValue)];
    CTH_TRACE("created capacity-samples=%d bytes-per-sample=%d protected-history-samples=%d\n",
        "audio buffer", capacitySamples, bytesPerSampleValue, protectedHistorySamples);
}

AudioBuffer::~AudioBuffer() {
    delete[] data;
    data = NULL;
}

void AudioBuffer::clear() {
    decodedEndSample = 0;
    submittedEndSample = 0;
}

long long AudioBuffer::protectedWindowStartSample() const {
    // Protected span:
    //   [protectedWindowStartSample(), submittedEndSample) = recent submitted
    //       history retained for visual-frame reads.
    //   [submittedEndSample, decodedEndSample) = decoded PCM queued ahead of output.
    // Future driver writes may overwrite only data outside this span.
    long long historyStartSample = submittedEndSample - protectedHistorySamples;
    long long capacityStartSample = decodedEndSample - capacitySamples;
    long long startSample = (historyStartSample > capacityStartSample) ? historyStartSample : capacityStartSample;

    if (startSample < 0)
        startSample = 0;
    if (startSample > submittedEndSample)
        startSample = submittedEndSample;

    return startSample;
}

int AudioBuffer::copyAt(long long samplePosition, char* dst, int samples) const {
    int copiedSamples = 0;
    long long startSample = protectedWindowStartSample();

    if ((samples <= 0) || (samplePosition < startSample) || (samplePosition >= decodedEndSample))
        return 0;

    long long availableSamples = decodedEndSample - samplePosition;
    int wantedSamples = (samples < availableSamples) ? samples : int(availableSamples);

    while (copiedSamples < wantedSamples) {
        int posSample = int((samplePosition + copiedSamples) % capacitySamples);
        int chunkSamples = min(wantedSamples - copiedSamples, capacitySamples - posSample);
        memcpy(dst + pcmBytesForSamples(copiedSamples, bytesPerSampleValue),
            data + pcmBytesForSamples(posSample, bytesPerSampleValue),
            pcmBytesForSamples(chunkSamples, bytesPerSampleValue));
        copiedSamples += chunkSamples;
    }

    return copiedSamples;
}

int AudioBuffer::appendDecodedPcm(const char* src, int samples) {
    int writtenSamples = 0;
    int wantedSamples = min(samples, writableSamples());

    while (writtenSamples < wantedSamples) {
        int posSample = int((decodedEndSample + writtenSamples) % capacitySamples);
        int chunkSamples = min(wantedSamples - writtenSamples, capacitySamples - posSample);
        memcpy(data + pcmBytesForSamples(posSample, bytesPerSampleValue),
            src + pcmBytesForSamples(writtenSamples, bytesPerSampleValue),
            pcmBytesForSamples(chunkSamples, bytesPerSampleValue));
        writtenSamples += chunkSamples;
    }

    decodedEndSample += writtenSamples;

    return writtenSamples;
}

int AudioBuffer::readForOutput(char* dst, int samples) {
    int copied = copyAt(submittedEndSample, dst, samples);

    submittedEndSample += copied;

    return copied;
}

int AudioBuffer::readProtectedPcmAt(long long samplePosition, char* dst, int samples) const {
    return copyAt(samplePosition, dst, samples);
}

int AudioOutput::service(AudioBuffer& buffer, char* scratch, int scratchSamples,
    int inputFinished) {
    if ((scratch == NULL) || (scratchSamples <= 0))
        return 0;

    double serviceStart = getTime();
    int writes = 0;
    int bytesPerSample = buffer.bytesPerSample();
    int configuredTargetDelaySamples = targetDelaySamples();
    if (scratchSamples <= 0)
        return 0;

    if (!isRealtime()) {
        double bufferReadStart = getTime();
        int samples = buffer.readForOutput(scratch, scratchSamples);
        double bufferReadEnd = getTime();
        if (samples <= 0)
            return 0;

        int bytes = pcmBytesForSamples(samples, bytesPerSample);
        double outputWriteStart = getTime();
        int written = write(scratch, bytes);
        double outputWriteEnd = getTime();
        if (written > 0)
            writes++;

        double delayStart = getTime();
        int finalDelay = outputDelayBytes();
        double delayEnd = getTime();
        CTH_TRACE("output submitted samples=%d bytes=%d written=%d queued-samples=%d submitted-end-sample=%lld delay-samples=%d delay-bytes=%d\n", "audio runtime",
            samples, bytes, written, buffer.queuedForOutputSamples(), buffer.submittedEndPosition(),
            finalDelay / bytesPerSample, finalDelay);
        CTH_TRACE("drain buffer-read-ms=%.3f output-write-ms=%.3f delay-query-ms=%.3f service-ms=%.3f samples=%d bytes=%d written=%d delay-samples=%d queued-samples=%d\n", "audio timing",
            (bufferReadEnd - bufferReadStart) * 1000.0,
            (outputWriteEnd - outputWriteStart) * 1000.0,
            (delayEnd - delayStart) * 1000.0,
            (getTime() - serviceStart) * 1000.0,
            samples, bytes, written, finalDelay / bytesPerSample, buffer.queuedForOutputSamples());
        return writes;
    }

    double delayStart = getTime();
    int delaySamples = outputDelaySamples();
    double delayEnd = getTime();
    int queuedBefore = buffer.queuedForOutputSamples();
    int samplesWanted = inputFinished ? buffer.queuedForOutputSamples() : configuredTargetDelaySamples - delaySamples;
    if (samplesWanted <= 0) {
        CTH_TRACE("service idle realtime=1 delay-query-ms=%.3f delay-samples=%d target-delay-samples=%d queued-samples=%d scratch-samples=%d input-finished=%d reason=delay-target-met\n",
            "audio timing", (delayEnd - delayStart) * 1000.0, delaySamples,
            configuredTargetDelaySamples, queuedBefore, scratchSamples, inputFinished);
        return 0;
    }
    if (samplesWanted > queuedBefore)
        samplesWanted = queuedBefore;
    if (samplesWanted > scratchSamples)
        samplesWanted = scratchSamples;

    CTH_TRACE("service plan realtime=1 delay-query-ms=%.3f delay-samples=%d target-delay-samples=%d queued-samples=%d scratch-samples=%d requested-samples=%d input-finished=%d\n",
        "audio timing", (delayEnd - delayStart) * 1000.0, delaySamples,
        configuredTargetDelaySamples, queuedBefore, scratchSamples, samplesWanted, inputFinished);

    double bufferReadStart = getTime();
    int samples = buffer.readForOutput(scratch, samplesWanted);
    double bufferReadEnd = getTime();
    if (samples <= 0)
        return 0;

    int bytes = pcmBytesForSamples(samples, bytesPerSample);
    double outputWriteStart = getTime();
    int written = write(scratch, bytes);
    double outputWriteEnd = getTime();
    if (written > 0)
        writes++;

    double finalDelayStart = getTime();
    int finalDelay = outputDelayBytes();
    double finalDelayEnd = getTime();
    CTH_TRACE("output submitted samples=%d bytes=%d written=%d queued-samples=%d submitted-end-sample=%lld delay-samples=%d delay-bytes=%d target-delay-samples=%d requested-samples=%d\n", "audio runtime",
        samples, bytes, written, buffer.queuedForOutputSamples(), buffer.submittedEndPosition(),
        finalDelay / bytesPerSample, finalDelay, configuredTargetDelaySamples, samplesWanted);
    CTH_TRACE("drain buffer-read-ms=%.3f output-write-ms=%.3f delay-query-ms=%.3f final-delay-query-ms=%.3f service-ms=%.3f samples=%d bytes=%d written=%d delay-samples=%d queued-samples=%d\n", "audio timing",
        (bufferReadEnd - bufferReadStart) * 1000.0,
        (outputWriteEnd - outputWriteStart) * 1000.0,
        (delayEnd - delayStart) * 1000.0,
        (finalDelayEnd - finalDelayStart) * 1000.0,
        (getTime() - serviceStart) * 1000.0,
        samples, bytes, written, finalDelay / bytesPerSample, buffer.queuedForOutputSamples());

    return writes;
}

AudioFrameBuilder::AudioFrameBuilder()
    : rawData(NULL)
    , rawCapacity(0) { }

AudioFrameBuilder::~AudioFrameBuilder() {
    delete[] rawData;
    rawData = NULL;
    rawCapacity = 0;
}

void AudioFrameBuilder::setRawCapacity(int rawBytes) {
    if (rawCapacity >= rawBytes)
        return;

    delete[] rawData;
    rawCapacity = rawBytes;
    rawData = new char[rawCapacity];
    CTH_TRACE("resized raw buffer to %d bytes\n", "audio frame builder", rawCapacity);
}

void AudioFrameBuilder::build(AudioFrame& frame, const AudioBuffer& buffer, long long centerSample) {
    int bytesPerSample = buffer.bytesPerSample();
    int rawBytes;
    int halfFrameSamples;
    long long startSample;
    int sampleOffset;
    int samplesRead;

    frame.clear();
    frame.centerSample = centerSample;

    if (bytesPerSample <= 0)
        return;

    rawBytes = pcmBytesForSamples(1024, bytesPerSample);
    halfFrameSamples = 1024 / 2;
    setRawCapacity(rawBytes);
    memset(rawData, 0, rawBytes);

    startSample = centerSample - halfFrameSamples;
    sampleOffset = 0;
    if (startSample < 0) {
        sampleOffset = int(-startSample);
        startSample = 0;
    }

    samplesRead = buffer.readProtectedPcmAt(startSample,
        rawData + pcmBytesForSamples(sampleOffset, bytesPerSample),
        1024 - sampleOffset);
    if (samplesRead <= 0) {
        CTH_TRACE("no samples center-sample=%lld start-sample=%lld\n", "audio frame builder",
            centerSample, startSample);
        return;
    }

    convert(frame.data + sampleOffset,
        rawData + pcmBytesForSamples(sampleOffset, bytesPerSample), samplesRead);
    memcpy(frame.processed, frame.data, sizeof(frame.processed));
    frame.samples = sampleOffset + samplesRead;

    CTH_TRACE("built frame center-sample=%lld start-sample=%lld samples=%d raw-bytes=%d\n", "audio frame builder",
        frame.centerSample, startSample, frame.samples,
        pcmBytesForSamples(frame.samples, bytesPerSample));
}

void AudioFrameBuilder::convert(char2* dst, void* src, int n) {
    unsigned char* data_u8 = (unsigned char*)src;
    char* data_s8 = (char*)src;
    unsigned short* data_u16 = (unsigned short*)src;
    short* data_s16 = (short*)src;

    int cInc = (soundChannels == 1) ? 0 : 1;

    switch (soundFormat) {
    case SF_u8:
        for (int i = 0; i < n; i++) {
            dst[i][1] = int(*data_u8) - 128;
            data_u8 += cInc;
            dst[i][0] = int(*data_u8) - 128;
            data_u8++;
        }
        break;

    case SF_s8:
        for (int i = 0; i < n; i++) {
            dst[i][1] = int(*data_s8);
            data_s8 += cInc;
            dst[i][0] = int(*data_s8);
            data_s8++;
        }
        break;

#if (__BYTE_ORDER == __BIG_ENDIAN)
    case SF_s16_be:
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
    case SF_s16_le:
#endif
        for (int i = 0; i < n; i++) {
            dst[i][1] = int(*data_u16) >> 8;
            data_u16 += cInc;
            dst[i][0] = int(*data_u16) >> 8;
            data_u16++;
        }
        break;

#if (__BYTE_ORDER == __BIG_ENDIAN)
    case SF_s16_le:
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
    case SF_s16_be:
#endif
        for (int i = 0; i < n; i++) {
            dst[i][1] = int(*data_u16) & 255;
            data_u16 += cInc;
            dst[i][0] = int(*data_u16) & 255;
            data_u16++;
        }
        break;

#if (__BYTE_ORDER == __BIG_ENDIAN)
    case SF_u16_be:
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
    case SF_u16_le:
#endif
        for (int i = 0; i < n; i++) {
            dst[i][1] = (int(*data_s16) >> 8) - 128;
            data_s16 += cInc;
            dst[i][0] = (int(*data_s16) >> 8) - 128;
            data_s16++;
        }
        break;

#if (__BYTE_ORDER == __BIG_ENDIAN)
    case SF_u16_le:
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
    case SF_u16_be:
#endif
        for (int i = 0; i < n; i++) {
            dst[i][1] = (int(*data_s16) & 255) - 128;
            data_s16 += cInc;
            dst[i][0] = (int(*data_s16) & 255) - 128;
            data_s16++;
        }
        break;

    default:
        CTH_ERROR("internal error: wrong sound format.\n");
    }
}

PcmSource::PcmSource()
    : error(0) { }

PcmSource::~PcmSource() { }

int PcmSource::rawBufferSize(int frameRawSize, int /*samplesRequested*/) const {
    return frameRawSize;
}

static unsigned int readLe16(const unsigned char* p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

static unsigned int readLe32(const unsigned char* p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8) | ((unsigned int)p[2] << 16)
        | ((unsigned int)p[3] << 24);
}

AudioInput::AudioInput(PcmSource* source_, int takeOwnership)
    : error(0)
    , source(source_)
    , sourceOwned(takeOwnership)
    , finished(0) {
    if ((source == NULL) || source->hasError()) {
        CTH_TRACE("source construction failed\n", "audio input");
        error = 1;
        return;
    }

    applyFormat();
}

AudioInput::~AudioInput() {
    if (sourceOwned)
        delete source;
    source = NULL;
}

void AudioInput::applyFormat() {
    const PcmFormat& format = source->format();

    CTH_TRACE("applying format rate=%d channels=%d format=%d\n", "audio input",
        format.sampleRate, format.channels, format.sampleFormat);
    soundSampleRate.setValue(format.sampleRate);
    soundChannels.setValue(format.channels);
    soundFormat.setValue(format.sampleFormat);
}

int AudioInput::read(char* dst, int rawSize, int samplesRequested) {
    if (finished)
        return 0;

    int samplesRead = source ? source->read(dst, rawSize, samplesRequested) : 0;

    if ((samplesRead == 0) && source && source->canFinish()) {
        if (soundPlayLoop) {
            CTH_TRACE("source reached end; rewinding\n", "audio input");
            source->rewind();
            applyFormat();
            samplesRead = source->read(dst, rawSize, samplesRequested);
        } else {
            CTH_TRACE("source reached end\n", "audio input");
            finished = 1;
        }
    }

    if (samplesRead < 0)
        samplesRead = 0;

    return samplesRead;
}

int AudioInput::rawBufferSize(int frameRawSize, int samplesRequested) const {
    return source ? source->rawBufferSize(frameRawSize, samplesRequested) : frameRawSize;
}

int AudioInput::isFinished() const { return finished; }

void AudioInput::update() {
    if (source) {
        source->update();
        applyFormat();
    }
    finished = 0;
}

int AudioInput::initInputControls() {
    return source ? source->initInputControls() : 0;
}

WavPcmSource::WavPcmSource(const char* name_)
    : PcmSource()
    , file(NULL)
    , dataStart(0)
    , dataLength(0)
    , dataRead(0) {
    strncpy(name, name_, PATH_MAX - 1);
    name[PATH_MAX - 1] = '\0';
    if (open())
        error = 1;
}

WavPcmSource::~WavPcmSource() {
    if (file)
        fclose0(file);
}

int WavPcmSource::open() {
    if (file)
        fclose0(file);

    CTH_INFO("Playing file '%s'.\n", name);
    CTH_TRACE("opening `%s'\n", "wav pcm source", name);

    file = fopen(name, "rb");
    if (file == NULL) {
        CTH_ERRNO(errno, "Can not open sound file `%s' for reading.", name);
        return 1;
    }

    return parseHeader();
}

int WavPcmSource::readChunkHeader(char id[4], unsigned int& size) {
    unsigned char sizeBytes[4];

    if (fread(id, 1, 4, file) != 4)
        return 1;
    if (fread(sizeBytes, 1, 4, file) != 4)
        return 1;

    size = readLe32(sizeBytes);
    return 0;
}

int WavPcmSource::parseHeader() {
    char id[4];
    unsigned int size;
    unsigned char formatBytes[16];
    int foundFormat = 0;
    int foundData = 0;

    dataStart = 0;
    dataLength = 0;
    dataRead = 0;

    if (readChunkHeader(id, size)) {
        CTH_ERROR("Can not read WAV RIFF header.\n");
        return 1;
    }
    if ((memcmp(id, "RIFF", 4) != 0) && (memcmp(id, "RIFX", 4) != 0)) {
        CTH_WARN("  Error in .wav header\n");
        return 1;
    }
    if (memcmp(id, "RIFX", 4) == 0) {
        CTH_WARN("  Big-endian RIFX WAV files are not supported yet.\n");
        return 1;
    }
    if (fread(id, 1, 4, file) != 4) {
        CTH_ERROR("Can not read WAV type.\n");
        return 1;
    }
    if (memcmp(id, "WAVE", 4) != 0) {
        CTH_WARN("  Error in .wav header\n");
        return 1;
    }

    while (!foundData && !feof(file)) {
        if (readChunkHeader(id, size))
            break;

        if (memcmp(id, "fmt ", 4) == 0) {
            if (size < 16) {
                CTH_WARN("  Unsupported .wav fmt chunk size %u.\n", size);
                return 1;
            }
            if (fread(formatBytes, 1, 16, file) != 16) {
                CTH_ERROR("Can not read WAV format chunk.\n");
                return 1;
            }

            unsigned int audioFormat = readLe16(formatBytes);
            unsigned int channels = readLe16(formatBytes + 2);
            unsigned int sampleRate = readLe32(formatBytes + 4);
            unsigned int bitsPerSample = readLe16(formatBytes + 14);

            if (audioFormat != 1) {
                CTH_WARN("  Unsupported .wav encoding %u.\n", audioFormat);
                return 1;
            }
            if ((channels < 1) || (channels > 2)) {
                CTH_WARN("  Unsupported .wav channel count %u.\n", channels);
                return 1;
            }

            pcmFormat.channels = channels;
            pcmFormat.sampleRate = sampleRate;
            switch (bitsPerSample) {
            case 8:
                pcmFormat.sampleFormat = SF_u8;
                break;
            case 16:
                pcmFormat.sampleFormat = SF_s16_le;
                break;
            default:
                CTH_WARN("  Unsupported .wav sample size %u.\n", bitsPerSample);
                return 1;
            }

            CTH_TRACE("format rate=%d channels=%d sample-format=%d\n", "wav pcm source",
                pcmFormat.sampleRate, pcmFormat.channels, pcmFormat.sampleFormat);

            if (size > 16)
                fseek(file, size - 16, SEEK_CUR);
            if (size & 1)
                fseek(file, 1, SEEK_CUR);

            foundFormat = 1;
        } else if (memcmp(id, "data", 4) == 0) {
            if (!foundFormat) {
                CTH_WARN("  WAV data chunk arrived before fmt chunk.\n");
                return 1;
            }
            dataStart = ftell(file);
            dataLength = size;
            dataRead = 0;
            foundData = 1;
            CTH_TRACE("data-start=%ld data-length=%ld\n", "wav pcm source", dataStart, dataLength);
        } else {
            fseek(file, size, SEEK_CUR);
            if (size & 1)
                fseek(file, 1, SEEK_CUR);
        }
    }

    if (!foundData) {
        CTH_WARN("  Error in .wav header\n");
        return 1;
    }

    return 0;
}

int WavPcmSource::read(char* dst, int rawSize, int samplesRequested) {
    if ((file == NULL) || error)
        return 0;

    if (dataRead >= dataLength)
        return 0;

    int bytesPerSample = pcmFormat.bytesPerSample();
    int maxSamples = (bytesPerSample > 0) ? rawSize / bytesPerSample : 0;
    int samples = min(samplesRequested, maxSamples);
    if ((bytesPerSample <= 0) || (samples <= 0))
        return 0;

    int remaining = dataLength - dataRead;
    int wanted = min(pcmBytesForSamples(samples, bytesPerSample), remaining);
    wanted -= wanted % bytesPerSample;
    if (wanted <= 0)
        return 0;
    int readBytes = fread(dst, 1, wanted, file);

    if (readBytes < 0)
        readBytes = 0;
    readBytes -= readBytes % bytesPerSample;
    dataRead += readBytes;

    return readBytes / bytesPerSample;
}

int WavPcmSource::canFinish() const { return 1; }

void WavPcmSource::rewind() {
    CTH_TRACE("rewind `%s'\n", "wav pcm source", name);
    if ((file == NULL) || error)
        return;

    fseek(file, dataStart, SEEK_SET);
    dataRead = 0;
}

Minimp3PcmSource::Minimp3PcmSource(const char* name_)
    : PcmSource()
    , decoder(NULL) {
    strncpy(name, name_, PATH_MAX - 1);
    name[PATH_MAX - 1] = '\0';
    if (open())
        error = 1;
}

Minimp3PcmSource::~Minimp3PcmSource() {
#if WITH_MINIMP3 == 1
    if (decoder) {
        mp3dec_ex_close((mp3dec_ex_t*)decoder);
        delete (mp3dec_ex_t*)decoder;
    }
#endif
    decoder = NULL;
}

int Minimp3PcmSource::open() {
#if WITH_MINIMP3 == 1
    CTH_INFO("Playing file '%s'.\n", name);
    CTH_TRACE("opening `%s'\n", "minimp3 pcm source", name);

    mp3dec_ex_t* dec = new mp3dec_ex_t;
    memset(dec, 0, sizeof(*dec));

    int result = mp3dec_ex_open(dec, name, MP3D_SEEK_TO_SAMPLE | MP3D_DO_NOT_SCAN);
    if (result != 0) {
        CTH_WARN("  Can not open MP3 file `%s' using minimp3: %d.\n", name, result);
        delete dec;
        return 1;
    }

    decoder = dec;
    return applyFormat();
#else
    CTH_WARN("  Embedded minimp3 decoder support is not compiled in.\n");
    CTH_TRACE("open failed because WITH_MINIMP3=0 file=`%s'\n", "minimp3 pcm source", name);
    return 1;
#endif
}

int Minimp3PcmSource::applyFormat() {
#if WITH_MINIMP3 == 1
    mp3dec_ex_t* dec = (mp3dec_ex_t*)decoder;
    if (dec == NULL)
        return 1;

    pcmFormat.sampleRate = dec->info.hz;
    pcmFormat.channels = dec->info.channels;
#if (__BYTE_ORDER == __BIG_ENDIAN)
    pcmFormat.sampleFormat = SF_s16_be;
#else
    pcmFormat.sampleFormat = SF_s16_le;
#endif

    if ((pcmFormat.channels < 1) || (pcmFormat.channels > 2) || (pcmFormat.sampleRate <= 0)) {
        CTH_WARN("  Unsupported MP3 format rate=%d channels=%d.\n",
            pcmFormat.sampleRate, pcmFormat.channels);
        return 1;
    }

    CTH_TRACE("format rate=%d channels=%d sample-format=%d\n", "minimp3 pcm source",
        pcmFormat.sampleRate, pcmFormat.channels, pcmFormat.sampleFormat);
    return 0;
#else
    return 1;
#endif
}

int Minimp3PcmSource::read(char* dst, int rawSize, int samplesRequested) {
#if WITH_MINIMP3 == 1
    if ((decoder == NULL) || error)
        return 0;

    int bytesPerSample = pcmFormat.bytesPerSample();
    int maxSamples = (bytesPerSample > 0) ? rawSize / bytesPerSample : 0;
    int samples = min(samplesRequested, maxSamples);
    if (samples <= 0)
        return 0;

    int channels = pcmFormat.channels;
    if (channels <= 0)
        return 0;

    size_t sampleValuesWanted = (size_t)samples * (size_t)channels;
    size_t sampleValuesRead = mp3dec_ex_read((mp3dec_ex_t*)decoder, (mp3d_sample_t*)dst,
        sampleValuesWanted);

    return (int)(sampleValuesRead / channels);
#else
    return 0;
#endif
}

int Minimp3PcmSource::canFinish() const { return 1; }

void Minimp3PcmSource::rewind() {
    CTH_TRACE("rewind `%s'\n", "minimp3 pcm source", name);
#if WITH_MINIMP3 == 1
    if ((decoder == NULL) || error)
        return;

    mp3dec_ex_seek((mp3dec_ex_t*)decoder, 0);
#endif
}

RandomNoisePcmSource::RandomNoisePcmSource()
    : PcmSource()
    , v1(0)
    , v2(0)
    , maxdv(2) {
    pcmFormat.sampleRate = int(soundSampleRate);
    pcmFormat.channels = 2;
    pcmFormat.sampleFormat = SF_u8;
    CTH_TRACE("created rate=%d channels=%d format=%d\n", "random noise pcm source",
        pcmFormat.sampleRate, pcmFormat.channels, pcmFormat.sampleFormat);
}

int RandomNoisePcmSource::read(char* dst, int rawSize, int samplesRequested) {
    int bytesPerSample = pcmFormat.bytesPerSample();
    int maxSamples = (bytesPerSample > 0) ? rawSize / bytesPerSample : 0;
    int frames = min(min(samplesRequested, maxSamples), 256);

    if (frames <= 0)
        return 0;

    unsigned char* soundData = (unsigned char*)dst;

    soundData[0] = 144;
    soundData[1] = 112;
    for (int x = 1; x < frames; x++) {
        unsigned char* current = soundData + x * 2;
        unsigned char* previous = current - 2;

        if (rand() % 256 > previous[0])
            v1 += rand() % maxdv;
        else
            v1 -= rand() % maxdv;

        if (rand() % 256 > previous[1])
            v2 += rand() % maxdv;
        else
            v2 -= rand() % maxdv;

        current[0] = previous[0] + v1;
        current[1] = previous[1] + v2;
    }

    return frames;
}

void RandomNoisePcmSource::rewind() {
    v1 = 0;
    v2 = 0;
}

AudioInputProcessor::AudioInputProcessor(AudioInput* input_, int takeOwnership)
    : input(input_)
    , inputOwned(takeOwnership)
    , tmpData(NULL)
    , tmpSize(0)
    , rawSize(0)
    , bytesPerSample(0) {
    size = 1 << ilog2(max(BUFF_WIDTH, BUFF_HEIGHT));
    data = new char2[1024];
    memset(data, 0, 1024 * sizeof(char2));
    memset(dataProc, 0, 1024 * sizeof(char2));
    setTmpData();
}

AudioInputProcessor::~AudioInputProcessor() {
    delete[] tmpData;
    tmpData = NULL;

    delete[] data;
    data = NULL;

    if (inputOwned)
        delete input;
    input = NULL;
}

void AudioInputProcessor::setTmpData() {
    bytesPerSample = (soundFormat < 2) ? soundChannels : 2 * soundChannels;
    rawSize = pcmBytesForSamples(size, bytesPerSample);
    int requestedTmpSize = input ? input->rawBufferSize(rawSize, size) : rawSize;
    if (requestedTmpSize < rawSize)
        requestedTmpSize = rawSize;

    if (tmpSize < requestedTmpSize) {
        CTH_TRACE("resizing raw buffer from %d to %d bytes (frame=%d)\n", "audio processor",
            tmpSize, requestedTmpSize, rawSize);
        delete[] tmpData;
        tmpSize = requestedTmpSize;
        tmpData = new char[tmpSize];
    }
}

void AudioInputProcessor::operator()() {
    if (input == NULL)
        return;

    bytesPerSample = (soundFormat < 2) ? soundChannels : 2 * soundChannels;
    rawSize = pcmBytesForSamples(size, bytesPerSample);
    setTmpData();

    int r = input->read(tmpData, rawSize, size);
    if (r < 0)
        r = 0;

    if (r >= 1024)
        convert(data, tmpData + pcmBytesForSamples(r - 1024, bytesPerSample), 1024);
    else {
        memcpy(data, data + r, sizeof(char2) * (1024 - r));
        convert(data + 1024 - r, tmpData, r);
    }
}

void AudioInputProcessor::change() {
    if (input)
        input->update();
    tmpSize = 0;
    setTmpData();
}

void AudioInputProcessor::convert(char2* dst, void* src, int n) {
    unsigned char* data_u8 = (unsigned char*)src;
    char* data_s8 = (char*)src;
    unsigned short* data_u16 = (unsigned short*)src;
    short* data_s16 = (short*)src;

    int cInc = (soundChannels == 1) ? 0 : 1;

    switch (soundFormat) {
    case SF_u8:
        for (int i = 0; i < n; i++) {
            dst[i][1] = int(*data_u8) - 128;
            data_u8 += cInc;
            dst[i][0] = int(*data_u8) - 128;
            data_u8++;
        }
        break;

    case SF_s8:
        for (int i = 0; i < n; i++) {
            dst[i][1] = int(*data_s8);
            data_s8 += cInc;
            dst[i][0] = int(*data_s8);
            data_s8++;
        }
        break;

#if (__BYTE_ORDER == __BIG_ENDIAN)
    case SF_s16_be:
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
    case SF_s16_le:
#endif
        for (int i = 0; i < n; i++) {
            dst[i][1] = int(*data_u16) >> 8;
            data_u16 += cInc;
            dst[i][0] = int(*data_u16) >> 8;
            data_u16++;
        }
        break;

#if (__BYTE_ORDER == __BIG_ENDIAN)
    case SF_s16_le:
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
    case SF_s16_be:
#endif
        for (int i = 0; i < n; i++) {
            dst[i][1] = int(*data_u16) & 255;
            data_u16 += cInc;
            dst[i][0] = int(*data_u16) & 255;
            data_u16++;
        }
        break;

#if (__BYTE_ORDER == __BIG_ENDIAN)
    case SF_u16_be:
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
    case SF_u16_le:
#endif
        for (int i = 0; i < n; i++) {
            dst[i][1] = (int(*data_s16) >> 8) - 128;
            data_s16 += cInc;
            dst[i][0] = (int(*data_s16) >> 8) - 128;
            data_s16++;
        }
        break;

#if (__BYTE_ORDER == __BIG_ENDIAN)
    case SF_u16_le:
#elif (__BYTE_ORDER == __LITTLE_ENDIAN)
    case SF_u16_be:
#endif
        for (int i = 0; i < n; i++) {
            dst[i][1] = (int(*data_s16) & 255) - 128;
            data_s16 += cInc;
            dst[i][0] = (int(*data_s16) & 255) - 128;
            data_s16++;
        }
        break;

    default:
        CTH_ERROR("internal error: wrong sound format.\n");
    }
}

#if WITH_DSP == 1

DspPcmSource::DspPcmSource()
    : PcmSource()
    , handle(-1)
    , dmaBuffer(NULL)
    , dmaSize(0)
    , sampleWindow(1 << ilog2(max(BUFF_WIDTH, BUFF_HEIGHT))) {
    init();
}

void DspPcmSource::setFragment() {
    int soundDSPFragment = (int(soundDSPFragments) << 16) | int(soundDSPFragmentSize);
    if (ioctl(handle, SNDCTL_DSP_SETFRAGMENT, &soundDSPFragment) < 0)
        CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SETFRAGMENT failed.");

    soundDSPFragments.setValue(soundDSPFragment >> 16);
    soundDSPFragmentSize.setValue(soundDSPFragment & 0x7fff);

    if ((1 << soundDSPFragmentSize) * 2 * int(soundChannels) < sampleWindow)
        CTH_WARN("  sound fragment size is not set big enough.\n");
}

void DspPcmSource::setChannels() {
    int channels = int(soundChannels) - 1;
    if (ioctl(handle, SNDCTL_DSP_STEREO, &channels) < 0)
        CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_STEREO failed");
    soundChannels.setValue(channels + 1);
}

void DspPcmSource::setSampleRate() {
    if (ioctl(handle, SNDCTL_DSP_SPEED, &(soundSampleRate.value)) < 0)
        CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SPEED failed");
}

void DspPcmSource::setFormat() {
    int sound_format;

    switch (soundFormat) {
    case SF_u8:
        sound_format = AFMT_U8;
        break;
    case SF_s8:
        sound_format = AFMT_S8;
        break;
    case SF_u16_le:
        sound_format = AFMT_U16_LE;
        break;
    case SF_s16_le:
        sound_format = AFMT_S16_LE;
        break;
    case SF_u16_be:
        sound_format = AFMT_U16_BE;
        break;
    case SF_s16_be:
        sound_format = AFMT_S16_BE;
        break;
    default:
        CTH_ERROR("Internal error: unknown sound format.\n");
        sound_format = AFMT_U8;
    }

    int requested_sound_format = sound_format;

    if (ioctl(handle, SNDCTL_DSP_SETFMT, &sound_format) < 0) {
        CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SETFMT failed. Trying 8bit unsigned");

        sound_format = AFMT_U8;
        if (ioctl(handle, SNDCTL_DSP_SETFMT, &sound_format) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SETFMT failed.");
    }

    CTH_TRACE("set sound format requested=%d returned=%d\n", "dsp pcm source",
        requested_sound_format, sound_format);

    switch (sound_format) {
    case AFMT_U8:
        soundFormat.setValue(SF_u8);
        break;
    case AFMT_S8:
        soundFormat.setValue(SF_s8);
        break;
    case AFMT_U16_LE:
        soundFormat.setValue(SF_u16_le);
        break;
    case AFMT_S16_LE:
        soundFormat.setValue(SF_s16_le);
        break;
    case AFMT_U16_BE:
        soundFormat.setValue(SF_u16_be);
        break;
    case AFMT_S16_BE:
        soundFormat.setValue(SF_s16_be);
        break;
    default:
        CTH_ERROR("Unknown sound format returned by SNDCTL_DSP_SETFMT %d.\n", sound_format);
        error = 1;
    }
}

void DspPcmSource::init() {
    CTH_DEBUG("  setting %s for reading...\n", SoundDeviceDSP::dev_dsp);
    CTH_TRACE("init method=%d sample-window=%d\n", "dsp pcm source", int(soundDSPMethod), sampleWindow);

    if (handle >= 0)
        close(handle);
    handle = -1;

    if ((handle = open(SoundDeviceDSP::dev_dsp, O_RDONLY)) < 0) {
        CTH_ERRNO(errno, "Can't open `%s' for reading.", SoundDeviceDSP::dev_dsp);
        error = 1;
        return;
    }

    switch (int(soundDSPMethod)) {
    case 0:
        CTH_INFO("   Using sound method 0 - optimal fragment size\n");
        soundDSPFragmentSize.setValue(ilog2(sampleWindow) - 1);
        setFragment();
        setChannels();
        setSampleRate();
        setFormat();
        break;

    case 1:
        CTH_INFO("   Using sound method 1 - small fragment size\n");
        soundDSPFragments.setValue(2);
        soundDSPFragmentSize.setValue(4);
        setFragment();
        setChannels();
        setSampleRate();
        setFormat();
        break;

    case 2: {
        CTH_INFO("   Using sound method 2 - old version\n");
        int sound_div = 4;
        if (ioctl(handle, SNDCTL_DSP_SUBDIVIDE, &sound_div) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SUBDIVIDE failed.");

        int dummy = 0;
        if (ioctl(handle, SNDCTL_DSP_STEREO, &dummy) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_STEREO failed.");

        dummy = 0;
        if (ioctl(handle, SNDCTL_DSP_SPEED, &dummy) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_SPEED failed.");

        int sound_blkSize;
        if (ioctl(handle, SNDCTL_DSP_GETBLKSIZE, &sound_blkSize) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_GETBLKSIZE failed.");

        setChannels();
        setSampleRate();
        setFormat();

        if (ioctl(handle, SNDCTL_DSP_GETBLKSIZE, &sound_blkSize) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_GETBLKSIZE");
        break;
    }

    case 3:
        CTH_INFO("   Using sound method 3 - primitiv version\n");
        setFormat();
        setChannels();
        setSampleRate();
        break;

    case 4: {
        CTH_INFO("   Using sound method 4 - directly using DMA buffer\n");
        setFormat();
        setChannels();
        setSampleRate();

        soundDSPFragments.setValue(2);
        soundDSPFragmentSize.setValue(10);
        setFragment();

        struct audio_buf_info info;
        if (ioctl(handle, SNDCTL_DSP_GETISPACE, &info) == -1) {
            CTH_ERROR("ioctl: SNDCTL_DSP_GETISPACE failed.");
            error = 1;
            break;
        }

        dmaSize = info.fragstotal * info.fragsize;
        if (dmaSize < 2048)
            CTH_ERROR("Fragment size changed. New value (%d) is too small.\n"
                    "Please use a different sound method.\n",
                dmaSize);

        dmaBuffer = (char*)mmap(NULL, dmaSize, PROT_READ, MAP_FILE | MAP_SHARED, handle, 0);
        if (dmaBuffer == (char*)-1) {
            CTH_ERROR("mmap failed.\n");
            dmaBuffer = NULL;
            dmaSize = 0;
            error = 1;
            break;
        }

        int tmp = 0;
        ioctl(handle, SNDCTL_DSP_SETTRIGGER, &tmp);

        tmp = PCM_ENABLE_INPUT;
        ioctl(handle, SNDCTL_DSP_SETTRIGGER, &tmp);
        break;
    }

    default:
        CTH_ERROR("Unknown sound method %d.", int(soundDSPMethod));
        CTH_ERROR("   available sound methods:\n"
                "   0: sophisticated 1 (optimal fragment size)\n"
                "   1: sophisticated 2 (small fragments)\n"
                "   2: simple (small DMA buffer)\n"
                "   3: primitiv\n"
                "   4: directly use DMA buffer\n");
        error = 1;
        return;
    }

    pcmFormat.sampleRate = int(soundSampleRate);
    pcmFormat.channels = int(soundChannels);
    pcmFormat.sampleFormat = int(soundFormat);
}

int DspPcmSource::read(char* dst, int rawSize, int samplesRequested) {
    int r = 0;
    int bytesPerSample = (soundFormat < 2) ? soundChannels : 2 * soundChannels;

    switch (int(soundDSPMethod)) {
    case 0: {
        audio_buf_info bi;
        const int nr_read = int(soundChannels) * samplesRequested / (1 << soundDSPFragmentSize);

        if (ioctl(handle, SNDCTL_DSP_GETISPACE, &bi) < 0)
            CTH_ERRNO(errno, "ioctl: SNDCTL_DSP_GETISPACE failed.");

        if (bi.fragments > nr_read) {
            for (int i = 0; i < bi.fragments - nr_read; i++)
                ::read(handle, dst, (1 << soundDSPFragmentSize));
            r = (1 << soundDSPFragmentSize) * nr_read;
        } else
            r = (1 << soundDSPFragmentSize) * bi.fragments;

        if (r == 0)
            r = (1 << soundDSPFragmentSize);

        if (::read(handle, dst, r) < 0)
            CTH_ERRNO(errno, "reading sound failed.");
        break;
    }

    case 1: {
        unsigned char* sbuff;
        int nr_read;
        for (nr_read = 0, sbuff = (unsigned char*)dst; nr_read < rawSize; nr_read += 32) {
            if (::read(handle, sbuff, 16) < 0)
                CTH_ERRNO(errno, "sound_read < 0");
            sbuff += 16;
            if (::read(handle, sbuff, 16) < 0)
                CTH_ERRNO(errno, "sound_read < 0");
            sbuff += 16;
        }
        r = nr_read;
        break;
    }

    case 2:
    case 3:
        r = ::read(handle, dst, rawSize);
        if (r < 0)
            CTH_ERRNO(errno, "get_sound: read < 0.");
        break;

    case 4:
        if (dmaBuffer != NULL) {
            memcpy(dst, dmaBuffer, 2048);
            r = 2048;
        }
        break;
    }

    if (int(soundDSPSync))
        ioctl(handle, SNDCTL_DSP_RESET);

    return r / bytesPerSample;
}

int DspPcmSource::rawBufferSize(int frameRawSize, int samplesRequested) const {
    switch (int(soundDSPMethod)) {
    case 0:
        return max(frameRawSize, (1 << soundDSPFragmentSize) * 4);
    case 1:
        return max(frameRawSize, samplesRequested * 4 + 32);
    case 4:
        return max(frameRawSize, 2048);
    default:
        return frameRawSize;
    }
}

void DspPcmSource::update() {
    if (dmaBuffer != NULL) {
        munmap(dmaBuffer, dmaSize);
        dmaBuffer = NULL;
        dmaSize = 0;
    }
    init();
}

int DspPcmSource::initInputControls() {
    CTH_INFO("Initializing OSS mixer device...\n");
    return init_mixer();
}

DspPcmSource::~DspPcmSource() {
    if (dmaBuffer != NULL)
        munmap(dmaBuffer, dmaSize);
    dmaBuffer = NULL;
    dmaSize = 0;

    if (handle >= 0)
        close(handle);
}

#else

DspPcmSource::DspPcmSource()
    : PcmSource()
    , handle(-1)
    , dmaBuffer(NULL)
    , dmaSize(0)
    , sampleWindow(0) {
    CTH_ERROR("DSP device was disabled at compile time.\n");
    error = 1;
}

DspPcmSource::~DspPcmSource() { }
void DspPcmSource::setFragment() { }
void DspPcmSource::setChannels() { }
void DspPcmSource::setSampleRate() { }
void DspPcmSource::setFormat() { }
void DspPcmSource::init() { }
int DspPcmSource::read(char*, int, int) { return 0; }
int DspPcmSource::rawBufferSize(int frameRawSize, int) const { return frameRawSize; }
void DspPcmSource::update() { }
int DspPcmSource::initInputControls() { return 0; }

#endif
