#include "cthugha.h"
#include "Interface.h"
#include "InterfaceRuntime.h"
#include "AutoChangeControls.h"
#include "SceneChangeStatusProvider.h"
#include "InputQueue.h"
#include "keys.h"
#include "imath.h"
#include "CthughaDisplay.h"
#include "DisplayDevice.h"
#include "AudioProcessing.h"
#include "Border.h"
#include "Flashlight.h"
#include "RuntimeConfigRegistry.h"
#include "RuntimeConfigSelection.h"
#include "RuntimeCommandSink.h"
#include "Scene.h"
#include "Image.h"
#include "flames.h"
#include "TranslationOptions.h"
#include "keymap.h"
#include "waves.h"

#include <ctype.h>
#include <signal.h>
#include <string>

////////////////////////////////////////////////////////////////////////////

//
// class Interface
//
Interface::Interface(const char* n, const char* ti, const char* te)
    : name(n)
    , title(ti)
    , text(te)
    , elements(NULL)
    , nElements(0)
    , sel(-1) { }

Interface::Interface(const char* n, const char* ti, const char* te, InterfaceElement* el[], int nEl)
    : name(n)
    , title(ti)
    , text(te)
    , elements(el)
    , nElements(nEl)
    , sel(-1) { }

Interface::~Interface() { }

void Interface::setElements(InterfaceElement** el, int nEl) {
    elements = el;
    nElements = nEl;
}

static int interfaceRuntimeMilliseconds(InterfaceRuntime& runtime) {
    return runtime.milliseconds();
}

ACTION(up) {
    context.runtime().moveSelectionBy(-int(invocation.value));
}

ACTION(down) {
    context.runtime().moveSelectionBy(int(invocation.value));
}

ACTION(home) {
    context.runtime().selectHome();
}
ACTION(end) {
    context.runtime().selectEnd();
}

ACTION(chgValue1) {
    context.changeValueByElementIncrement(1, invocation.value);
}
ACTION(chgValue2) {
    context.changeValueByElementIncrement(2, invocation.value);
}
ACTION(chgValue3) {
    context.changeValueByElementIncrement(3, invocation.value);
}
ACTION(setValue) {
    context.setValueFromElement(invocation.value);
}

static const char* InterfaceList[] = { "Help", // F1
    "EffectControls", // F2
    "Options", // F3
    "sound", // F5
    "mixer", // F6
    "playList", // F7
    "playList", // F8

    "display", // F9
    "flame", "border", "translate", "wave", "table", "waveScaling", "object",
    "palette", "image", "flashlight", "Help", NULL };

ACTION(nextInterface) {
    context.runtime().selectNextInList(InterfaceList);
}
ACTION(prevInterface) {
    context.runtime().selectPreviousInList(InterfaceList);
}

// default display handler
void Interface::display(InterfaceRuntime& runtime) {
    double line = 0.0;

    if (title) {
        line = displayDevice->print(
            title, 0, 'l', (sel == -1) ? TEXT_COLOR_HIGHLIGHT : TEXT_COLOR_NORMAL);
        line = displayDevice->print(
            "---------------------------------------------", line, 'l', TEXT_COLOR_NORMAL);
    }

    if (text) {
        line = displayDevice->print(text, line, 'l', TEXT_COLOR_NORMAL);
    }

    for (int i = 0; i < nElements; i++) {
        line = displayDevice->print(elements[i]->text(runtime, sel == i), line, 'l',
            (sel == i) ? TEXT_COLOR_HIGHLIGHT : TEXT_COLOR_NORMAL);
    }

    int showStatus = runtime.showStatus();
    if (showStatus) {
        static char str[512];
        const SceneChangeStatusProvider* provider =
            runtime.sceneChangeStatusProvider();
        const char* autoChangeStatus = provider != NULL
            ? provider->sceneChangeStatus()
            : "";
        snprintf(str, sizeof(str), "%s%s", (cthughaDisplay != NULL) ? cthughaDisplay->status() : "",
            autoChangeStatus);

        displayDevice->print(str, text_size.y - 1, 'l', TEXT_COLOR_NORMAL, 1);
    }

    if (runtime.saveToPreset()) {
        displayDevice->print("save to preset slot (press 0..9)", text_size.y - (showStatus ? 2 : 1), 'l',
            TEXT_COLOR_NORMAL);
    }

}

