#include "cthugha.h"
#include "Interface.h"
#include "InterfaceRuntime.h"
#include "display.h"
#include "imath.h"
#include "Border.h"
#include "DisplayDevice.h"
#include "CthughaBuffer.h"
#include "Flashlight.h"
#include "RuntimeCommandSink.h"
#include "Scene.h"
#include "Image.h"
#include "flames.h"
#include "keymap.h"
#include "TranslationOptions.h"
#include "waves.h"

//
// ERROR:
//
// When the OptionList changes this Interface is not updated!
// this happends when new random palettes are generated
//

class InterfaceList : public Interface {
    enum { size = 6 };
    int pos;
    EffectControl* effectControl;

public:
    InterfaceList(const char* name, const char* title, EffectControl* o)
        : Interface(name, title, NULL)
        , pos(0)
        , effectControl(o) { }

    virtual void display(InterfaceRuntime& runtime) {

        Interface::display(runtime);

        int n = effectControl->getNEntries();
        if (n == 0)
            return;

        // make sure selected line is on screen
        if (sel < pos) {
            pos = max(sel, 0);
        }
        if ((sel - size) >= pos) {
            pos = sel;
        }
        // display size lines
        for (int i = 0; i < size; i++) {
            int s = i + pos;
            if ((s >= n) || (s < 0))
                continue;

            char str1[128];
            snprintf(str1, sizeof(str1), "%c%s", (s == sel) ? '>' : ' ', effectControl->entries[s]->name);

            char str2[128];
            snprintf(str2, sizeof(str2), "%s %3s%c", effectControl->entries[s]->desc, effectControl->entries[s]->use.text(),
                (s == sel) ? '<' : ' ');

            char str3[128];
            snprintf(str3, sizeof(str3), "%40s", str2);

            displayDevice->print(
                str3, (i + 2), 'l', (s == sel) ? TEXT_COLOR_HIGHLIGHT : TEXT_COLOR_NORMAL);
            displayDevice->print(
                str1, (i + 2), 'l', (s == sel) ? TEXT_COLOR_HIGHLIGHT : TEXT_COLOR_NORMAL);
        }
    }

    virtual void doKey(InterfaceRuntime& runtime, KeymapRegistry& keymaps,
        CommandRegistry& commands, CommandDispatcher& dispatcher,
        CommandContext& context, int key) {
        int ret = key;
        int n = effectControl->getNEntries();

        nElements = n;

        if ((sel < n) && (sel >= 0)) {
            ret = runtime.runEffectChoiceKey(*effectControl,
                effectControl->entries[sel]->use, keymaps, commands,
                dispatcher, context, sel, key);
        }

        if (ret)
            ret = dispatcher.dispatchKeymap(keymaps, commands, name, key,
                context);
        if (ret)
            ret = dispatcher.dispatchKeymap(keymaps, commands, "list", key,
                context);
        if (ret)
            ret = dispatcher.dispatchKeymap(keymaps, commands, "default", key,
                context);

        nElements = 0;
    }
};

ACTION(toggleUse) {
    context.toggleEffectChoiceUse();
}

ACTION(activate) {
    context.activateEffectChoice();
}

void registerListKeyActions(CommandRegistry& registry) {
#define REGISTER_ACTION(a) registry.registerAction(new a##Action())
    REGISTER_ACTION(toggleUse);
    REGISTER_ACTION(activate);
#undef REGISTER_ACTION
}

#if 0
int InterfaceList::do_key(int key) {
    switch(key) {
    case CK_ENTER:
	if(sel >= nFixed) {
	    optList->entries[sel-nFixed]->use.setValue(1);	// make it usable
	    optList->setValue(sel - nFixed);			// make it active
	    optList->change(0, 0);				// validate and action
	    set(interfaces[0]);					// and close this display
	    return 0;
	} else
	    return key;
    case ' ':
	if(sel >= nFixed) {
	    optList->entries[sel-nFixed]->use.setValue(1);	// make it usable
	    optList->setValue(sel - nFixed);			// make it active
	    optList->change(0, 0);				// validate and action
	    return 0;
	} else
	    return key;
    }
    return Interface::do_key(key);
}
#endif

void registerListInterfaces(InterfaceRuntime& runtime, ImageOption& images) {
    runtime.registerOwnedInterface(
        new InterfaceList("Display", "Select Display", &screen));
    runtime.registerOwnedInterface(
        new InterfaceList("Flame", "Select Flame", &flame));
    runtime.registerOwnedInterface(
        new InterfaceList("Border", "Select Border of Buffer", &border));
    runtime.registerOwnedInterface(
        new InterfaceList("Translate", "Select Translation Table", &translation));
    runtime.registerOwnedInterface(
        new InterfaceList("Wave", "Select Wave", &wave));
    runtime.registerOwnedInterface(
        new InterfaceList("Table", "Select Sound Table", &table));
    runtime.registerOwnedInterface(
        new InterfaceList("WaveScaling", "Select Wave Scaling", &waveScale));
    runtime.registerOwnedInterface(
        new InterfaceList("Object", "Select 3D Object (for some waves)", &object));
    runtime.registerOwnedInterface(
        new InterfaceList("Palette", "Select Palette", &palette));
    runtime.registerOwnedInterface(
        new InterfaceList("Image", "Select Image", &images));
    runtime.registerOwnedInterface(
        new InterfaceList("Flashlight", "Select Flashlight", &flashlight));
}
