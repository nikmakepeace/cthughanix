// Ini-file adapter for EffectControl values, usage flags, and hot slots.

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

/** Reads EffectControl hot-slot selections from the currently open ini source. */
void effectControlGetHotIni();

/** Writes EffectControl hot-slot selections to the currently open ini output. */
void effectControlPutHotIni();

/**
 * Checks whether an ini key belongs to any registered EffectControl.
 *
 * @param entry Ini key without the "cthugha." prefix.
 * @return Nonzero when the key is an EffectControl initial value, usage flag, hot
 *         slot, or legacy wildcard key.
 */
int effectControlIsIniEntry(const char* entry);

#endif
