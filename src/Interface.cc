#include "cthugha.h"
#include "Interface.h"
#include "InterfaceRuntime.h"
#include "AutoChangeControls.h"
#include "AutoChangerStatusProvider.h"
#include "keys.h"
#include "imath.h"
#include "CthughaBuffer.h"
#include "CthughaDisplay.h"
#include "DisplayDevice.h"
#include "AudioProcessing.h"
#include "Border.h"
#include "Flashlight.h"
#include "RuntimeConfigRegistry.h"
#include "RuntimeConfigSelection.h"
#include "RuntimeCommandSink.h"
#include "Scene.h"
#include "VideoDirector.h"
#include "flames.h"
#include "TranslationOptions.h"
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

ErrorMessages errors;

static int interfaceRuntimeMilliseconds() {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    return (runtime != NULL) ? runtime->milliseconds() : 0;
}

ACTION(up) {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime != NULL)
        runtime->moveSelectionBy(-int(v));
}

ACTION(down) {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime != NULL)
        runtime->moveSelectionBy(int(v));
}

ACTION(home) {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime != NULL)
        runtime->selectHome();
}
ACTION(end) {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime != NULL)
        runtime->selectEnd();
}

ACTION(chgValue1) {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime != NULL)
        runtime->changeContextValueByElementIncrement(1, v);
}
ACTION(chgValue2) {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime != NULL)
        runtime->changeContextValueByElementIncrement(2, v);
}
ACTION(chgValue3) {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime != NULL)
        runtime->changeContextValueByElementIncrement(3, v);
}
ACTION(setValue) {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime != NULL)
        runtime->setContextValueFromElement(v);
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
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime != NULL)
        runtime->selectNextInList(InterfaceList);
}
ACTION(prevInterface) {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime != NULL)
        runtime->selectPreviousInList(InterfaceList);
}

// default display handler
void Interface::display() {
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
        line = displayDevice->print(elements[i]->text(sel == i), line, 'l',
            (sel == i) ? TEXT_COLOR_HIGHLIGHT : TEXT_COLOR_NORMAL);
    }

    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    int showStatus = runtime != NULL ? runtime->showStatus() : 0;
    if (showStatus) {
        static char str[512];
        const AutoChangerStatusProvider* provider = runtime != NULL
            ? runtime->autoChangerStatusProvider()
            : NULL;
        const char* autoChangeStatus = provider != NULL
            ? provider->autoChangerStatus()
            : "";
        snprintf(str, sizeof(str), "%s%s", (cthughaDisplay != NULL) ? cthughaDisplay->status() : "",
            autoChangeStatus);

        displayDevice->print(str, text_size.y - 1, 'l', TEXT_COLOR_NORMAL, 1);
    }

    if ((runtime != NULL) && runtime->saveToPreset()) {
        displayDevice->print("save to preset slot (press 0..9)", text_size.y - (showStatus ? 2 : 1), 'l',
            TEXT_COLOR_NORMAL);
    }

}

void Interface::doKey(int key) {

    if (Keymap::action(name, key) == 0)
        return;
    Keymap::action("default", key);
}

// default runner
void Interface::run(InterfaceRuntime& /* runtime */) {

    this->preRun();

    // handle keys
    int key;
    while ((key = getkey()) != CK_NONE) {

        if ((sel >= 0) && (sel < nElements))
            if (elements[sel]->doKey(key) == 0)
                continue;

        this->doKey(key);
    }
}

//
// class InterfaceElementOption
//

Keymap InterfaceElementOption::keymap("OptionElement");

InterfaceElementOption::InterfaceElementOption(const char* t, Option* o, int i1, int i2, int i3)
    : InterfaceElement(t)
    , opt(o)
    , inc1(i1)
    , inc2(i2)
    , inc3(i3) { }

