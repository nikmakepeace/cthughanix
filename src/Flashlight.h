// Visual flashlight option.

#ifndef __FLASHLIGHT_H
#define __FLASHLIGHT_H

#include "EffectControl.h"
#include "FlashlightRenderer.h"

/** Global on/off option for palette flashlight. */
extern EffectControl flashlight;

/**
 * Registers flashlight option entries.
 */
void init_flashlight();

#endif
