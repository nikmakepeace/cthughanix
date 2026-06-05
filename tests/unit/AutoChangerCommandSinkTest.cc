#include "AutoChanger.h"
#include "AutoChangeSettings.h"
#include "AudioAnalyzer.h"
#include "RuntimeCommandSink.h"
#include "VideoDirector.h"

#include <assert.h>
#include <vector>

int cth_log_enabled(int) {
    return 0;
}

int cth_log(int, const char*, ...) {
    return 0;
}

int cth_log_context(int, const char*, const char*, ...) {
    return 0;
}

int cth_log_error(const char*, ...) {
    return 0;
}

int cth_log_errno(int, const char*, ...) {
    return 0;
}

static int fakeTime = 1000;

int gettime() {
    return fakeTime;
}

AudioMetrics::AudioMetrics()
    : amplitude(0)
    , amplitudeLeft(0)
    , amplitudeRight(0)
    , noisy(0) { }

AcousticContext::AcousticContext()
    : intensityValue(0.0)
    , lastAmplitudeValue(0)
    , attackLevelValue(0)
    , fireValue(0)
    , cumulativeFireLevelValue(0) { }

void AcousticContext::update(const AudioMetrics&) { }
double AcousticContext::intensity() const { return 0.0; }
int AcousticContext::fire() const { return 0; }
int AcousticContext::cumulativeFireLevel() const { return 0; }
void AcousticContext::resetCumulativeFireLevel() { }

VideoDirector& videoDirector() {
    static char storage[sizeof(VideoDirector)];
    return *reinterpret_cast<VideoDirector*>(storage);
}

int VideoDirector::observeQuiet(int) {
    return 0;
}

class RecordingSink : public RuntimeCommandSink {
public:
    std::vector<RuntimeCommandType> commands;

    virtual RuntimeChangeSet apply(const RuntimeCommand& command) {
        commands.push_back(command.type);
        return RuntimeChangeSet();
    }
};

static AutoChangeConfig autoChangeConfigWithLittle(int changeLittle) {
    AutoChangeConfig config;
    config.quietMs = 0;
    config.waitMinMs = 0;
    config.waitRandomMs = 1;
    config.cumulativeFireLevel = 0;
    config.locked = 0;
    config.changeLittle = changeLittle;
    return config;
}

static void testAutoChangerRequestsChangeOneForLittleChanges() {
    OwnedAutoChangeSettings settings(autoChangeConfigWithLittle(1));

    RecordingSink sink;
    AcousticContext acousticContext;
    AutoChanger changer(sink, settings, acousticContext);
    changer.change();

    assert(sink.commands.size() == 1);
    assert(sink.commands[0] == RuntimeCommandChangeOne);
}

static void testAutoChangerRequestsChangeAllForFullChanges() {
    OwnedAutoChangeSettings settings(autoChangeConfigWithLittle(0));

    RecordingSink sink;
    AcousticContext acousticContext;
    AutoChanger changer(sink, settings, acousticContext);
    changer.change();

    assert(sink.commands.size() == 1);
    assert(sink.commands[0] == RuntimeCommandChangeAll);
}

int main() {
    testAutoChangerRequestsChangeOneForLittleChanges();
    testAutoChangerRequestsChangeAllForFullChanges();
    return 0;
}
