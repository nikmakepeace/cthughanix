/** @file
 * Unit coverage for audio passthrough cursor ownership.
 */

#include "AudioPassthrough.h"
#include "ProcessServices.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>

int cth_log_enabled(int) { return 0; }
int cth_log(int, const char*, ...) { return 0; }
int cth_log_context(int, const char*, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }
int cth_log_errno(int, const char*, ...) { return 0; }

const char* audioSampleFormatText(int) { return "signed-8"; }

class FakeSecondsClock : public SecondsClock {
public:
    virtual double nowSeconds() const {
        return 0.0;
    }
};

class FakeLogSink : public LogSink {
protected:
    virtual void write(int, const char*, int, const char*, va_list) { }

public:
    virtual int enabled(int) const {
        return 0;
    }
};

static PcmFormat signed8MonoFormat() {
    PcmFormat format;
    format.sampleRate = 1000;
    format.channels = 1;
    format.sampleFormat = SF_s8;
    return format;
}

class RecordingOutput : public AudioOutput {
    int realtimeValue;

protected:
    virtual int defaultTargetLatencyMs() const { return 0; }

public:
    int bytesWritten;
    int maxBytesPerWrite;

    RecordingOutput(SecondsClock& clock, LogSink& log, int realtime = 0,
        int maxBytes = 0)
        : AudioOutput(0, NULL, clock, log)
        , realtimeValue(realtime)
        , bytesWritten(0)
        , maxBytesPerWrite(maxBytes) { }

    virtual int write(const void*, int size) {
        int accepted = size;
        if ((maxBytesPerWrite > 0) && (accepted > maxBytesPerWrite))
            accepted = maxBytesPerWrite;
        bytesWritten += accepted;
        return accepted;
    }

    virtual int isOpen() const { return 1; }
    virtual int isRealtime() const { return realtimeValue; }
};

static void testPassthroughAdvancesOnlyItsCursor() {
    FakeLogSink log;
    DecodedAudioHistory history(16, signed8MonoFormat(), 8, log);
    char pcm[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    std::atomic<int> inputFinished(0);
    FakeSecondsClock clock;
    AudioPassthrough passthrough(new RecordingOutput(clock, log), history,
        inputFinished, log);

    assert(history.appendDecodedPcm(pcm, 8) == 8);
    assert(passthrough.start(1000, 1, 4, 0) == 0);
    assert(passthrough.serviceOnce() == 1);

    assert(history.decodedEndPosition() == 8);
    assert(passthrough.submittedSamplePosition() == 4);
    assert(passthrough.queuedSamples() == 4);

    inputFinished.store(1);
    assert(passthrough.serviceOnce() == 1);
    assert(passthrough.complete());
}

static void testPassthroughResyncsWhenHistoryMovedOn() {
    FakeLogSink log;
    DecodedAudioHistory history(8, signed8MonoFormat(), 3, log);
    char first[5] = { 0, 1, 2, 3, 4 };
    char second[5] = { 5, 6, 7, 8, 9 };
    std::atomic<int> inputFinished(0);
    FakeSecondsClock clock;
    AudioPassthrough passthrough(new RecordingOutput(clock, log, 0, 1),
        history, inputFinished, log);

    assert(history.appendDecodedPcm(first, 5) == 5);
    assert(history.appendDecodedPcm(second, 5) == 5);
    assert(history.oldestAvailablePosition() == 7);
    assert(passthrough.start(1000, 1, 1, 0) == 0);
    assert(passthrough.serviceOnce() == 1);

    assert(passthrough.submittedSamplePosition() == 8);
    assert(history.decodedEndPosition() == 10);
}

static void testPresentationClockSubtractsTargetDelay() {
    FakeLogSink log;
    DecodedAudioHistory history(16, signed8MonoFormat(), 8, log);
    char pcm[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    std::atomic<int> inputFinished(0);
    FakeSecondsClock clock;
    AudioPassthrough passthrough(new RecordingOutput(clock, log, 1), history,
        inputFinished, log);

    assert(history.appendDecodedPcm(pcm, 8) == 8);
    assert(passthrough.start(1000, 1, 4, 0) == 0);
    assert(passthrough.providesPresentationClock());
    assert(passthrough.serviceOnce() == 1);

    assert(passthrough.submittedSamplePosition() == 4);
    assert(passthrough.presentationSamplePosition() == 0);
}

int main() {
    testPassthroughAdvancesOnlyItsCursor();
    testPassthroughResyncsWhenHistoryMovedOn();
    testPresentationClockSubtractsTargetDelay();
    return 0;
}
