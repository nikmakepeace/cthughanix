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

extern CoreOptionEntry* _flames[];
extern int _nFlames;

extern CoreOptionEntryList generalFlameEntries;

class OptionGeneralFlame : public CoreOption {
public:
    OptionGeneralFlame(int buffer)
        : CoreOption(buffer, "flame-general", generalFlameEntries) { }

    const char* text() const;
};

#endif
