#include "cthugha.h"
#include "AudioRuntime.h"
#include "CthughaDisplay.h"

static AudioInputProcessor* audioProcessor = NULL;
static AudioInput* audioInput = NULL;
static AudioOutput* audioOutput = NULL;
static AudioBuffer* audioBuffer = NULL;
static AudioFrameBuilder* audioFrameBuilder = NULL;
static char* audioRuntimeChunk = NULL;
static char* audioRuntimeOutputChunk = NULL;
static AudioFrame audioRuntimeFrame;
static int audioRuntimeChunkSamples = 0;
static int audioRuntimeOutputChunkSamples = 0;
static int audioRuntimeInputFinished = 0;
static int audioRuntimeComplete = 0;

static long long audioRuntimeDecodedSamplePosition();
static long long audioRuntimeSubmittedSamplePosition();
static long long audioRuntimeAudibleSamplePosition();

#ifndef WITH_MINIMP3
#define WITH_MINIMP3 1
#endif

static const char* audioRuntimeContextName(RuntimeSoundInputContext context) {
    return (context == RSIC_FileChild) ? "file child" : "main process";
}

static int audioRuntimeUsesNativeFilePipeline(AudioSourceStrategy sourceStrategy) {
    if (sourceStrategy == ASS_WavFile)
        return 1;
#if WITH_MINIMP3 == 1
    if (sourceStrategy == ASS_Mp3File)
        return 1;
#endif
    return 0;
}

static int audioRuntimeBytesPerSample() {
    return (soundFormat < 2) ? int(soundChannels) : 2 * int(soundChannels);
}

static int audioRuntimeSamplesPerSecond() {
    return int(soundSampleRate);
}

static int audioRuntimeStrategyChunkSamples() {
    int samples = audioRuntimeSamplesPerSecond() / 20;

    if (samples < 1024)
        samples = 1024;
    if (samples > 8192)
        samples = 8192;

    return samples;
}

static int audioRuntimeStrategyBufferSamples() {
    int samples = audioRuntimeSamplesPerSecond() * 3;

    if (samples < 32768)
        samples = 32768;

    return samples;
}

static int audioRuntimeStrategyHistorySamples() {
    int samples = audioRuntimeSamplesPerSecond();

    if (samples < 16384)
        samples = 16384;

    return samples;
}

static int audioRuntimeFillBuffer(int maxSamples) {
    if ((audioInput == NULL) || (audioOutput == NULL) || (audioBuffer == NULL)
        || (audioRuntimeChunk == NULL))
        return 0;
    if (audioRuntimeInputFinished)
        return 0;

    int bytesPerSample = audioRuntimeBytesPerSample();
    if (bytesPerSample <= 0)
        return 0;

    int freeSamples = audioBuffer->writableSamples();
    int limitedSamples = (maxSamples < freeSamples) ? maxSamples : freeSamples;
    int samplesWanted = (audioRuntimeChunkSamples < limitedSamples) ? audioRuntimeChunkSamples : limitedSamples;
    if (samplesWanted <= 0)
        return 0;

    double readStart = getTime();
    int samplesRead = audioInput->read(audioRuntimeChunk,
        pcmBytesForSamples(samplesWanted, bytesPerSample), samplesWanted);
    double readEnd = getTime();
    if (samplesRead > 0) {
        double bufferStart = getTime();
        int buffered = audioBuffer->appendDecodedPcm(audioRuntimeChunk, samplesRead);
        double bufferEnd = getTime();
        int bytesRead = pcmBytesForSamples(samplesRead, bytesPerSample);
        CTH_TRACE("input produced samples=%d bytes=%d appended-samples=%d queued-samples=%d decoded-end-sample=%lld\n", "audio runtime",
            samplesRead, bytesRead, buffered, audioBuffer->queuedForOutputSamples(),
            audioBuffer->decodedEndPosition());
        CTH_TRACE("fill read-ms=%.3f buffer-write-ms=%.3f samples=%d bytes=%d queued-samples=%d\n", "audio timing",
            (readEnd - readStart) * 1000.0, (bufferEnd - bufferStart) * 1000.0,
            samplesRead, bytesRead, audioBuffer->queuedForOutputSamples());
        return buffered;
    }

    if (audioInput->isFinished()) {
        audioRuntimeInputFinished = 1;
        CTH_TRACE("input finished decoded-end-sample=%lld queued-samples=%d\n", "audio runtime",
            audioBuffer->decodedEndPosition(), audioBuffer->queuedForOutputSamples());
    }

    return 0;
}

