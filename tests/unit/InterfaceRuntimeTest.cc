/** @file
 * Unit coverage for owned InterfaceRuntime state and scoped command context.
 */

#include "cthugha.h"
#include "InterfaceRuntime.h"
#include "Interface.h"
#include "Option.h"
#include "RuntimeCommandSink.h"

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
void Interface::display() { }
void Interface::doKey(int) { }
void Interface::run(InterfaceRuntime& runtime) {
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
const char* InterfaceElementOption::text(int) { return str; }
int InterfaceElementOption::doKey(int) { return 1; }

Keymap* Keymap::first = NULL;
Keymap* Keymap::current = NULL;

static RuntimeCommandSink* runtimeCommandSinkValue = NULL;
static InterfaceRuntime* interfaceRuntimeValue = NULL;
static InterfaceRuntime* actionRuntimeValue = NULL;

Keymap::Keymap(const char* n)
    : next(NULL)
    , name(n)
    , bindingList(NULL) { }

int Keymap::action(int) {
    if (actionRuntimeValue != NULL)
        actionRuntimeValue->changeContextValueByElementIncrement(1, 1.0);
    return 0;
}

int Keymap::action(const char*, int) {
    return 1;
}

void Keymap::setRuntimeCommandSink(RuntimeCommandSink* sink) {
    runtimeCommandSinkValue = sink;
}

RuntimeCommandSink* Keymap::runtimeCommandSink() {
    return runtimeCommandSinkValue;
}

void Keymap::setInterfaceRuntime(InterfaceRuntime* runtime) {
    interfaceRuntimeValue = runtime;
}

InterfaceRuntime* Keymap::interfaceRuntime() {
    return interfaceRuntimeValue;
}

class CapturingSink : public RuntimeCommandSink {
public:
    int count;
    RuntimeCommand command;

    CapturingSink()
        : count(0)
        , command(RuntimeCommand::requestClose()) { }

    virtual RuntimeChangeSet apply(const RuntimeCommand& command_) {
        count++;
        command = command_;
        return RuntimeChangeSet();
    }
};

static void testSelectionIsOwnedByRuntime() {
    InterfaceRuntime runtime;
    Interface main("main", "Main", NULL);
    Interface other("other", "Other", NULL);

    main.nElements = 3;
    runtime.registerInterface(main);
    runtime.registerInterface(other);

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
    InterfaceRuntime runtime;

    assert(runtime.showStatus() == 0);
    runtime.toggleStatus();
    assert(runtime.showStatus() == 1);

    assert(runtime.saveToPreset() == 0);
    runtime.toggleSaveToPreset();
    assert(runtime.saveToPreset() == 1);
    runtime.clearSaveToPreset();
    assert(runtime.saveToPreset() == 0);
}

static void testOptionCommandContextIsScoped() {
    InterfaceRuntime runtime;
    CapturingSink sink;
    SimpleOption option;
    InterfaceElementOption element("value: %s", &option, 5, 10, 20);
    Keymap keymap("test");

    Keymap::setRuntimeCommandSink(&sink);
    actionRuntimeValue = &runtime;

    assert(runtime.runOptionKey(option, element, keymap, 'x') == 0);
    assert(sink.count == 1);
    assert(sink.command.type == RuntimeCommandChangeOptionBy);
    assert(sink.command.option == &option);
    assert(sink.command.value == 5);

    runtime.changeContextValueByElementIncrement(1, 1.0);
    assert(sink.count == 1);

    actionRuntimeValue = NULL;
    Keymap::setRuntimeCommandSink(NULL);
}

int main() {
    testSelectionIsOwnedByRuntime();
    testStatusAndPresetFlagsAreOwnedByRuntime();
    testOptionCommandContextIsScoped();
    return 0;
}