const char* InterfaceElementOption::text(int selected) {
    static char strRet[512];
    char fmt[512];
    char in[512];

    snprintf(fmt, sizeof(fmt), "%%c%%-%ds%%c", min(text_size.x - 3, 77));
    snprintf(in, sizeof(in), str, opt->text());

    // make format and include the > <
    snprintf(strRet, sizeof(strRet), fmt, selected ? '>' : ' ', in, selected ? '<' : ' ');

    return strRet;
}

int InterfaceElementOption::doKey(int key) {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime == NULL)
        return 1;

    return runtime->runOptionKey(*opt, *this, keymap, key);
}

//
// Effect Control Element
//
Keymap InterfaceElementEffectControl::effectControlKeymap("EffectControlElement");

InterfaceElementEffectControl::InterfaceElementEffectControl(
    const char* t, EffectControl* o, int i1, int i2, int i3)
    : InterfaceElementOption(t, o, i1, i2, i3)
    , effectControl(o) { }

int InterfaceElementEffectControl::doKey(int key) {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime == NULL)
        return 1;

    return runtime->runEffectControlKey(*effectControl, *this,
        effectControlKeymap, keymap, key);
}

ACTION(lockElement) {
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    if (runtime != NULL)
        runtime->toggleContextEffectControlLock();
}

static const char* runtimeConfigSelectionTextForInterface(
    RuntimeConfigSelectionField field, Option* fallback) {
    static std::string text;
    InterfaceRuntime* runtime = Keymap::interfaceRuntime();
    const RuntimeConfigRegistry* registry = runtime != NULL
        ? runtime->runtimeConfigRegistry()
        : NULL;
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

    virtual const char* text(int selected) {
        static char strRet[512];
        char fmt[512];
        char in[512];

        snprintf(fmt, sizeof(fmt), "%%c%%-%ds%%c", min(text_size.x - 3, 77));
        snprintf(in, sizeof(in), str,
            runtimeConfigSelectionTextForInterface(field, opt));
        snprintf(strRet, sizeof(strRet), fmt, selected ? '>' : ' ', in, selected ? '<' : ' ');

        return strRet;
    }
};

class InterfaceElementAudioProcessingOption
    : public InterfaceElementRuntimeConfigOption {
    void updateOption() {
        InterfaceRuntime* runtime = Keymap::interfaceRuntime();
        AudioProcessingSelector* selector = runtime != NULL
            ? runtime->audioProcessingSelector()
            : NULL;
        opt = (selector != NULL)
            ? static_cast<Option*>(&selector->option())
            : static_cast<Option*>(&optionDummy);
    }

public:
    InterfaceElementAudioProcessingOption(const char* t,
        RuntimeConfigSelectionField field_)
        : InterfaceElementRuntimeConfigOption(t, &optionDummy, field_) { }

    virtual const char* text(int selected) {
        updateOption();
        return InterfaceElementRuntimeConfigOption::text(selected);
    }

    virtual int doKey(int key) {
        updateOption();
        return InterfaceElementOption::doKey(key);
    }
};

class InterfaceElementAutoChangeOption : public InterfaceElementOption {
    AutoChangeControlField field;

    void updateOption() {
        InterfaceRuntime* runtime = Keymap::interfaceRuntime();
        AutoChangeControls* controls = runtime != NULL
            ? runtime->autoChangeControls()
            : NULL;
        opt = (controls != NULL) ? &controls->option(field) : &optionDummy;
    }

public:
    InterfaceElementAutoChangeOption(const char* t,
        AutoChangeControlField field_, int i1 = 1, int i2 = 10, int i3 = 100)
        : InterfaceElementOption(t, &optionDummy, i1, i2, i3)
        , field(field_) { }

    virtual const char* text(int selected) {
        updateOption();
        return InterfaceElementOption::text(selected);
    }

