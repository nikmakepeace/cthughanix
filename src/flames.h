#ifndef __FLAMES_H
#define __FLAMES_H

#include "cthugha.h"
#include "CoreOption.h"
#include "Flame.h"

class FlameEntry : public CoreOptionEntry {
    const Flame* flameValue;

public:
    FlameEntry(const Flame& flame, int inUse = 1);

    const Flame& flame() const;
};

class FlameOption : public CoreOption {
public:
    FlameOption();

    const Flame* currentFlame();
};

extern FlameOption flame;

class GeneralFlameOption : public CoreOption {
public:
    GeneralFlameOption();

    virtual void change(const char* to, int doSave = 1);
    virtual void change(int by, int doSave = 1);
    virtual void changeRandom(int doSave = 1);
    virtual const char* text() const;
};

extern GeneralFlameOption flameGeneral;

extern CoreOptionEntry* _flames[];
extern int _nFlames;

#endif
