#ifndef __COLOR_PALETTE_H
#define __COLOR_PALETTE_H

#include "cthugha.h"

typedef unsigned char Palette[256][3]; /* one Palette: 256 entries, each 3 bytes */

class ColorPalette {
    Palette paletteValue;

public:
    ColorPalette();
    explicit ColorPalette(const Palette& palette);

    const Palette& raw() const;
    Palette& raw();

    void clear();
    void copyFrom(const Palette& palette);
    void copyFrom(const ColorPalette& palette);
    void copyTo(Palette& palette) const;

    int equals(const Palette& palette) const;
    int equals(const ColorPalette& palette) const;

    unsigned char component(int index, int channel) const;
    void setComponent(int index, int channel, int value);
    void setColor(int index, int red, int green, int blue);
};

#endif
