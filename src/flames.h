#ifndef __FLAMES_H
#define __FLAMES_H

#include "CoreOption.h"

extern CoreOptionEntry * _flames[];	
extern int _nFlames;

int init_flames();

extern CoreOptionEntryList generalFlameEntries;	

class OptionGeneralFlame : public CoreOption {
public:
    OptionGeneralFlame(int buffer) : CoreOption(buffer, "flame-general", generalFlameEntries) {}

    const char * text() const;
};

#endif
