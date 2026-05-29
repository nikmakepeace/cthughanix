#ifndef __FRAME_PALETTE_H
#define __FRAME_PALETTE_H

#include "ColorPalette.h"

class FramePalette {
    ColorPalette currentPaletteValue;
    int paletteDirtyValue;

public:
    FramePalette();

    const ColorPalette& currentPalette() const;
    int paletteDirty() const;
    void clearPaletteDirty();
    void setPalette(const ColorPalette& palette);
};

#endif