void Interface::doKey(InterfaceRuntime& runtime, KeymapRegistry& keymaps,
    CommandRegistry& commands, CommandDispatcher& dispatcher,
    CommandContext& context, int key) {

    if (dispatcher.dispatchKeymap(keymaps, commands, name, key, context) == 0)
        return;
    dispatcher.dispatchKeymap(keymaps, commands, "default", key, context);
}

// default runner
void Interface::run(InterfaceRuntime& runtime, InputQueue& inputQueue,
    KeymapRegistry& keymaps, CommandRegistry& commands,
    CommandDispatcher& dispatcher, CommandContext& context) {

    this->preRun();

    // handle keys
    int key;
    while ((key = inputQueue.popKey()) != CK_NONE) {

        if ((sel >= 0) && (sel < nElements))
            if (elements[sel]->doKey(runtime, keymaps, commands,
                    dispatcher, context, key) == 0)
                continue;

        this->doKey(runtime, keymaps, commands, dispatcher, context, key);
    }
}

//
// class InterfaceElementOption
//

InterfaceElementOption::InterfaceElementOption(const char* t, Option* o, int i1, int i2, int i3)
    : InterfaceElement(t)
    , opt(o)
    , inc1(i1)
    , inc2(i2)
    , inc3(i3) { }

const char* InterfaceElementOption::text(InterfaceRuntime& /* runtime */,
    int selected) {
    static char strRet[512];
    char fmt[512];
    char in[512];

    snprintf(fmt, sizeof(fmt), "%%c%%-%ds%%c", min(text_size.x - 3, 77));
    snprintf(in, sizeof(in), str, opt->text());

    // make format and include the > <
    snprintf(strRet, sizeof(strRet), fmt, selected ? '>' : ' ', in, selected ? '<' : ' ');

    return strRet;
}

int InterfaceElementOption::doKey(InterfaceRuntime& runtime,
    KeymapRegistry& keymaps, CommandRegistry& commands,
    CommandDispatcher& dispatcher, CommandContext& context, int key) {
    return runtime.runOptionKey(*opt, *this, keymaps, commands, dispatcher,
        context, "OptionElement", key);
}

//
// Effect Control Element
//
InterfaceElementEffectControl::InterfaceElementEffectControl(
    const char* t, EffectControl* o, int i1, int i2, int i3)
    : InterfaceElementOption(t, o, i1, i2, i3)
    , effectControl(o) { }

int InterfaceElementEffectControl::doKey(InterfaceRuntime& runtime,
    KeymapRegistry& keymaps, CommandRegistry& commands,
    CommandDispatcher& dispatcher, CommandContext& context, int key) {
    return runtime.runEffectControlKey(*effectControl, *this, keymaps,
        commands, dispatcher, context, "EffectControlElement",
        "OptionElement", key);
}

ACTION(lockElement) {
    context.toggleEffectControlLock();
}

static const char* runtimeConfigSelectionTextForInterface(
    RuntimeConfigSelectionField field, Option* fallback,
    InterfaceRuntime& runtime) {
    static std::string text;
    const RuntimeConfigRegistry* registry = runtime.runtimeConfigRegistry();
    if (registry == NULL) {
        text = fallback != NULL ? fallback->text() : "";
        return text.c_str();
    }

    Config config = registry->currentConfig();
    text = runtimeConfigSelectionTextOrFallback(config, field,
        fallback != NULL ? fallback->text() : "");
    return text.c_str();
}

class InterfaceElementRuntimeConfigOption : public InterfaceElementOption {
    RuntimeConfigSelectionField field;

public:
    InterfaceElementRuntimeConfigOption(const char* t, Option* o,
        RuntimeConfigSelectionField field_)
        : InterfaceElementOption(t, o)
        , field(field_) { }

