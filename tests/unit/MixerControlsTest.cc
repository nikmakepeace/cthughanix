/** @file
 * Unit coverage for mixer Option adapters.
 */

#include "Mixer.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>

int cth_log_enabled(int) { return 0; }
int cth_log(int, const char*, ...) { return 0; }
int cth_log_context(int, const char*, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }
int cth_log_errno(int, const char*, ...) { return 0; }

Option::~Option() { }

class FakeMixerDevice : public MixerDevice {
public:
    std::vector<MixerChannel> discoveredChannels;
    int setVolumeCalls;
    int setVolumeEncodedVolume;
    int returnedActive;

    FakeMixerDevice()
        : discoveredChannels()
        , setVolumeCalls(0)
        , setVolumeEncodedVolume(0)
        , returnedActive(1) { }

    virtual int initialize(const std::string&,
        const std::vector<MixerInitialVolume>&,
        std::vector<MixerChannel>& channels) {
        channels = discoveredChannels;
        return 0;
    }

    virtual int setVolume(const std::string&, const MixerChannel&,
        int encodedVolume, int& active) {
        setVolumeCalls++;
        setVolumeEncodedVolume = encodedVolume;
        active = returnedActive;
        return 0;
    }
};

class RecordingOption : public Option {
public:
    RecordingOption()
        : Option("recording") { }

    virtual void change(int) { }
    virtual void change(const char*) { }
    virtual const char* text() const { return "recording"; }
};

static void testMixerControlsBuildOptionsFromSessionChannels() {
    FakeMixerDevice device;
    device.discoveredChannels.push_back(
        MixerChannel("line", 1, 10 + 256 * 20, 1));
    MixerSession session(device, "/dev/mixer-test",
        std::vector<MixerInitialVolume>());
    assert(session.initialize() == 0);

    MixerControls controls(session);

    assert(controls.optionCount() == 1);
    assert(controls.optionAt(0) != NULL);
    assert(controls.optionAt(1) == NULL);
    assert(strstr(controls.optionAt(0)->text(), "10:20") != NULL);
    assert(strchr(controls.optionAt(0)->text(), '*') != NULL);
}

static void testMixerControlsMutateOwnedSession() {
    FakeMixerDevice device;
    device.discoveredChannels.push_back(
        MixerChannel("mic", 2, 3 + 256 * 4, 1));
    MixerSession session(device, "/dev/mixer-test",
        std::vector<MixerInitialVolume>());
    assert(session.initialize() == 0);
    MixerControls controls(session);
    Option* option = controls.optionAt(0);

    assert(controls.changeOptionBy(*option, 5) == 1);
    assert(device.setVolumeCalls == 1);
    assert(device.setVolumeEncodedVolume == 8 + 256 * 9);
    assert(session.channels()[0].encodedVolume == 8 + 256 * 9);
    assert(strstr(option->text(), "  8:9") != NULL);

    assert(controls.changeOptionTo(*option, "12") == 1);
    assert(device.setVolumeCalls == 2);
    assert(device.setVolumeEncodedVolume == 12 + 256 * 12);
    assert(session.channels()[0].encodedVolume == 12 + 256 * 12);
    assert(strstr(option->text(), "12:12") != NULL);
}

static void testMixerControlsIgnoreUnrelatedOptions() {
    FakeMixerDevice device;
    device.discoveredChannels.push_back(MixerChannel("pcm", 3, 1, 1));
    MixerSession session(device, "/dev/mixer-test",
        std::vector<MixerInitialVolume>());
    assert(session.initialize() == 0);
    MixerControls controls(session);
    RecordingOption unrelated;

    assert(controls.changeOptionBy(unrelated, 1) == 0);
    assert(controls.changeOptionTo(unrelated, "12") == 0);
    assert(device.setVolumeCalls == 0);
}

int main() {
    testMixerControlsBuildOptionsFromSessionChannels();
    testMixerControlsMutateOwnedSession();
    testMixerControlsIgnoreUnrelatedOptions();
    return 0;
}
