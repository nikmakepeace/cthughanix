#ifndef __PNG_H
#define __PNG_H

#include "cthugha.h"
#include "CoreOption.h"

CoreOptionEntry* read_png_image(FILE* file, const char* name, const char* dir,
    const char* totalName);

#endif
