/** @file
 * Application-owned interface runtime state and command context.
 */

#include "cthugha.h"
#include "InterfaceRuntime.h"

#include "EffectControl.h"
#include "Interface.h"
#include "RuntimeCommandSink.h"
#include "keymap.h"

#include <strings.h>

InterfaceCommandContext::InterfaceCommandContext()
    : option(NULL)
    , effectControl(NULL)
    , optionElement(NULL) { }

InterfaceRuntime::InterfaceRuntime()
    : currentInterfaceValue(NULL)
    , runtimeConfigRegistryValue(NULL)
    , autoChangerStatusProviderValue(NULL)
    , autoChangeControlsValue(NULL)
    , audioProcessingSelectorValue(NULL)
    , saveToPresetValue(0)
    , showStatusValue(0) { }

void InterfaceRuntime::registerInterface(Interface& interface_) {
    for (std::vector<Interface*>::iterator it = interfacesValue.begin();
         it != interfacesValue.end(); ++it) {
        if (*it == &interface_)
            return;
    }

    interfacesValue.push_back(&interface_);
}

void InterfaceRuntime::set(const char* name) {
    if (name == NULL)
        return;

    for (std::vector<Interface*>::iterator it = interfacesValue.begin();
         it != interfacesValue.end(); ++it) {
        Interface* candidate = *it;
        if (strcasecmp(name, candidate->name) == 0) {
            currentInterfaceValue = candidate;
            clampCurrentSelection();
            return;
        }
    }

    CTH_ERROR("Unknown interface '%s'\n", name);
}

void InterfaceRuntime::runCurrent() {
    if (currentInterfaceValue != NULL)
        currentInterfaceValue->run(*this);
}

void InterfaceRuntime::clampCurrentSelection() {
    if (currentInterfaceValue == NULL)
        return;

    if (currentInterfaceValue->sel < -1)
        currentInterfaceValue->sel = -1;
    if (currentInterfaceValue->sel >= currentInterfaceValue->nElements)
        currentInterfaceValue->sel = currentInterfaceValue->nElements - 1;
}

void InterfaceRuntime::moveSelectionBy(int by) {
    if (currentInterfaceValue == NULL)
        return;

    currentInterfaceValue->sel += by;
    clampCurrentSelection();
}

void InterfaceRuntime::setSelection(int selection) {
    if (currentInterfaceValue == NULL)
        return;

    currentInterfaceValue->sel = selection;
    clampCurrentSelection();
}

void InterfaceRuntime::selectHome() {
    setSelection(-1);
}

void InterfaceRuntime::selectEnd() {
    if (currentInterfaceValue != NULL)
        setSelection(currentInterfaceValue->nElements - 1);
}

void InterfaceRuntime::selectNextInList(const char* const* names) {
    if ((currentInterfaceValue == NULL) || (names == NULL))
        return;

    for (const char* const* name = names; *name != NULL; name++) {
        if (strcasecmp(*name, currentInterfaceValue->name) == 0) {
            if (name[1] != NULL)
                set(name[1]);
            return;
        }
    }
}

void InterfaceRuntime::selectPreviousInList(const char* const* names) {
    if ((currentInterfaceValue == NULL) || (names == NULL))
        return;

    for (const char* const* name = names + 1; *name != NULL; name++) {
        if (strcasecmp(*name, currentInterfaceValue->name) == 0) {
            set(name[-1]);
            return;
        }
    }
}

void InterfaceRuntime::setRuntimeConfigRegistry(RuntimeConfigRegistry* registry) {
    runtimeConfigRegistryValue = registry;
}

const RuntimeConfigRegistry* InterfaceRuntime::runtimeConfigRegistry() const {
    return runtimeConfigRegistryValue;
}

void InterfaceRuntime::setAutoChangerStatusProvider(
    const AutoChangerStatusProvider* provider) {
    autoChangerStatusProviderValue = provider;
}

const AutoChangerStatusProvider* InterfaceRuntime::autoChangerStatusProvider() const {
    return autoChangerStatusProviderValue;
}

void InterfaceRuntime::setAutoChangeControls(AutoChangeControls* controls) {
    autoChangeControlsValue = controls;
}

AutoChangeControls* InterfaceRuntime::autoChangeControls() const {
    return autoChangeControlsValue;
}

void InterfaceRuntime::setAudioProcessingSelector(AudioProcessingSelector* selector) {
    audioProcessingSelectorValue = selector;
}

AudioProcessingSelector* InterfaceRuntime::audioProcessingSelector() const {
    return audioProcessingSelectorValue;
}

void InterfaceRuntime::toggleSaveToPreset() {
    saveToPresetValue = 1 - saveToPresetValue;
}

