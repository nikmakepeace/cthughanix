/** @file
 * Integration coverage for the visualiser control service and panel client.
 */

#include "ControlPanelClient.h"
#include "ControlService.h"
#include "ControlTransport.h"
#include "ProcessServices.h"
#include "RuntimeConfigRegistry.h"
#include "RuntimeCommandSink.h"
#include "SceneChoiceSelection.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include <chrono>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifndef _WIN32
#include <unistd.h>
#endif

int cth_log_enabled(int) {
    return 0;
}

int cth_log(int, const char*, ...) {
    return 0;
}

int cth_log_error(const char*, ...) {
    return 0;
}

class QuietLogSink : public LogSink {
public:
    virtual int enabled(int) const {
        return 0;
    }

protected:
    virtual void write(int, const char*, int, const char*, va_list) {
    }
};

class RecordingLauncher : public ControlPanelProcessLauncher {
public:
    int calls;
    std::string endpointValue;

    RecordingLauncher()
        : calls(0)
        , endpointValue() { }

    virtual bool launchPanel(
        const std::string& endpoint, std::string*) {
        calls++;
        endpointValue = endpoint;
        return true;
    }
};

class FakeChoice : public SceneChoice {
    std::string nameValue;
    int inUseValue;

public:
    FakeChoice(const char* name_, int inUse_)
        : nameValue(name_)
        , inUseValue(inUse_) { }

    virtual const char* name() const { return nameValue.c_str(); }
    virtual int sameName(const char* other) const {
        return nameValue == (other != 0 ? other : "");
    }
    virtual int inUse() const { return inUseValue; }
    virtual void setUse(int inUse) { inUseValue = inUse; }
};

class FakeSelection : public SceneFlameSelection,
                      public SceneGeneralFlameSelection,
                      public SceneWaveSelection,
                      public SceneWaveObjectSelection,
                      public SceneTranslationSelection,
                      public ScenePaletteSelection,
                      public SceneImageSelection {
    std::vector<FakeChoice> choices;
    int currentValueValue;

public:
    FakeSelection()
        : choices()
        , currentValueValue(0) {
        choices.push_back(FakeChoice("first", 1));
        choices.push_back(FakeChoice("second", 1));
    }

    virtual const char* currentName() const {
        return choices[currentValueValue].name();
    }
    virtual int currentValue() const { return currentValueValue; }
    virtual int entryCount() const { return int(choices.size()); }
    virtual int choiceCount() const { return int(choices.size()); }
    virtual SceneChoice* choiceAt(int index) {
        if (index < 0 || index >= int(choices.size()))
            return 0;
        return &choices[index];
    }
    virtual const SceneChoice* choiceAt(int index) const {
        if (index < 0 || index >= int(choices.size()))
            return 0;
        return &choices[index];
    }
    virtual void change(int by) {
        currentValueValue = (currentValueValue + by) % int(choices.size());
    }
    virtual void change(const char*, RandomSource&) { currentValueValue = 1; }
    virtual int changeRandom(RandomSource&) { currentValueValue = 0; return 1; }
    virtual void setValue(int index) { currentValueValue = index; }
    virtual const Flame* currentFlame() { return 0; }
    virtual int encodedValue() const { return currentValueValue; }
    virtual const char* selectionText() const { return currentName(); }
    virtual Wave* currentWave() { return 0; }
    virtual WObject* currentObject() { return 0; }
    virtual TranslationTable currentTranslationTable() {
        return TranslationTable();
    }
    virtual PaletteEntry* currentPaletteEntry() { return 0; }
    virtual const IndexedImage* currentImage() { return 0; }
};

class FakeSelections : public SceneVisualSelections {
    FakeSelection selection;

public:
    virtual SceneFlameSelection& flame() { return selection; }
    virtual SceneGeneralFlameSelection& generalFlame() { return selection; }
    virtual SceneWaveSelection& wave() { return selection; }
    virtual SceneOptionSelection& waveScale() { return selection; }
    virtual SceneOptionSelection& table() { return selection; }
    virtual SceneOptionSelection& object() { return selection; }
    virtual SceneTranslationSelection& translation() { return selection; }
    virtual ScenePaletteSelection& palette() { return selection; }
    virtual SceneOptionSelection& border() { return selection; }
    virtual SceneOptionSelection& flashlight() { return selection; }
    virtual SceneImageSelection& images() { return selection; }
};

