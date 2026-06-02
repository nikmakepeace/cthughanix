// Audio output base class, null output, and output option globals.

#include "cthugha.h"
#include "Audio.h"
#include "AudioInternal.h"
#include "defaults.h"

char pulse_server[PATH_MAX] = DEFAULT_PULSE_SERVER_TEXT;
int pulse_latency_msec = DEFAULT_PULSE_LATENCY_MS;
char audio_output_dump[PATH_MAX] = DEFAULT_AUDIO_OUTPUT_DUMP_PATH;
char audio_input_file[PATH_MAX] = DEFAULT_AUDIO_INPUT_FILE_PATH;

const char* pulse_server_name() {
    return (pulse_server[0] != '\0') ? pulse_server : NULL;
}

const char* pulse_server_display_name() {
    return pulse_server_name() ? pulse_server_name() : "default";
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

    CTH_DEBUG("audio output: configured timing realtime=%d samples-per-second=%d bytes-per-sample=%d input-chunk-samples=%d target-buffer-ms=%d target-buffer-samples=%d scratch-samples=%d\n",
        isRealtime(), outputSamplesPerSecond, outputBytesPerSample,
        inputChunkSamples, targetLatencyMs, outputTargetDelaySamples, outputScratchSamples);
}

int AudioOutput::queuedTargetSamples() const {
    return isRealtime() ? outputTargetDelaySamples : outputScratchSamples;
}

int AudioOutput::playbackComplete(const AudioBuffer& buffer, int inputFinished) const {
    return inputFinished && (buffer.queuedForOutputSamples() == 0);
}

int AudioNullOutput::defaultTargetLatencyMs() const { return DEFAULT_AUDIO_NULL_TARGET_LATENCY_MS; }
int AudioNullOutput::write(const void*, int size) { return size; }
int AudioNullOutput::isOpen() const { return 1; }
int AudioNullOutput::isRealtime() const { return 0; }
int AudioOutput::service(AudioBuffer& buffer, char* scratch, int scratchSamples,
    int /*inputFinished*/) {
    if ((scratch == NULL) || (scratchSamples <= 0))
        return 0;

    double serviceStart = getTime();
    int bytesPerSample = buffer.bytesPerSample();
    if (bytesPerSample <= 0)
        return 0;

    int queuedBefore = buffer.queuedForOutputSamples();
    if (queuedBefore <= 0)
        return 0;

    int samplesWanted = queuedBefore;
    if (samplesWanted > queuedBefore)
        samplesWanted = queuedBefore;
    if (samplesWanted > scratchSamples)
        samplesWanted = scratchSamples;
    if (samplesWanted <= 0)
        return 0;

    CTH_TRACE("service plan realtime=%d queued-samples=%d scratch-samples=%d requested-samples=%d\n",
        "audio timing", isRealtime(), queuedBefore, scratchSamples, samplesWanted);

    double bufferReadStart = getTime();
    long long startSample = buffer.submittedEndPosition();
    int samples = buffer.peekForOutput(scratch, samplesWanted);
    double bufferReadEnd = getTime();
    if (samples <= 0)
        return 0;

    int bytes = pcmBytesForSamples(samples, bytesPerSample);
    double outputWriteStart = getTime();
    int written = write(scratch, bytes);
    double outputWriteEnd = getTime();
    int committedSamples = 0;
    int committedBytes = 0;
    if (written > 0) {
        committedSamples = buffer.commitOutputSamples(written / bytesPerSample);
        committedBytes = pcmBytesForSamples(committedSamples, bytesPerSample);
    }
    if (committedSamples > 0) {
        audioOutputDumpSubmittedPcm(scratch, committedBytes);
    }

    audioDebugSubmittedPcm(scratch, committedSamples, committedBytes, written, buffer.queuedForOutputSamples(),
        buffer.submittedEndPosition());
    CTH_TRACE("output submitted samples=%d bytes=%d written=%d committed-samples=%d committed-bytes=%d queued-samples=%d submitted-start-sample=%lld submitted-end-sample=%lld requested-samples=%d\n", "audio runtime",
        samples, bytes, written, committedSamples, committedBytes,
        buffer.queuedForOutputSamples(), startSample, buffer.submittedEndPosition(),
        samplesWanted);
    CTH_TRACE("drain buffer-read-ms=%.3f output-write-ms=%.3f service-ms=%.3f samples=%d bytes=%d written=%d committed-samples=%d queued-samples=%d\n", "audio timing",
        (bufferReadEnd - bufferReadStart) * 1000.0,
        (outputWriteEnd - outputWriteStart) * 1000.0,
        (getTime() - serviceStart) * 1000.0,
        samples, bytes, written, committedSamples,
        buffer.queuedForOutputSamples());

    return committedSamples > 0 ? 1 : 0;
}