    virtual const char* text(InterfaceRuntime& runtime, int selected) {
        static char strRet[512];
        char fmt[512];
        char in[512];

        snprintf(fmt, sizeof(fmt), "%%c%%-%ds%%c", min(text_size.x - 3, 77));
        snprintf(in, sizeof(in), str,
            runtimeConfigSelectionTextForInterface(field, opt, runtime));
        snprintf(strRet, sizeof(strRet), fmt, selected ? '>' : ' ', in, selected ? '<' : ' ');

        return strRet;
    }
};

class InterfaceElementAudioProcessingOption
    : public InterfaceElementRuntimeConfigOption {
    void updateOption(InterfaceRuntime& runtime) {
        AudioProcessingSelector* selector = runtime.audioProcessingSelector();
        opt = (selector != NULL)
            ? static_cast<Option*>(&selector->option())
            : static_cast<Option*>(&optionDummy);
    }

public:
    InterfaceElementAudioProcessingOption(const char* t,
        RuntimeConfigSelectionField field_)
        : InterfaceElementRuntimeConfigOption(t, &optionDummy, field_) { }

    virtual const char* text(InterfaceRuntime& runtime, int selected) {
        updateOption(runtime);
        return InterfaceElementRuntimeConfigOption::text(runtime, selected);
    }

    virtual int doKey(InterfaceRuntime& runtime, KeymapRegistry& keymaps,
        CommandRegistry& commands, CommandDispatcher& dispatcher,
        CommandContext& context, int key) {
        updateOption(runtime);
        return InterfaceElementOption::doKey(runtime, keymaps, commands,
            dispatcher, context, key);
    }
};

class InterfaceElementAutoChangeOption : public InterfaceElementOption {
    AutoChangeControlField field;

    void updateOption(InterfaceRuntime& runtime) {
        AutoChangeControls* controls = runtime.autoChangeControls();
        opt = (controls != NULL) ? &controls->option(field) : &optionDummy;
    }

public:
    InterfaceElementAutoChangeOption(const char* t,
        AutoChangeControlField field_, int i1 = 1, int i2 = 10, int i3 = 100)
        : InterfaceElementOption(t, &optionDummy, i1, i2, i3)
        , field(field_) { }

    virtual const char* text(InterfaceRuntime& runtime, int selected) {
        updateOption(runtime);
        return InterfaceElementOption::text(runtime, selected);
    }

    virtual int doKey(InterfaceRuntime& runtime, KeymapRegistry& keymaps,
        CommandRegistry& commands, CommandDispatcher& dispatcher,
        CommandContext& context, int key) {
        updateOption(runtime);
        return InterfaceElementOption::doKey(runtime, keymaps, commands,
            dispatcher, context, key);
    }
};

class InterfaceElementRuntimeConfigEffectControl
    : public InterfaceElementEffectControl {
    RuntimeConfigSelectionField field;

public:
    InterfaceElementRuntimeConfigEffectControl(const char* t, EffectControl* o,
        RuntimeConfigSelectionField field_)
        : InterfaceElementEffectControl(t, o)
        , field(field_) { }

    virtual const char* text(InterfaceRuntime& runtime, int selected) {
        static char strRet[512];
        char fmt[512];
        char in[512];

        snprintf(fmt, sizeof(fmt), "%%c%%-%ds%%c", min(text_size.x - 3, 77));
        snprintf(in, sizeof(in), str,
            runtimeConfigSelectionTextForInterface(field, effectControl,
                runtime));
        snprintf(strRet, sizeof(strRet), fmt, selected ? '>' : ' ', in,
            selected ? '<' : ' ');

        return strRet;
    }
};

void ErrorMessages::addMessage(const char* text, InterfaceRuntime& runtime) {
    if (nMsgs == 128) {
        CTH_ERROR("too many errors: %s\n", text);
        return;
    }
    strncpy(msgs[nMsgs], text, 128);
    on_screen[nMsgs] = interfaceRuntimeMilliseconds(runtime);

    nMsgs++;
}
void ErrorMessages::display(InterfaceRuntime& runtime) {

    // bring messages to screen
    for (int i = 0; i < nMsgs; i++) {
        displayDevice->print(msgs[i], -(nMsgs - i), 'r', TEXT_COLOR_ERROR);
    }

    // remove old messages
    const int errorTime = 3000;
    const int currentTime = interfaceRuntimeMilliseconds(runtime);
    while ((nMsgs > 0) && ((currentTime - on_screen[0]) > errorTime)) {
        for (int i = 1; i < nMsgs; i++) {
            strncpy(msgs[i - 1], msgs[i], 128);
            on_screen[i - 1] = on_screen[i];
        }
        nMsgs--;
    }
}