static Config sampleConfig() {
    Config config;
    config.scene.flame = "first";
    config.scene.translation = "first";
    config.scene.image = "first";
    config.scene.object = "first";
    config.scene.table = "first";
    config.scene.waveScale = "first";
    config.scene.palette = "first";
    config.scene.flashlight = "off";
    config.scene.audioProcessing = "none";
    config.display.maxFramesPerSecond = 25;
    return config;
}

class UpdatingRuntimeSink : public RuntimeCommandSink {
    RuntimeConfigRegistry& registry;
    Config configValue;

public:
    int calls;
    RuntimeCommandType lastType;
    std::string lastText;
    int lastValue;

    UpdatingRuntimeSink(RuntimeConfigRegistry& registry_, const Config& config_)
        : registry(registry_)
        , configValue(config_)
        , calls(0)
        , lastType(RuntimeCommandChangeAll)
        , lastText()
        , lastValue(0) { }

    virtual RuntimeChangeSet apply(const RuntimeCommand& command) {
        RuntimeChangeSet changes;
        calls++;
        lastType = command.type;
        lastText = command.text != 0 ? command.text : "";
        lastValue = command.value;

        if (command.type == RuntimeCommandChangeMaxFpsTo) {
            configValue.display.maxFramesPerSecond = command.value;
            registry.setBaseline(configValue);
            changes.displayChanged = 1;
            changes.fpsChanged = 1;
        } else if (command.type == RuntimeCommandChangeSceneTo) {
            if (command.sceneTarget == RuntimeSceneFlame)
                configValue.scene.flame = lastText;
            registry.setBaseline(configValue);
            changes.sceneChanges = 1;
        } else if (command.type == RuntimeCommandChangeSoundProcessingTo) {
            configValue.scene.audioProcessing = lastText;
            registry.setBaseline(configValue);
            changes.audioProcessingChanged = 1;
        }

        return changes;
    }

    void setAppFlame(const char* flame) {
        configValue.scene.flame = flame;
        registry.setBaseline(configValue);
    }
};

struct ObservedMessages {
    int connected;
    int disconnected;
    int errors;
    std::vector<ControlJsonValue> messages;

    ObservedMessages()
        : connected(0)
        , disconnected(0)
        , errors(0)
        , messages() { }
};

static std::string typeOf(const ControlJsonValue& message) {
    const ControlJsonValue* type = message.member("type");
    if (type == 0 || type->type() != ControlJsonValue::StringType)
        return "";
    return type->asString();
}

static const ControlJsonValue* latestMessageOfType(
    const ObservedMessages& observed, const char* type) {
    for (std::vector<ControlJsonValue>::const_reverse_iterator it
             = observed.messages.rbegin();
         it != observed.messages.rend(); ++it) {
        if (typeOf(*it) == type)
            return &*it;
    }
    return 0;
}

static int latestStateMaxFps(
    const ObservedMessages& observed, int fallback) {
    const ControlJsonValue* state = latestMessageOfType(observed, "state");
    const ControlJsonValue* display
        = state != 0 ? state->member("display") : 0;
    const ControlJsonValue* maxFps
        = display != 0 ? display->member("maxFps") : 0;
    return maxFps != 0 ? int(maxFps->asNumber(fallback)) : fallback;
}

static std::string latestStateFlame(const ObservedMessages& observed) {
    const ControlJsonValue* state = latestMessageOfType(observed, "state");
    const ControlJsonValue* scene = state != 0 ? state->member("scene") : 0;
    const ControlJsonValue* flame = scene != 0 ? scene->member("flame") : 0;
    return flame != 0 ? flame->asString() : "";
}

static int latestAckId(const ObservedMessages& observed) {
    const ControlJsonValue* ack = latestMessageOfType(observed, "ack");
    const ControlJsonValue* id = ack != 0 ? ack->member("id") : 0;
    return id != 0 ? int(id->asNumber(0)) : 0;
}

