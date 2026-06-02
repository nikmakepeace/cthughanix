// -*- C++ -*-
#ifndef __EFFECT_CONTROL_H
#define __EFFECT_CONTROL_H

#include "cthugha.h"
#include "Option.h"

#include <string>

//
// Remarks:
//
// * CthughaBuffer must be inidializes BEFORE any initial values
//   are set for the EffectControls
//

class EffectChoice {
protected:
    char* name; // name (short)
    char* desc; // description or long name
    OptionOnOff use; // in use or not
public:
    EffectChoice(const char* n, const char* d, int inUse = 1);
    virtual ~EffectChoice() {
        delete[] name;
        name = NULL;
        delete[] desc;
        desc = NULL;
    }
    virtual int operator()() { return 0; } // do nothing by default

    const char* Name() const { return name; }
    const char* Desc() const { return desc; }
    void setUse(int inUse) { use.setValue(inUse); }
    int inUse() const { return int(use); }
    const char* useText() const { return use.text(); }

    virtual int sameName(const char* other);

    friend class EffectControl;
    friend class InterfaceList;
    friend class activateAction;
};

enum EffectControlFlags {
    EFFECT_CONTROL_NO_FLAGS = 0,

    // This option participates in AutoChanger's random/all changes.  New
    // EffectControls should opt in deliberately so construction alone does not
    // make them runtime mutation targets.
    EFFECT_CONTROL_AUTO_CHANGE = 1 << 0
};

class OffEntry : public EffectChoice {
public:
    OffEntry(const char* name = "off")
        : EffectChoice(name, "", 1) { }
    virtual int sameName(const char* other);
};

class OnEntry : public EffectChoice {
public:
    OnEntry()
        : EffectChoice("on", "", 1) { }
    virtual int sameName(const char* other);
};

//
// A single linked list for selectable effect choices.
// implementation is not very efficient
//
class EffectChoiceList {
    EffectChoice* entry;
    EffectChoiceList* next;

public:
    EffectChoiceList()
        : entry(NULL)
        , next(NULL) { }
    EffectChoiceList(EffectChoice* e)
        : entry(e)
        , next(NULL) { }
    EffectChoiceList(EffectChoice** e, int n) {
        if (n > 0) {
            entry = e[0];
            next = (n > 1) ? new EffectChoiceList(e + 1, n - 1) : (EffectChoiceList*)NULL;
        } else {
            entry = NULL;
            next = NULL;
        }
    }
    int inList(EffectChoice* e) {
        return (entry == e) ? 1 : ((next == NULL) ? 0 : next->inList(e));
    }

    void add(EffectChoice* e) {
        if (inList(e))
            return;

        if (entry == NULL)
            entry = e;
        else if (next == NULL)
            next = new EffectChoiceList(e);
        else
            next->add(e);
    }

    EffectChoice* operator[](int n) {
        return (n == 0) ? entry : ((next != NULL) ? (*next)[n - 1] : (EffectChoice*)NULL);
    }
    EffectChoice* operator[](int n) const {
        return (n == 0) ? entry : ((next != NULL) ? (*next)[n - 1] : (EffectChoice*)NULL);
    }
    int n() const { return (next == NULL) ? ((entry == NULL) ? 0 : 1) : 1 + next->n(); }
};

class EffectControl : public Option {
protected:
    //
    // buffer this option belongs to
    //
    int buffer;

    //
    // List of all effect controls
    //
    static EffectControl* first;
    EffectControl* next;

    //
    // History and Hot Values
    //
    int* oldValues;
    int history;
    int* hot;

    //
    // Choices
    //
    EffectChoiceList& entries;
    int flags;

    std::string initialEntry;

    void doSave();
    void doRestore();

protected:
    int isAutoChangeCandidate() const;

public:
    EffectControl(int buffer, const char* name, EffectChoiceList& e,
        int flags = EFFECT_CONTROL_NO_FLAGS);

    EffectControl& operator=(const EffectControl& other);

    void setInitialEntry(const char* i) { initialEntry = (i != NULL) ? i : ""; }

    virtual const char* name() const;

    //
    // Changeing
    //
    OptionOnOff lock; // individual lock

    static void changeToInitial();
    virtual void change(const char* to, int doSave = 1);
    virtual void change(int by, int doSave = 1);
    virtual void changeRandom(int save_ = 1);

    int optNr(const char* n);

    void change(int) { CTH_ERROR("internal error. wrong change called for option `%s'.\n", name()); }
    void change(const char*) {
        CTH_ERROR("internal error. wrong change called for option `%s'.\n", name());
    }

    static EffectControl* changeOne();
    static void changeAll();

    virtual const char* text() const;

    //
    // Choices
    //
    virtual const char* text(int i) const { // get name of entry i
        if ((i < 0) || (i >= getNEntries()))
            return "unknown";
        return entries[i]->name;
    }
    void add(EffectChoice*); // add a new choice
    void add(EffectChoice**, int nEntries); // add several new choices
    int defined(const char* name); // check if a choice is already defined

    virtual int operator()() { // do the action of the current entry
        if ((value < 0) || (value >= getNEntries()))
            return 0;
        return (entries[value]->operator()());
    }

    EffectChoice* operator[](int i) { // get the i-th entry
        return entries[i];
    }
    EffectChoice* current() { // get the current entry
        return operator[](value);
    }
    int currentN() const { // get current value
        return value;
    }
    const char* currentName() const { // return current name
        if ((value < 0) || (value >= getNEntries()))
            return "unknown";
        return entries[value]->name;
    }
    const char* currentDesc() const { // return current description
        if ((value < 0) || (value >= getNEntries()))
            return "";
        return entries[value]->desc;
    }
    int getNEntries() const { return entries.n(); }
    int autoChangeEnabled() const { return (flags & EFFECT_CONTROL_AUTO_CHANGE) != 0; }
    int bufferIndex() const { return buffer; }
    EffectControl* nextRegistered() const { return next; }

    //
    // control History and Hot Values
    //

    static void save();
    static void restore();

    static void save(int to);
    static void restore(int from);
    void setHotValue(int slot, int value_);
    int hotValue(int slot) const;
    static int hotSlotCount();
    static EffectControl* firstRegistered();

    friend class InterfaceList;
    friend class activateAction;
};

extern EffectControl screen;

#endif
