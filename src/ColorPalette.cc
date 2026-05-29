#include "ColorPalette.h"

static unsigned char clampPaletteComponent(int value) {
    if (value < 0)
        return 0;
    if (value > 255)
        return 255;
    return (unsigned char)value;
}

ColorPalette::ColorPalette() {
    clear();
}

ColorPalette::ColorPalette(const Palette& palette) {
    copyFrom(palette);
}

const Palette& ColorPalette::raw() const {
    return paletteValue;
}

Palette& ColorPalette::raw() {
    return paletteValue;
}

void ColorPalette::clear() {
    memset(paletteValue, 0, sizeof(Palette));
}

void ColorPalette::copyFrom(const Palette& palette) {
    memcpy(paletteValue, palette, sizeof(Palette));
}

void ColorPalette::copyFrom(const ColorPalette& palette) {
    copyFrom(palette.raw());
}

void ColorPalette::copyTo(Palette& palette) const {
    memcpy(palette, paletteValue, sizeof(Palette));
}

int ColorPalette::equals(const Palette& palette) const {
    return memcmp(paletteValue, palette, sizeof(Palette)) == 0;
}

int ColorPalette::equals(const ColorPalette& palette) const {
    return equals(palette.raw());
}

unsigned char ColorPalette::component(int index, int channel) const {
    return paletteValue[index][channel];
}

void ColorPalette::setComponent(int index, int channel, int value) {
    paletteValue[index][channel] = clampPaletteComponent(value);
}

void ColorPalette::setColor(int index, int red, int green, int blue) {
    setComponent(index, 0, red);
    setComponent(index, 1, green);
    setComponent(index, 2, blue);
}