static void audioRuntimePumpPipeline() {
    if ((audioInput == NULL) || (audioOutput == NULL) || (audioBuffer == NULL)
        || (audioRuntimeChunk == NULL) || (audioRuntimeOutputChunk == NULL))
        return;

    if (audioRuntimeComplete)
        return;

    double pumpStart = getTime();
    int fillCalls = 0;
    int targetDelay = audioOutput->targetDelaySamples();

    int inputTarget = audioOutput->queuedTargetSamples();
    int loops = 0;
    while (!audioRuntimeInputFinished && (audioBuffer->queuedForOutputSamples() < inputTarget)
        && (loops < 16)) {
        if (audioRuntimeFillBuffer(audioRuntimeChunkSamples) <= 0)
            break;
        fillCalls++;
        loops++;
    }

    int writeCalls = audioOutput->service(*audioBuffer, audioRuntimeOutputChunk,
        audioRuntimeOutputChunkSamples, audioRuntimeInputFinished);

    if (audioOutput->playbackComplete(*audioBuffer, audioRuntimeInputFinished))
        audioRuntimeComplete = 1;

    CTH_TRACE("pump total-ms=%.3f fills=%d writes=%d queued-samples=%d delay-samples=%d target-delay-samples=%d complete=%d\n", "audio timing",
        (getTime() - pumpStart) * 1000.0, fillCalls, writeCalls,
        audioBuffer->queuedForOutputSamples(), audioOutput->outputDelaySamples(), targetDelay, audioRuntimeComplete);

    if (audioRuntimeComplete) {
        CTH_INFO("Stopping...\n");
        CTH_TRACE("playback complete decoded-end-sample=%lld submitted-end-sample=%lld audible-sample=%lld\n", "audio runtime",
            audioRuntimeDecodedSamplePosition(), audioRuntimeSubmittedSamplePosition(),
            audioRuntimeAudibleSamplePosition());
        cthugha_close++;
    }
}

static void audioRuntimeBuildFrame() {
    if ((audioFrameBuilder == NULL) || (audioBuffer == NULL))
        return;

    long long audibleSample = audioRuntimeAudibleSamplePosition();
    double visualLatencySeconds = cthughaDisplay ? cthughaDisplay->visualLatencySeconds() : 0;
    long long visualOffsetSamples = (long long)(audioRuntimeSamplesPerSecond() * visualLatencySeconds);

    long long frameCenterSample = audibleSample + visualOffsetSamples;
    if (frameCenterSample > audioBuffer->decodedEndPosition())
        frameCenterSample = audioBuffer->decodedEndPosition();

    double buildStart = getTime();
    audioFrameBuilder->build(audioRuntimeFrame, *audioBuffer, frameCenterSample);
    CTH_TRACE("frame-build-ms=%.3f audible-sample=%lld visual-offset-samples=%lld visual-latency-ms=%.3f center-sample=%lld queued-samples=%d\n",
        "audio timing",
        (getTime() - buildStart) * 1000.0, audibleSample, visualOffsetSamples,
        visualLatencySeconds * 1000.0, frameCenterSample, audioBuffer->queuedForOutputSamples());
}

void audioRuntimeInit(RuntimeSoundInputContext context, int initializeInputControls) {
    CTH_TRACE("init requested context=%s initialize-input-controls=%d\n", "audio runtime",
        audioRuntimeContextName(context), initializeInputControls);

    if (audioRuntimeIsInitialized()) {
        CTH_TRACE("init skipped because audio is already installed\n", "audio runtime");
        return;
    }

    Settings settings = Settings::fromCurrentOptions();
    Environment environment = Environment::detect();
    RuntimeFactory runtimeFactory(settings, environment);
    AudioSourceStrategy sourceStrategy = runtimeFactory.selectAudioSourceStrategy();
    audioRuntimeInputFinished = 0;
    audioRuntimeComplete = 0;

    if (audioRuntimeUsesNativeFilePipeline(sourceStrategy)) {
        audioInput = runtimeFactory.createAudioInput();
        if ((audioInput == NULL) || audioInput->hasError()) {
            CTH_TRACE("native file input construction failed strategy=%d\n", "audio runtime",
                sourceStrategy);
            delete audioInput;
            audioInput = NULL;
            exit(0);
        }

        audioOutput = runtimeFactory.createAudioOutput();
        if ((audioOutput == NULL) || !audioOutput->isOpen()) {
            CTH_TRACE("native file output construction failed strategy=%d\n", "audio runtime",
                sourceStrategy);
            delete audioInput;
            audioInput = NULL;
            delete audioOutput;
            audioOutput = NULL;
            exit(0);
        }

        int bytesPerSample = audioRuntimeBytesPerSample();
        audioRuntimeChunkSamples = audioRuntimeStrategyChunkSamples();
        audioOutput->configureTiming(audioRuntimeSamplesPerSecond(), bytesPerSample,
            audioRuntimeChunkSamples);
        audioRuntimeOutputChunkSamples = audioOutput->scratchSamples();
        audioBuffer = new AudioBuffer(audioRuntimeStrategyBufferSamples(), bytesPerSample,
            audioRuntimeStrategyHistorySamples());
        audioFrameBuilder = new AudioFrameBuilder();
        audioRuntimeChunk = new char[pcmBytesForSamples(audioRuntimeChunkSamples, bytesPerSample)];
        audioRuntimeOutputChunk = new char[pcmBytesForSamples(audioRuntimeOutputChunkSamples, bytesPerSample)];
        audioRuntimeFrame.clear();
        CTH_TRACE("installed native file pipeline strategy=%d input=%p buffer=%p output=%p frame-builder=%p input-chunk-samples=%d output-chunk-samples=%d bytes-per-sample=%d target-delay-samples=%d\n", "audio runtime",
            sourceStrategy, audioInput, audioBuffer, audioOutput, audioFrameBuilder,
            audioRuntimeChunkSamples, audioRuntimeOutputChunkSamples, bytesPerSample,
            audioOutput->targetDelaySamples());
    } else {
        audioProcessor = runtimeFactory.createAudioProcessor();
        if (audioProcessor != NULL) {
            CTH_TRACE("installed native AudioInputProcessor path\n", "audio runtime");
            if (initializeInputControls && audioProcessor->audioInput()->initInputControls()) {
                CTH_TRACE("native input control initialization failed\n", "audio runtime");
                exit(0);
            }
        } else {
            CTH_TRACE("native AudioInputProcessor unavailable; using legacy SoundDevice path\n", "audio runtime");
            SoundDevice::install(runtimeFactory.createLegacySoundDevice(context), initializeInputControls);
        }
    }

    CTH_TRACE("init completed context=%s\n", "audio runtime", audioRuntimeContextName(context));
}

