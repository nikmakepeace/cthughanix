/** @file
 * Unit coverage for owned InterfaceRuntime state and scoped command context.
 */

#include "cthugha.h"
#include "InterfaceRuntime.h"
#include "Interface.h"
#include "InputQueue.h"
#include "Option.h"
#include "ProcessServices.h"
#include "RuntimeCommandSink.h"
#include "RuntimeCommandTargets.h"
#include "keymap.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>

int cth_log_enabled(int) { return 0; }
int cth_log(int, const char*, ...) { return 0; }
int cth_log_context(int, const char*, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }
int cth_log_errno(int, const char*, ...) { return 0; }

Option::~Option() { }

class SimpleOption : public Option {
public:
    SimpleOption()
        : Option("simple") { }

    virtual void change(int) { }
    virtual void change(const char*) { }
    virtual const char* text() const { return "simple"; }
};

class FakeMillisecondClock : public MillisecondClock {
public:
    int value;

    explicit FakeMillisecondClock(int value_)
        : value(value_) { }

    virtual int milliseconds() const { return value; }
};

Interface::Interface(const char* n, const char* ti, const char* te)
    : name(n)
    , title(ti)
    , text(te)
    , elements(NULL)
    , nElements(0)
    , sel(-1) { }

Interface::Interface(const char* n, const char* ti, const char* te,
    InterfaceElement** el, int nEl)
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
void Interface::display(InterfaceRuntime&) { }
void Interface::doKey(InterfaceRuntime&, KeymapRegistry&, int) { }
void Interface::run(InterfaceRuntime& runtime, InputQueue&,
    KeymapRegistry&) {
    (void)runtime;
    preRun();
}

InterfaceElementOption::InterfaceElementOption(const char* t, Option* o,
    int i1, int i2, int i3)
    : InterfaceElement(t)
    , opt(o)
    , inc1(i1)
    , inc2(i2)
    , inc3(i3) { }
const char* InterfaceElementOption::text(InterfaceRuntime&, int) { return str; }
int InterfaceElementOption::doKey(InterfaceRuntime&, KeymapRegistry&, int) {
    return 1;
}

KeymapRegistry::KeymapRegistry()
    : firstValue(NULL) { }
KeymapRegistry::~KeymapRegistry() { }
int KeymapRegistry::action(const char*, int, InterfaceRuntime& runtime) {
    runtime.changeContextValueByElementIncrement(1, 1.0);
    return 0;
}

class CapturingTargetRouter : public RuntimeCommandTargetRouter {
public:
    int optionByCalls;
    Option* lastOption;
    int lastValue;

    CapturingTargetRouter()
        : optionByCalls(0)
        , lastOption(NULL)
        , lastValue(0) { }

    virtual RuntimeChangeSet changeEffectControlBy(
        EffectControl&, int) {
        return RuntimeChangeSet();
    }

    virtual RuntimeChangeSet changeEffectControlTo(
        EffectControl&, const char*) {
        return RuntimeChangeSet();
    }

    virtual RuntimeChangeSet activateEffectControl(
        EffectControl&, int) {
        return RuntimeChangeSet();
    }

    virtual RuntimeChangeSet toggleEffectChoiceUse(
        EffectControl&, int) {
        return RuntimeChangeSet();
    }

    virtual RuntimeChangeSet changeOptionBy(Option& option, int by) {
        optionByCalls++;
        lastOption = &option;
        lastValue = by;
        return RuntimeChangeSet();
    }

    virtual RuntimeChangeSet changeOptionTo(Option&, const char*) {
        return RuntimeChangeSet();
    }

    virtual RuntimeChangeSet toggleEffectControlLock(EffectControl&) {
        return RuntimeChangeSet();
    }
};

