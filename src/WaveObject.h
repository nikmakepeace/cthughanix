#ifndef __WAVE_OBJECT_H
#define __WAVE_OBJECT_H

#include "EffectControl.h"
#include "Wave.h"

#include <stdio.h>

extern EffectChoice* _objects[];
extern int _nObjects;

EffectChoice* read_object(FILE* file, const char* name, const char* dir, const char* total_name);
WObject* waveObjectEntryObject(EffectChoice* entry);
int waveObjectEntryOwnsObject(const EffectChoice* entry);

#endif
