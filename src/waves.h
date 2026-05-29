/*
 *  sound-display (wave-functions)
 */
#ifndef __WAVES_H
#define __WAVES_H

#include "cthugha.h"
#include "CoreOption.h"
#include "Wave.h"

class CthughaBuffer;

class WaveEntry : public CoreOptionEntry {
    Wave* waveValue;

public:
    WaveEntry(Wave& wave, int inUse = 1);

    Wave& wave() const;
};

class WaveOption : public CoreOption {
public:
    WaveOption();

    Wave* currentWave();
};

extern WaveOption wave;
extern CoreOption waveScale;
extern CoreOption table;
extern CoreOption object;

int init_tables();
int init_wave();
WObject* currentWaveObject();

typedef unsigned char pal_table[256]; /* Table for display_wave */
extern pal_table tables[]; /* Palette-Tables */
extern int nr_tables; /* number of tables */

extern OptionOnOff use_objects; /* use 3-D objects */

#endif
