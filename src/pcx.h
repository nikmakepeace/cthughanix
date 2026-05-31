#ifndef __PCX_H
#define __PCX_H

#include "cthugha.h"
#include "CoreOption.h"
#include "ColorPalette.h"

struct ImageLoadTarget;

CoreOptionEntry* read_pcx_image(FILE* file, const char* name, const char* dir,
    const char* totalName, const ImageLoadTarget& target);

int save_pcx(unsigned char* buffer, int width, int height, Palette pal);

#endif