static void testSelectionIsOwnedByRuntime() {
    FakeMillisecondClock clock(1000);
    InterfaceRuntime runtime(clock);
    Interface main("main", "Main", NULL);
    Interface other("other", "Other", NULL);

    main.nElements = 3;
    runtime.registerInterface(main);
    runtime.registerInterface(other);

    assert(runtime.find("main") == &main);
    assert(runtime.find("other") == &other);
    assert(runtime.find("missing") == NULL);

    runtime.registerOwnedInterface(new Interface("owned", "Owned", NULL));
    assert(runtime.find("owned") != NULL);

    runtime.set("main");
    assert(runtime.current() == &main);
    runtime.moveSelectionBy(10);
    assert(main.sel == 2);
    assert(other.sel == -1);

    runtime.set("other");
    assert(runtime.current() == &other);
    assert(other.sel == -1);
}

static void testStatusAndPresetFlagsAreOwnedByRuntime() {
    FakeMillisecondClock clock(1000);
    InterfaceRuntime runtime(clock);

    assert(runtime.showStatus() == 0);
    runtime.toggleStatus();
    assert(runtime.showStatus() == 1);

    assert(runtime.saveToPreset() == 0);
    runtime.toggleSaveToPreset();
    assert(runtime.saveToPreset() == 1);
    runtime.clearSaveToPreset();
    assert(runtime.saveToPreset() == 0);

    assert(strcmp(runtime.extraKeymap(), "") == 0);
    runtime.setExtraKeymap("temporary");
    assert(strcmp(runtime.extraKeymap(), "temporary") == 0);
    runtime.setExtraKeymap(NULL);
    assert(strcmp(runtime.extraKeymap(), "") == 0);
}

static void testHelpScrollStateIsOwnedByRuntime() {
    FakeMillisecondClock clock(1000);
    InterfaceRuntime runtime(clock);

    assert(runtime.helpScrolling() == 0);
    assert(runtime.helpScrollPosition() == 0.0);

    runtime.toggleHelpScrolling();
    assert(runtime.helpScrolling() == 1);
    runtime.advanceHelpScroll(2.5);
    assert(runtime.helpScrollPosition() == 2.5);
    assert(runtime.helpScrolling() == 1);

    runtime.scrollHelpBy(-3.0, 10);
    assert(runtime.helpScrollPosition() == 9.5);
    assert(runtime.helpScrolling() == 0);
}

static void testCreditsAnimationStateIsOwnedByRuntime() {
    FakeMillisecondClock clock(1000);
    InterfaceRuntime runtime(clock);

    assert(runtime.creditsPosition() == 0.0);
    assert(runtime.updateCreditsPosition(1000, 20) == -16.0);
    assert(runtime.creditsPosition() == -16.0);
    assert(runtime.updateCreditsPosition(1500, 20) == -14.0);
}

static void testOptionCommandContextIsScoped() {
    FakeMillisecondClock clock(1000);
    InterfaceRuntime runtime(clock);
    CapturingTargetRouter router;
    SimpleOption option;
    InterfaceElementOption element("value: %s", &option, 5, 10, 20);
    KeymapRegistry keymaps;

    runtime.setCommandRouter(&router);
    assert(runtime.runOptionKey(option, element, keymaps, "test", 'x') == 0);
    assert(router.optionByCalls == 1);
    assert(router.lastOption == &option);
    assert(router.lastValue == 5);

    runtime.changeContextValueByElementIncrement(1, 1.0);
    assert(router.optionByCalls == 1);

    runtime.setCommandRouter(NULL);
}

static void testMillisecondsComeFromInjectedClock() {
    FakeMillisecondClock clock(1234);
    InterfaceRuntime runtime(clock);

    assert(runtime.milliseconds() == 1234);
    clock.value = 5678;
    assert(runtime.milliseconds() == 5678);
}

int main() {
    testSelectionIsOwnedByRuntime();
    testStatusAndPresetFlagsAreOwnedByRuntime();
    testHelpScrollStateIsOwnedByRuntime();
    testCreditsAnimationStateIsOwnedByRuntime();
    testOptionCommandContextIsScoped();
    testMillisecondsComeFromInjectedClock();
    return 0;
}
