// Ini-file adapter for EffectControl values, usage flags, and preset slots.

#ifndef __EFFECT_CONTROL_INI_H
#define __EFFECT_CONTROL_INI_H

class EffectControl;

/** Reads initial EffectControl selections from the currently open ini source. */
void effectControlGetIniInitials();

/** Writes current EffectControl selections to the currently open ini output. */
void effectControlPutIniInitials();

/** Reads per-entry EffectControl usage flags from the currently open ini source. */
void effectControlGetIniUsages();

/** Writes per-entry EffectControl usage flags to the currently open ini output. */
void effectControlPutIniUsages();

/** Reads EffectControl preset-slot selections from the currently open ini source. */
void effectControlGetPresetIni();

/** Writes EffectControl preset-slot selections to the currently open ini output. */
void effectControlPutPresetIni();

/**
 * Checks whether an ini key belongs to any registered EffectControl.
 *
 * @param entry Ini key without the "cthugha." prefix.
 * @return Nonzero when the key is an EffectControl initial value, usage flag, preset
 *         slot, or legacy wildcard key.
 */
int effectControlIsIniEntry(const char* entry);

#endif
