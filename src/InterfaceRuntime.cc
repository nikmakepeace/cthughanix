/** @file
 * Application-owned interface runtime state and command context.
 */

#include "cthugha.h"
#include "InterfaceRuntime.h"

#include "EffectControl.h"
#include "InputQueue.h"
#include "Interface.h"
#include "ProcessServices.h"
#include "RuntimeCommandTargets.h"
#include "keymap.h"

#include <string.h>
#include <strings.h>

InterfaceCommandContext::InterfaceCommandContext()
    : option(NULL)
    , effectControl(NULL)
    , optionElement(NULL) { }

InterfaceRuntime::InterfaceRuntime(MillisecondClock& clock_)
    : currentInterfaceValue(NULL)
    , runtimeConfigRegistryValue(NULL)
    , autoChangerStatusProviderValue(NULL)
    , autoChangeControlsValue(NULL)
    , audioProcessingSelectorValue(NULL)
    , runtimeCommandSinkValue(NULL)
    , commandRouterValue(NULL)
    , clock(clock_)
    , saveToPresetValue(0)
    , showStatusValue(0)
    , helpScrollPositionValue(0.0)
    , helpScrollingValue(0)
    , creditsPositionValue(0.0)
    , creditsFirstTimeValue(-1) {
    extraKeymapValue[0] = '\0';
}

InterfaceRuntime::~InterfaceRuntime() {
    for (std::vector<Interface*>::iterator it = ownedInterfacesValue.begin();
         it != ownedInterfacesValue.end(); ++it)
        delete *it;
}

void InterfaceRuntime::registerInterface(Interface& interface_) {
    for (std::vector<Interface*>::iterator it = interfacesValue.begin();
         it != interfacesValue.end(); ++it) {
        if (*it == &interface_)
            return;
    }

    interfacesValue.push_back(&interface_);
}

void InterfaceRuntime::registerOwnedInterface(Interface* interface_) {
    if (interface_ == NULL)
        return;

    ownedInterfacesValue.push_back(interface_);
    registerInterface(*interface_);
}

Interface* InterfaceRuntime::find(const char* name) {
    if (name == NULL)
        return NULL;

    for (std::vector<Interface*>::iterator it = interfacesValue.begin();
         it != interfacesValue.end(); ++it) {
        Interface* candidate = *it;
        if (strcasecmp(name, candidate->name) == 0)
            return candidate;
    }

    return NULL;
}

const Interface* InterfaceRuntime::find(const char* name) const {
    if (name == NULL)
        return NULL;

    for (std::vector<Interface*>::const_iterator it = interfacesValue.begin();
         it != interfacesValue.end(); ++it) {
        const Interface* candidate = *it;
        if (strcasecmp(name, candidate->name) == 0)
            return candidate;
    }

    return NULL;
}

void InterfaceRuntime::set(const char* name) {
    if (name == NULL)
        return;

    Interface* candidate = find(name);
    if (candidate != NULL) {
        currentInterfaceValue = candidate;
        clampCurrentSelection();
        return;
    }

    CTH_ERROR("Unknown interface '%s'\n", name);
}