    virtual int doKey(int key) {
        updateOption();
        return InterfaceElementOption::doKey(key);
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

    virtual const char* text(int selected) {
        static char strRet[512];
        char fmt[512];
        char in[512];

        snprintf(fmt, sizeof(fmt), "%%c%%-%ds%%c", min(text_size.x - 3, 77));
        snprintf(in, sizeof(in), str,
            runtimeConfigSelectionTextForInterface(field, effectControl));
        snprintf(strRet, sizeof(strRet), fmt, selected ? '>' : ' ', in,
            selected ? '<' : ' ');

        return strRet;
    }
};

void ErrorMessages::addMessage(const char* text) {
    if (nMsgs == 128) {
        CTH_ERROR("too many errors: %s\n", text);
        return;
    }
    strncpy(msgs[nMsgs], text, 128);
    on_screen[nMsgs] = interfaceRuntimeMilliseconds();

    nMsgs++;
}
void ErrorMessages::display() {

    // bring messages to screen
    for (int i = 0; i < nMsgs; i++) {
        displayDevice->print(msgs[i], -(nMsgs - i), 'r', TEXT_COLOR_ERROR);
    }

    // remove old messages
    const int errorTime = 3000;
    const int currentTime = interfaceRuntimeMilliseconds();
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
    char extraKeymap[512];

public:
    InterfaceMain()
        : Interface("main", NULL, NULL) {
        extraKeymap[0] = '\0';
    }

    void doKey(int key) {
        if (extraKeymap[0] != '\0') {
            if (Keymap::action(extraKeymap, key) == 0)
                return;
        }
        Interface::doKey(key);
    }

    friend class setExtraKeymapAction;
} interfaceMain;

ACTION(setExtraKeymap) { strncpy(interfaceMain.extraKeymap, p, 512); }

Interface interfaceMixer("Mixer", "Mixer", NULL);

class InterfaceEffectControl : public Interface {
public:
    InterfaceEffectControl()
        : Interface("EffectControls", "Effect Controls", NULL) {

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
                    &videoDirector().imageOption(),
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
        O(10) = &videoDirector().imageOption();
        O(11) = &object;
        O(12) = &flashlight;
    }
    void doKey(int key) {
        if (Keymap::action("EffectControls", key) == 0)
            return;
        if (Keymap::action("Options", key) == 0)
            return;
        Keymap::action("default", key);
    }

} interfaceEffectControl;

InterfaceElement* elementsOption[] = {
    new InterfaceElementOption("Maximal Frames/second    : %10s", &maxFramesPerSecond),
    new InterfaceElementOption("Zoom (0=max)             : %10s", &zoom),
    new InterfaceElementAutoChangeOption("Minimal time btw. change : %10s",
        AutoChangeControlWaitMinMs, 100, 500, 1000),
    new InterfaceElementAutoChangeOption("Extra random time        : %10s",
        AutoChangeControlWaitRandomMs, 100, 500, 1000),
    new InterfaceElementAutoChangeOption("Quiet change time        : %10s",
        AutoChangeControlQuietMs, 100, 500, 1000),
    new InterfaceElementOption("Time before silence Msg. : %10s", &changeMsgTime, 100, 500, 1000),
    new InterfaceElementAutoChangeOption("Cumulative fire level    : %10s",
        AutoChangeControlCumulativeFireLevel, 10, 50, 100),
    new InterfaceElementAutoChangeOption("Little changes only      : %10s",
        AutoChangeControlChangeLittle),
    new InterfaceElementAutoChangeOption("Lock                     : %10s",
        AutoChangeControlLocked),
};
int nElementsOption = sizeof(elementsOption) / sizeof(InterfaceElement*);

Interface interfaceOption("Options", "Options", NULL, elementsOption, nElementsOption);

void registerListInterfaces(InterfaceRuntime& runtime);
void registerHelpInterface(InterfaceRuntime& runtime);
void registerCreditsInterface(InterfaceRuntime& runtime);
void registerAudioInterfaces(InterfaceRuntime& runtime);

void registerDefaultInterfaces(InterfaceRuntime& runtime) {
    runtime.registerInterface(interfaceMain);
    runtime.registerInterface(interfaceMixer);
    runtime.registerInterface(interfaceEffectControl);
    runtime.registerInterface(interfaceOption);
    registerAudioInterfaces(runtime);
    registerListInterfaces(runtime);
    registerHelpInterface(runtime);
    registerCreditsInterface(runtime);
}
