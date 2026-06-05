/** @file
 * Unit coverage for settings-backed runtime auto-change controls.
 */

#include "RuntimeAutoChangeControls.h"

#include "AutoChangeControls.h"
#include "AutoChangeSettings.h"
#include "VideoDirector.h"

#include <assert.h>

OptionTime changeMsgTime("message-time", 0);

int cth_log(int, const char*, ...) { return 0; }
int cth_log_context(int, const char*, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }
int cth_log_errno(int, const char*, ...) { return 0; }

class FakeAutoChangeSettings : public AutoChangeSettings {
public:
    int quietMsValue;
    int waitMinMsValue;
    int waitRandomMsValue;
    int waitRandomMinimumMsValue;
    int cumulativeFireLevelValue;
    int lockedValue;
    int changeLittleValue;

    FakeAutoChangeSettings()
        : quietMsValue(0)
        , waitMinMsValue(1000)
        , waitRandomMsValue(2000)
        , waitRandomMinimumMsValue(1)
        , cumulativeFireLevelValue(10)
        , lockedValue(0)
        , changeLittleValue(0) { }

    virtual int quietMs() const { return quietMsValue; }
    virtual void setQuietMs(int value) { quietMsValue = value; }
    virtual int waitMinMs() const { return waitMinMsValue; }
    virtual void setWaitMinMs(int value) { waitMinMsValue = value; }
    virtual int waitRandomMs() const { return waitRandomMsValue; }
    virtual void setWaitRandomMs(int value) { waitRandomMsValue = value; }
    virtual int waitRandomMinimumMs() const { return waitRandomMinimumMsValue; }
    virtual int waitRandomRangeMs() const { return waitRandomMsValue; }
    virtual int cumulativeFireLevel() const {
        return cumulativeFireLevelValue;
    }
    virtual void setCumulativeFireLevel(int value) {
        cumulativeFireLevelValue = value;
    }
    virtual int locked() const { return lockedValue; }
    virtual void setLocked(int value) { lockedValue = value ? 1 : 0; }
    virtual int changeLittle() const { return changeLittleValue; }
    virtual void setChangeLittle(int value) {
        changeLittleValue = value ? 1 : 0;
    }
    virtual AutoChangeConfig config() const {
        AutoChangeConfig config;
        config.quietMs = quietMsValue;
        config.waitMinMs = waitMinMsValue;
        config.waitRandomMs = waitRandomMsValue;
        config.waitRandomMinimumMs = waitRandomMinimumMsValue;
        config.cumulativeFireLevel = cumulativeFireLevelValue;
        config.locked = lockedValue;
        config.changeLittle = changeLittleValue;
        return config;
    }
};

class RecordingOption : public Option {
public:
    int byCalls;
    int toCalls;

    RecordingOption()
        : Option("recording")
        , byCalls(0)
        , toCalls(0) { }

    virtual void change(int) { byCalls++; }
    virtual void change(const char*) { toCalls++; }
    virtual const char* text() const { return "recording"; }
};

static void testToggleLockMutatesSettings() {
    FakeAutoChangeSettings settings;
    AutoChangeControls controls(settings);
    DefaultRuntimeAutoChangeControls runtimeControls(controls);

    runtimeControls.toggleLock();
    assert(settings.lockedValue == 1);
    runtimeControls.toggleLock();
    assert(settings.lockedValue == 0);
}

static void testGenericAutoChangeOptionsMutateSettings() {
    FakeAutoChangeSettings settings;
    AutoChangeControls controls(settings);
    DefaultRuntimeAutoChangeControls runtimeControls(controls);

    RuntimeChangeSet changes;
    int handled = runtimeControls.changeAutoChangeOptionBy(
        controls.waitMinOption(), 500, changes);
    assert(handled == 1);
    assert(changes.autoChangeChanged == 1);
    assert(settings.waitMinMsValue == 1500);

    RuntimeChangeSet textChanges;
    handled = runtimeControls.changeAutoChangeOptionTo(
        controls.quietOption(), "2.50sec", textChanges);
    assert(handled == 1);
    assert(textChanges.autoChangeChanged == 1);
    assert(settings.quietMsValue == 2500);

    RuntimeChangeSet boolChanges;
    handled = runtimeControls.changeAutoChangeOptionBy(
        controls.changeLittleOption(), 1, boolChanges);
    assert(handled == 1);
    assert(settings.changeLittleValue == 1);
}

static void testUnrelatedOptionsAreNotChanged() {
    FakeAutoChangeSettings settings;
    AutoChangeControls controls(settings);
    DefaultRuntimeAutoChangeControls runtimeControls(controls);
    RecordingOption unrelated;

    RuntimeChangeSet changes;
    int handled = runtimeControls.changeAutoChangeOptionBy(
        unrelated, 7, changes);
    assert(handled == 0);
    assert(!changes.any());
    assert(unrelated.byCalls == 0);

    handled = runtimeControls.changeAutoChangeOptionTo(
        unrelated, "anything", changes);
    assert(handled == 0);
    assert(!changes.any());
    assert(unrelated.toCalls == 0);
}

int main() {
    testToggleLockMutatesSettings();
    testGenericAutoChangeOptionsMutateSettings();
    testUnrelatedOptionsAreNotChanged();
    return 0;
}
