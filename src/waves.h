/*
 *  sound-display (wave-functions)
 */
#ifndef __WAVES_H
#define __WAVES_H

#include "cthugha.h"
#include "CoreOption.h"
#include "Wave.h"

class CthughaBuffer;

/**
 * CoreOption entry that points at one built-in Wave catalog item.
 */
class WaveEntry : public CoreOptionEntry {
    Wave* waveValue;

public:
    /**
     * Creates an option entry for a wave.
     *
     * @param wave Borrowed wave catalog entry.
     * @param inUse Nonzero when the option should be selectable by default.
     */
    WaveEntry(Wave& wave, int inUse = 1);

    /** @return Borrowed wave catalog entry. */
    Wave& wave() const;
};

/**
 * Option wrapper for selecting the current wave renderer.
 */
class WaveOption : public CoreOption {
public:
    WaveOption();

    /** @return Currently selected wave, or NULL. */
    Wave* currentWave();
};

/** Global wave renderer selection option. */
extern WaveOption wave;

/** Wave-scale option; interpretation is wave-specific. */
extern CoreOption waveScale;

/** Palette color-table option for wave drawing. */
extern CoreOption table;

/** 3D object selection option for object-capable waves. */
extern CoreOption object;

/** Initializes palette color tables used by wave drawing. */
int init_tables();

/**
 * Initializes wave options, color tables, and optional 3D object files.
 *
 * @return Zero on completion.
 */
int init_wave();

/** @return Currently selected 3D object line list, or NULL. */
WObject* currentWaveObject();

/** Palette remap table used by wave renderers: input intensity -> palette index. */
typedef unsigned char pal_table[256];

/** Built-in palette color tables for wave drawing. */
extern pal_table tables[];

/** Number of entries in tables. */
extern int nr_tables;

/** Enables loading/selecting 3D object files for object-capable waves. */
extern OptionOnOff use_objects;

#endif