void InterfaceRuntime::clearSaveToPreset() {
    saveToPresetValue = 0;
}

int InterfaceRuntime::saveToPreset() const {
    return saveToPresetValue;
}

void InterfaceRuntime::toggleStatus() {
    showStatusValue = 1 - showStatusValue;
}

int InterfaceRuntime::showStatus() const {
    return showStatusValue;
}

void InterfaceRuntime::clearCommandContext() {
    commandContextValue = InterfaceCommandContext();
}

int InterfaceRuntime::runOptionKey(Option& option, InterfaceElementOption& element,
    Keymap& keymap, int key) {
    commandContextValue.option = &option;
    commandContextValue.effectControl = NULL;
    commandContextValue.optionElement = &element;
    int result = keymap.action(key);
    clearCommandContext();
    return result;
}

int InterfaceRuntime::runEffectControlKey(EffectControl& effectControl,
    InterfaceElementOption& element, Keymap& effectControlKeymap,
    Keymap& optionKeymap, int key) {
    commandContextValue.option = &effectControl;
    commandContextValue.effectControl = &effectControl;
    commandContextValue.optionElement = &element;

    int result = effectControlKeymap.action(key);
    if (result == 1)
        result = optionKeymap.action(key);

    clearCommandContext();
    return result;
}

int InterfaceRuntime::runEffectChoiceKey(EffectControl& effectControl,
    Option& option, int key) {
    commandContextValue.option = &option;
    commandContextValue.effectControl = &effectControl;
    commandContextValue.optionElement = NULL;

    int result = Keymap::action("ListOption", key);
    if (result)
        result = Keymap::action("Option", key);

    clearCommandContext();
    return result;
}

static void applyInterfaceRuntimeCommand(const RuntimeCommand& command) {
    RuntimeCommandSink* sink = Keymap::runtimeCommandSink();
    if (sink != NULL)
        sink->apply(command);
}

static int interfaceRuntimeIncrement(const InterfaceElementOption& element,
    int incrementIndex) {
    switch (incrementIndex) {
    case 1:
        return element.inc1;
    case 2:
        return element.inc2;
    case 3:
        return element.inc3;
    default:
        return 0;
    }
}

void InterfaceRuntime::changeContextValueByElementIncrement(
    int incrementIndex, double value) {
    if (commandContextValue.optionElement == NULL)
        return;

    int increment = interfaceRuntimeIncrement(
        *commandContextValue.optionElement, incrementIndex);
    if (increment == 0)
        return;

    int step = int(value * increment);
    if (commandContextValue.effectControl != NULL) {
        applyInterfaceRuntimeCommand(RuntimeCommand::changeEffectControlBy(
            *commandContextValue.effectControl, step));
    } else if (commandContextValue.option != NULL) {
        applyInterfaceRuntimeCommand(RuntimeCommand::changeOptionBy(
            *commandContextValue.option, step));
    }
}

void InterfaceRuntime::setContextValueFromElement(double value) {
    if (commandContextValue.optionElement == NULL)
        return;

    char text[128];
    snprintf(text, sizeof(text), "%d",
        int(value * commandContextValue.optionElement->inc1));

    if (commandContextValue.effectControl != NULL) {
        applyInterfaceRuntimeCommand(RuntimeCommand::changeEffectControlTo(
            *commandContextValue.effectControl, text));
        applyInterfaceRuntimeCommand(RuntimeCommand::changeEffectControlBy(
            *commandContextValue.effectControl, 0));
    } else if (commandContextValue.option != NULL) {
        applyInterfaceRuntimeCommand(RuntimeCommand::changeOptionTo(
            *commandContextValue.option, text));
        applyInterfaceRuntimeCommand(RuntimeCommand::changeOptionBy(
            *commandContextValue.option, 0));
    }
}

void InterfaceRuntime::toggleContextEffectControlLock() {
    if (commandContextValue.effectControl != NULL) {
        applyInterfaceRuntimeCommand(RuntimeCommand::toggleEffectControlLock(
            *commandContextValue.effectControl));
    }
}

void InterfaceRuntime::toggleContextEffectChoiceUse() {
    if ((commandContextValue.effectControl == NULL)
        || (currentInterfaceValue == NULL))
        return;

    applyInterfaceRuntimeCommand(RuntimeCommand::toggleEffectChoiceUse(
        *commandContextValue.effectControl, currentInterfaceValue->sel));
}

void InterfaceRuntime::activateContextEffectChoice() {
    if ((commandContextValue.effectControl == NULL)
        || (commandContextValue.option == NULL)
        || (currentInterfaceValue == NULL))
        return;

    applyInterfaceRuntimeCommand(RuntimeCommand::activateEffectControl(
        *commandContextValue.effectControl, currentInterfaceValue->sel));
}
