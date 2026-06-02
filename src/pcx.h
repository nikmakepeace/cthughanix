#ifndef __PCX_H
#define __PCX_H

#include "cthugha.h"
#include "EffectControl.h"
#include "ColorPalette.h"

struct ImageLoadTarget;

/**
 * Loads an 8-bit indexed PCX image into an ImageEntry.
 *
 * @param file Open file positioned at the beginning of PCX data.
 * @param name Option/display name for the image.
 * @param dir Directory containing the image, for diagnostics.
 * @param totalName Full image path, for diagnostics.
 * @param target Target buffer dimensions in pixels.
 * @return Newly allocated ImageEntry, or NULL on unsupported/invalid input.
 */
EffectChoice* read_pcx_image(FILE* file, const char* name, const char* dir,
    const char* totalName, const ImageLoadTarget& target);

/**
 * Saves indexed pixels as a PCX screenshot.
 *
 * @param buffer Source 8-bit indexed pixels.
 * @param width Image width in pixels.
 * @param height Image height in pixels.
 * @param pal 256-color RGB palette.
 * @return Zero on success, nonzero on write failure.
 */
int save_pcx(unsigned char* buffer, int width, int height, Palette pal);

#endif