//
// the interfaces
//
class InterfaceMain : public Interface {
public:
    InterfaceMain()
        : Interface("main", NULL, NULL) { }

    void doKey(InterfaceRuntime& runtime, KeymapRegistry& keymaps,
        CommandRegistry& commands, CommandDispatcher& dispatcher,
        CommandContext& context, int key) {
        const char* extraKeymap = runtime.extraKeymap();
        if (extraKeymap[0] != '\0') {
            if (dispatcher.dispatchKeymap(keymaps, commands, extraKeymap,
                    key, context) == 0)
                return;
        }
        Interface::doKey(runtime, keymaps, commands, dispatcher, context, key);
    }
};

ACTION(setExtraKeymap) { context.runtime().setExtraKeymap(invocation.param); }

class InterfaceEffectControl : public Interface {
    ImageOption& images;

public:
    explicit InterfaceEffectControl(ImageOption& images_)
        : Interface("EffectControls", "Effect Controls", NULL)
        , images(images_) {

        nElements = 13;
        elements = new InterfaceElement*[nElements];

        {
            elements[0] = new InterfaceElementRuntimeConfigEffectControl(
                "Display (d,D)         : %s", &screen,
                RuntimeConfigSelectionDisplay);
            elements[1] = new InterfaceElementRuntimeConfigEffectControl(
                "Flame (f,F)           : %s", &flame,
                RuntimeConfigSelectionFlame);
            elements[2]
                = new InterfaceElementRuntimeConfigEffectControl(
                    "General Flame (g)     : %s", &flameGeneral,
                    RuntimeConfigSelectionGeneralFlame);
            elements[3] = new InterfaceElementRuntimeConfigEffectControl(
                "Border (=)            : %s", &border,
                RuntimeConfigSelectionBorder);
            elements[4]
                = new InterfaceElementRuntimeConfigEffectControl(
                    "Translate (t,T)       : %s", &translation,
                    RuntimeConfigSelectionTranslation);
            elements[5] = new InterfaceElementRuntimeConfigEffectControl(
                "Wave (w)              : %s", &wave,
                RuntimeConfigSelectionWave);
            elements[6]
                = new InterfaceElementAudioProcessingOption(
                    "Sound Processing (m,M): %s",
                    RuntimeConfigSelectionAudioProcessing);
            elements[7] = new InterfaceElementRuntimeConfigEffectControl(
                "Table (b,B)           : %s", &table,
                RuntimeConfigSelectionTable);
            elements[8] = new InterfaceElementRuntimeConfigEffectControl(
                "WaveScale (W)         : %s", &waveScale,
                RuntimeConfigSelectionWaveScale);
            elements[9]
                = new InterfaceElementRuntimeConfigEffectControl(
                    "Palette (p,P)         : %s", &palette,
                    RuntimeConfigSelectionPalette);
            elements[10]
                = new InterfaceElementRuntimeConfigEffectControl(
                    "Image (x,X))          : %s",
                    &images,
                    RuntimeConfigSelectionImage);
            elements[11] = new InterfaceElementRuntimeConfigEffectControl(
                "3D-Object (j,J)       : %s", &object,
                RuntimeConfigSelectionObject);
            elements[12]
                = new InterfaceElementRuntimeConfigEffectControl(
                    "Flashlight (s)        : %s", &flashlight,
                    RuntimeConfigSelectionFlashlight);
        }
    }

    ~InterfaceEffectControl() {
        for (int i = 0; i < nElements; i++)
            delete elements[i];
        delete[] elements;
    }