void InterfaceRuntime::runCurrent(InputQueue& inputQueue,
    KeymapRegistry& keymaps) {
    if (currentInterfaceValue != NULL)
        currentInterfaceValue->run(*this, inputQueue, keymaps);
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

void InterfaceRuntime::setRuntimeCommandSink(RuntimeCommandSink* sink) {
    runtimeCommandSinkValue = sink;
}

RuntimeCommandSink* InterfaceRuntime::runtimeCommandSink() const {
    return runtimeCommandSinkValue;
}

void InterfaceRuntime::setCommandRouter(RuntimeCommandTargetRouter* router) {
    commandRouterValue = router;
}

RuntimeCommandTargetRouter* InterfaceRuntime::commandRouter() const {
    return commandRouterValue;
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

void InterfaceRuntime::setExtraKeymap(const char* name) {
    if (name == NULL) {
        extraKeymapValue[0] = '\0';
        return;
    }

    strncpy(extraKeymapValue, name, sizeof(extraKeymapValue) - 1);
    extraKeymapValue[sizeof(extraKeymapValue) - 1] = '\0';
}

const char* InterfaceRuntime::extraKeymap() const {
    return extraKeymapValue;
}

void InterfaceRuntime::toggleHelpScrolling() {
    helpScrollingValue = 1 - helpScrollingValue;
}

void InterfaceRuntime::scrollHelpBy(double by, int lineCount) {
    helpScrollPositionValue += by;
    if ((helpScrollPositionValue < 0.0) && (lineCount > 0))
        helpScrollPositionValue += lineCount;
    helpScrollingValue = 0;
}

void InterfaceRuntime::advanceHelpScroll(double by) {
    helpScrollPositionValue += by;
}

double InterfaceRuntime::helpScrollPosition() const {
    return helpScrollPositionValue;
}

int InterfaceRuntime::helpScrolling() const {
    return helpScrollingValue;
}

double InterfaceRuntime::updateCreditsPosition(int currentTime, int textHeight) {
    if (creditsFirstTimeValue == -1)
        creditsFirstTimeValue = currentTime;

    int timeDiff = currentTime - creditsFirstTimeValue;
    creditsPositionValue = -(double(textHeight) * 0.8) + double(timeDiff) / 250.0;
    return creditsPositionValue;
}

double InterfaceRuntime::creditsPosition() const {
    return creditsPositionValue;
}

int InterfaceRuntime::milliseconds() const {
    return clock.milliseconds();
}

void InterfaceRuntime::clearCommandContext() {
    commandContextValue = InterfaceCommandContext();
}

int InterfaceRuntime::runOptionKey(Option& option, InterfaceElementOption& element,
    KeymapRegistry& keymaps, const char* keymapName, int key) {
    commandContextValue.option = &option;
    commandContextValue.effectControl = NULL;
    commandContextValue.optionElement = &element;
    int result = keymaps.action(keymapName, key, *this);
    clearCommandContext();
    return result;
}

int InterfaceRuntime::runEffectControlKey(EffectControl& effectControl,
    InterfaceElementOption& element, KeymapRegistry& keymaps,
    const char* effectControlKeymapName, const char* optionKeymapName,
    int key) {
    commandContextValue.option = &effectControl;
    commandContextValue.effectControl = &effectControl;
    commandContextValue.optionElement = &element;

    int result = keymaps.action(effectControlKeymapName, key, *this);
    if (result == 1)
        result = keymaps.action(optionKeymapName, key, *this);

    clearCommandContext();
    return result;
}

int InterfaceRuntime::runEffectChoiceKey(EffectControl& effectControl,
    Option& option, KeymapRegistry& keymaps, int key) {
    commandContextValue.option = &option;
    commandContextValue.effectControl = &effectControl;
    commandContextValue.optionElement = NULL;

    int result = keymaps.action("ListOption", key, *this);
    if (result)
        result = keymaps.action("Option", key, *this);

    clearCommandContext();
    return result;
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
    if ((increment == 0) || (commandRouterValue == NULL))
        return;

    int step = int(value * increment);
    if (commandContextValue.effectControl != NULL) {
        commandRouterValue->changeEffectControlBy(
            *commandContextValue.effectControl, step);
    } else if (commandContextValue.option != NULL) {
        commandRouterValue->changeOptionBy(*commandContextValue.option, step);
    }
}

void InterfaceRuntime::setContextValueFromElement(double value) {
    if ((commandContextValue.optionElement == NULL)
        || (commandRouterValue == NULL))
        return;

    char text[128];
    snprintf(text, sizeof(text), "%d",
        int(value * commandContextValue.optionElement->inc1));

    if (commandContextValue.effectControl != NULL) {
        commandRouterValue->changeEffectControlTo(
            *commandContextValue.effectControl, text);
        commandRouterValue->changeEffectControlBy(
            *commandContextValue.effectControl, 0);
    } else if (commandContextValue.option != NULL) {
        commandRouterValue->changeOptionTo(*commandContextValue.option, text);
        commandRouterValue->changeOptionBy(*commandContextValue.option, 0);
    }
}

void InterfaceRuntime::toggleContextEffectControlLock() {
    if ((commandContextValue.effectControl != NULL)
        && (commandRouterValue != NULL))
        commandRouterValue->toggleEffectControlLock(
            *commandContextValue.effectControl);
}

void InterfaceRuntime::toggleContextEffectChoiceUse() {
    if ((commandContextValue.effectControl == NULL)
        || (currentInterfaceValue == NULL)
        || (commandRouterValue == NULL))
        return;

    commandRouterValue->toggleEffectChoiceUse(
        *commandContextValue.effectControl, currentInterfaceValue->sel);
}

void InterfaceRuntime::activateContextEffectChoice() {
    if ((commandContextValue.effectControl == NULL)
        || (commandContextValue.option == NULL)
        || (currentInterfaceValue == NULL)
        || (commandRouterValue == NULL))
        return;

    commandRouterValue->activateEffectControl(
        *commandContextValue.effectControl, currentInterfaceValue->sel);
}