static void pump(ControlService& service, ControlPanelClient& client,
    ObservedMessages& observed, int milliseconds) {
    std::chrono::steady_clock::time_point deadline
        = std::chrono::steady_clock::now()
        + std::chrono::milliseconds(milliseconds);
    while (std::chrono::steady_clock::now() < deadline) {
        service.serviceFrame(8);
        std::vector<ControlPanelClientEvent> events = client.pollEvents();
        for (std::vector<ControlPanelClientEvent>::const_iterator it
                 = events.begin();
             it != events.end(); ++it) {
            if (it->type == ControlPanelClientEvent::Connected)
                observed.connected++;
            else if (it->type == ControlPanelClientEvent::Disconnected)
                observed.disconnected++;
            else if (it->type == ControlPanelClientEvent::Error)
                observed.errors++;
            else if (it->type == ControlPanelClientEvent::Message)
                observed.messages.push_back(it->message);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

static std::string uniqueInstanceId() {
    std::ostringstream out;
    out << "integration-"
        << std::chrono::steady_clock::now().time_since_epoch().count();
    return out.str();
}

static void testServiceClientSynchronizesBothDirections() {
#ifndef _WIN32
    setenv("XDG_RUNTIME_DIR", "/tmp", 1);
#endif
    Config config = sampleConfig();
    RuntimeConfigRegistry registry(config);
    UpdatingRuntimeSink runtimeSink(registry, config);
    FakeSelections selections;
    QuietLogSink log;
    RecordingLauncher launcher;
    ControlService service(runtimeSink, registry, selections, log, launcher);

    std::string error;
    assert(service.start(uniqueInstanceId(), &error));

    service.launchControlPanel();
    service.launchControlPanel();
    assert(launcher.calls == 1);
    assert(launcher.endpointValue == service.endpoint());

    ControlPanelClient client(service.endpoint());
    client.start();
    ObservedMessages observed;
    pump(service, client, observed, 500);

    assert(observed.connected == 1);
    assert(observed.errors == 0);
    assert(latestMessageOfType(observed, "catalogs") != 0);
    assert(latestStateMaxFps(observed, -1) == 25);

    service.launchControlPanel();
    assert(launcher.calls == 1);

    int id = client.sendSetNumber("display.maxFps", 72);
    pump(service, client, observed, 500);
    assert(runtimeSink.calls == 1);
    assert(runtimeSink.lastType == RuntimeCommandChangeMaxFpsTo);
    assert(runtimeSink.lastValue == 72);
    assert(latestAckId(observed) == id);
    assert(latestStateMaxFps(observed, -1) == 72);

    runtimeSink.setAppFlame("second");
    service.runtimeStateChanged();
    pump(service, client, observed, 500);
    assert(latestStateFlame(observed) == "second");

    client.stop();
    service.stop();
}

static void testServiceFrameIsBoundedWithStalledClient() {
#ifndef _WIN32
    setenv("XDG_RUNTIME_DIR", "/tmp", 1);
#endif
    Config config = sampleConfig();
    RuntimeConfigRegistry registry(config);
    UpdatingRuntimeSink runtimeSink(registry, config);
    FakeSelections selections;
    QuietLogSink log;
    RecordingLauncher launcher;
    ControlService service(runtimeSink, registry, selections, log, launcher);

    std::string error;
    assert(service.start(uniqueInstanceId(), &error));
    std::unique_ptr<ControlStream> stalled = connectControlEndpoint(
        service.endpoint(), 1000, &error);
    assert(stalled.get() != 0);

    std::chrono::steady_clock::time_point started
        = std::chrono::steady_clock::now();
    for (int i = 0; i < 200; i++) {
        service.runtimeStateChanged();
        service.serviceFrame(4);
    }
    std::chrono::steady_clock::duration elapsed
        = std::chrono::steady_clock::now() - started;

    stalled->close();
    service.stop();

    assert(std::chrono::duration_cast<std::chrono::milliseconds>(elapsed)
            .count()
        < 250);
}

int main() {
    testServiceClientSynchronizesBothDirections();
    testServiceFrameIsBoundedWithStalledClient();
    return 0;
}