    void preRun() {
#define O(i)                                                                                       \
    ((InterfaceElementOption*)elements[i])->opt                                                    \
        = ((InterfaceElementEffectControl*)elements[i])->effectControl
        O(1) = &flame;
        O(2) = &flameGeneral;
        O(3) = &border;
        O(4) = &translation;
        O(5) = &wave;
        O(7) = &table;
        O(8) = &waveScale;
        O(9) = &palette;
        O(10) = &images;
        O(11) = &object;
        O(12) = &flashlight;
    }
    void doKey(InterfaceRuntime& runtime, KeymapRegistry& keymaps,
        CommandRegistry& commands, CommandDispatcher& dispatcher,
        CommandContext& context, int key) {
        (void)runtime;
        if (dispatcher.dispatchKeymap(keymaps, commands, "EffectControls",
                key, context) == 0)
            return;
        if (dispatcher.dispatchKeymap(keymaps, commands, "Options", key,
                context) == 0)
            return;
        dispatcher.dispatchKeymap(keymaps, commands, "default", key, context);
    }

};

class InterfaceOptions : public Interface {
public:
    explicit InterfaceOptions(Option& quietMessageOption)
        : Interface("Options", "Options", NULL) {
        nElements = 9;
        elements = new InterfaceElement*[nElements];
        elements[0] = new InterfaceElementOption(
            "Maximal Frames/second    : %10s", &maxFramesPerSecond);
        elements[1] = new InterfaceElementOption(
            "Zoom (0=max)             : %10s", &zoom);
        elements[2] = new InterfaceElementAutoChangeOption(
            "Minimal time btw. change : %10s",
            AutoChangeControlWaitMinMs, 100, 500, 1000);
        elements[3] = new InterfaceElementAutoChangeOption(
            "Extra random time        : %10s",
            AutoChangeControlWaitRandomMs, 100, 500, 1000);
        elements[4] = new InterfaceElementAutoChangeOption(
            "Quiet change time        : %10s",
            AutoChangeControlQuietMs, 100, 500, 1000);
        elements[5] = new InterfaceElementOption(
            "Time before silence Msg. : %10s", &quietMessageOption, 100, 500, 1000);
        elements[6] = new InterfaceElementAutoChangeOption(
            "Cumulative fire level    : %10s",
            AutoChangeControlCumulativeFireLevel, 10, 50, 100);
        elements[7] = new InterfaceElementAutoChangeOption(
            "Little changes only      : %10s",
            AutoChangeControlChangeLittle);
        elements[8] = new InterfaceElementAutoChangeOption(
            "Lock                     : %10s",
            AutoChangeControlLocked);
    }

    ~InterfaceOptions() {
        for (int i = 0; i < nElements; i++)
            delete elements[i];
        delete[] elements;
    }
};

void registerListInterfaces(InterfaceRuntime& runtime, ImageOption& images);
void registerHelpInterface(InterfaceRuntime& runtime);
void registerCreditsInterface(InterfaceRuntime& runtime);
void registerAudioInterfaces(InterfaceRuntime& runtime);
void registerListKeyActions(CommandRegistry& registry);
void registerHelpKeyActions(CommandRegistry& registry);

void registerInterfaceKeyActions(CommandRegistry& registry) {
#define REGISTER_ACTION(a) registry.registerAction(new a##Action())
    REGISTER_ACTION(up);
    REGISTER_ACTION(down);
    REGISTER_ACTION(home);
    REGISTER_ACTION(end);
    REGISTER_ACTION(chgValue1);
    REGISTER_ACTION(chgValue2);
    REGISTER_ACTION(chgValue3);
    REGISTER_ACTION(setValue);
    REGISTER_ACTION(nextInterface);
    REGISTER_ACTION(prevInterface);
    REGISTER_ACTION(lockElement);
    REGISTER_ACTION(setExtraKeymap);
#undef REGISTER_ACTION

    registerListKeyActions(registry);
    registerHelpKeyActions(registry);
}

void registerDefaultInterfaces(InterfaceRuntime& runtime, ImageOption& images,
    Option& quietMessageOption) {
    runtime.registerOwnedInterface(new InterfaceMain());
    runtime.registerOwnedInterface(new Interface("Mixer", "Mixer", NULL));
    runtime.registerOwnedInterface(new InterfaceEffectControl(images));
    runtime.registerOwnedInterface(new InterfaceOptions(quietMessageOption));
    registerAudioInterfaces(runtime);
    registerListInterfaces(runtime, images);
    registerHelpInterface(runtime);
    registerCreditsInterface(runtime);
}
