#include "cthugha.h"
#include "CthughaBuffer.h"
#include "PaletteTransition.h"

static int palettesEqual(const Palette& lhs, const Palette& rhs) {
    return memcmp(lhs, rhs, sizeof(Palette)) == 0;
}

static int absoluteValue(int value) {
    return (value < 0) ? -value : value;
}

static void stepPalette(Palette& current, const Palette& target, int remainingFrames) {
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 3; j++) {
            int delta = target[i][j] - current[i][j];
            if (delta == 0)
                continue;

            int step = (absoluteValue(delta) + remainingFrames - 1) / remainingFrames;
            if (delta < 0)
                current[i][j] -= step;
            else
                current[i][j] += step;
        }
    }
}

PaletteTransition::PaletteTransition()
    : hasTargetValue(0)
    , remainingFramesValue(0) {
    memset(targetPalette, 0, sizeof(Palette));
}

int PaletteTransition::hasTarget(const Palette& target) const {
    return hasTargetValue && palettesEqual(targetPalette, target);
}

void PaletteTransition::achieve(const Palette& target, int frameBudget) {
    if (hasTarget(target))
        return;

    memcpy(targetPalette, target, sizeof(Palette));
    remainingFramesValue = (frameBudget > 0) ? frameBudget : 0;
    hasTargetValue = 1;
}

void PaletteTransition::execute(CthughaBuffer& buffer) {
    if (!hasTargetValue) {
        buffer.palChanged = 0;
        return;
    }

    if (palettesEqual(buffer.currentPalette, targetPalette)) {
        buffer.palChanged = 0;
        return;
    }

    if (remainingFramesValue <= 0) {
        buffer.setPalette(targetPalette);
        return;
    }

    Palette nextPalette;
    memcpy(nextPalette, buffer.currentPalette, sizeof(Palette));
    stepPalette(nextPalette, targetPalette, remainingFramesValue);
    remainingFramesValue--;
    buffer.setPalette(nextPalette);
}