void audioRuntimeTick() {
    if (audioBuffer != NULL) {
        double tickStart = getTime();
        double pumpStart = tickStart;
        audioRuntimePumpPipeline();
        double pumpEnd = getTime();
        audioRuntimeBuildFrame();
        double buildEnd = getTime();
        CTH_TRACE("tick pipeline-ms=%.3f pump-ms=%.3f build-ms=%.3f queued-samples=%d delay-samples=%d decoded-end-sample=%lld submitted-end-sample=%lld audible-sample=%lld\n",
            "audio timing",
            (buildEnd - tickStart) * 1000.0,
            (pumpEnd - pumpStart) * 1000.0,
            (buildEnd - pumpEnd) * 1000.0,
            audioBuffer->queuedForOutputSamples(),
            audioOutput ? audioOutput->outputDelaySamples() : 0,
            audioRuntimeDecodedSamplePosition(),
            audioRuntimeSubmittedSamplePosition(),
            audioRuntimeAudibleSamplePosition());
        return;
    }

    if (audioProcessor != NULL) {
        (*audioProcessor)();
        return;
    }

    if (soundDevice)
        (*soundDevice)();
}

void audioRuntimeShutdown() {
    CTH_TRACE("shutdown requested pipeline=%d native=%d legacy=%d\n", "audio runtime",
        audioBuffer != NULL, audioProcessor != NULL, soundDevice != NULL);
    delete[] audioRuntimeChunk;
    audioRuntimeChunk = NULL;
    delete[] audioRuntimeOutputChunk;
    audioRuntimeOutputChunk = NULL;

    delete audioBuffer;
    audioBuffer = NULL;

    delete audioFrameBuilder;
    audioFrameBuilder = NULL;
    audioRuntimeFrame.clear();

    delete audioOutput;
    audioOutput = NULL;

    delete audioInput;
    audioInput = NULL;

    delete audioProcessor;
    audioProcessor = NULL;

    audioRuntimeInputFinished = 0;
    audioRuntimeComplete = 0;
    audioRuntimeChunkSamples = 0;
    audioRuntimeOutputChunkSamples = 0;

    delete soundDevice;
    soundDevice = NULL;
    CTH_TRACE("shutdown completed\n", "audio runtime");
}

int audioRuntimeIsInitialized() {
    return (audioBuffer != NULL) || (audioProcessor != NULL) || (soundDevice != NULL);
}

AudioInputProcessor* audioRuntimeProcessor() {
    return audioProcessor;
}

AudioFrame* audioRuntimeCurrentFrame() {
    return audioFrameBuilder ? &audioRuntimeFrame : NULL;
}

int audioRuntimeIsComplete() {
    return audioRuntimeComplete;
}

static long long audioRuntimeDecodedSamplePosition() {
    return audioBuffer ? audioBuffer->decodedEndPosition() : 0;
}

static long long audioRuntimeSubmittedSamplePosition() {
    return audioBuffer ? audioBuffer->submittedEndPosition() : 0;
}

static long long audioRuntimeAudibleSamplePosition() {
    if ((audioBuffer == NULL) || (audioOutput == NULL))
        return 0;

    return audioOutput->audibleSamplePosition(*audioBuffer);
}
