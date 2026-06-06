// -*- C++ -*-

#ifndef __KEYMAP_H
#define __KEYMAP_H

#include "cthugha.h"

struct InputConfig;
class InterfaceRuntime;

class Action {
    const char* name;
    int nameLen;

    Action* next;

public:
    Action(const char* n)
        : name(n)
        , nameLen(strlen(n))
        , next(NULL) {
    }
    virtual ~Action() { }
    virtual void act(const char* /* param */, double /* value */,
        InterfaceRuntime& /* runtime */) { };

    int operator==(const char* n) const { return (strncasecmp(name, n, nameLen) == 0); }
    int length() const { return nameLen; }

    friend class CommandRegistry;
    friend class Keymap;
};

#define ACTION(a)                                                                                  \
    class a##Action : public Action {                                                              \
    public:                                                                                        \
        a##Action()                                                                                \
            : Action(#a) { }                                                                       \
        virtual void act(const char* param, double value, InterfaceRuntime& runtime);              \
    };                                                                                             \
    void a##Action::act(const char* p, double v, InterfaceRuntime& runtime)

class CommandRegistry {
    Action* firstValue;

public:
    CommandRegistry();
    ~CommandRegistry();

    void registerAction(Action* action);
    Action* findLongestPrefix(const char* text) const;
};

class Keymap {
protected:
    Keymap* next;

    char* name;
    struct Binding {
        int key;
        struct ActionList {
            Action* action;
            char* param;
            ActionList* next;

            ActionList(Action* a, char* p, ActionList* n)
                : action(a)
                , param(p)
                , next(n) { }
        }* actionList;

        Binding()
            : key(0)
            , actionList(NULL) { }
    };
    struct BindingList : Binding {
        BindingList* next;

        BindingList(Binding& b, BindingList* n)
            : Binding(b)
            , next(n) { }
    }* bindingList;

    void add(Binding& b);
    Binding parseBinding(const char* line, CommandRegistry& commands);

public:
    Keymap(const char* name);
    ~Keymap();

    void add(const char* line, CommandRegistry& commands);

    int action(int key, InterfaceRuntime& runtime);

    friend class KeymapRegistry;
};

class KeymapRegistry {
    Keymap* firstValue;

    Keymap* find(const char* name, int create = 0);

public:
    KeymapRegistry();
    ~KeymapRegistry();

    void readFile(const char* fileName, CommandRegistry& commands);
    void addList(CommandRegistry& commands, int dummy, ...);

    int action(const char* name, int key, InterfaceRuntime& runtime);

    void init(const InputConfig& config, CommandRegistry& commands);

};

void registerDefaultKeyActions(CommandRegistry& registry);

#endif
