// -*- C++ -*-

#ifndef __INTERFACE_H
#define __INTERFACE_H

#include "cthugha.h"
#include "EffectControl.h"
#include "keymap.h"

class AutoChangerStatusProvider;
class AutoChangeControls;
class AudioProcessingSelector;
class InterfaceRuntime;
class RuntimeConfigRegistry;

class InterfaceElement {
protected:
    const char* str;

public:
    InterfaceElement(const char* t)
        : str(t) { }
    virtual ~InterfaceElement() { }

    virtual const char* text(int /* selected */) { return str; }
    virtual int doKey(int /* key */) { return 1; }
};

class Interface {
public:
    const char* name;
    const char* title;
    const char* text;

    InterfaceElement** elements;
    int nElements;
    int sel;

    Interface(const char* n, const char* ti, const char* te);
    Interface(const char* n, const char* ti, const char* te, InterfaceElement* el[], int nEl);

    virtual ~Interface();

    void setElements(InterfaceElement** el, int nEl);

    virtual void preRun() { }
    virtual void display();
    virtual void doKey(int key);

    /**
     * Services the active interface once.
     *
     * Application runs the interface before and after frame generation so input
     * handling and overlay/status updates can bracket visual work.
     *
     * @param runtime Runtime state owning the current interface selection and
     *        adapter pointers.
     */
    virtual void run(InterfaceRuntime& runtime);
};

class InterfaceElementOption : public InterfaceElement {
public:
    Option* opt;
    int inc1;
    int inc2;
    int inc3;

    static Keymap keymap;

    InterfaceElementOption(const char* t, Option* o, int i1 = 1, int i2 = 10, int i3 = 100);

    virtual const char* text(int selected);
    virtual int doKey(int key);
};

class InterfaceElementEffectControl : public InterfaceElementOption {
public:
    EffectControl* effectControl;

    static Keymap effectControlKeymap;

    InterfaceElementEffectControl(const char* t, EffectControl* o, int i1 = 1, int i2 = 10, int i3 = 100);

    virtual int doKey(int key);
};

class ErrorMessages {
    char msgs[128][128];
    int on_screen[128];
    int nMsgs;

public:
    ErrorMessages()
        : nMsgs(0) { }

    void addMessage(const char* text);
    void display();
};

extern Interface interfaceMixer;
extern ErrorMessages errors;

/**
 * Registers the existing concrete panel definitions into an owned runtime.
 *
 * @param runtime Interface runtime that will own selection/adapter state.
 */
void registerDefaultInterfaces(InterfaceRuntime& runtime);

#endif
