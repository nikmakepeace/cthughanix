// Per-frame palette state and dirty tracking.

#include "cthugha.h"
#include "FramePalette.h"

FramePalette::FramePalette()
    : paletteDirtyValue(1) { }

const ColorPalette& FramePalette::currentPalette() const {
    return currentPaletteValue;
}

int FramePalette::paletteDirty() const {
    return paletteDirtyValue;
}

void FramePalette::clearPaletteDirty() {
    paletteDirtyValue = 0;
}

void FramePalette::setPalette(const ColorPalette& palette) {
    if (!currentPaletteValue.equals(palette)) {
        currentPaletteValue.copyFrom(palette);
        paletteDirtyValue = 1;
    }
}
