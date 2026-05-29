#ifndef __PALETTE_TRANSITION_H
#define __PALETTE_TRANSITION_H

#include "display.h"

class CthughaBuffer;

class PaletteTransition {
    Palette targetPalette;
    int hasTargetValue;
    int remainingFramesValue;

public:
    PaletteTransition();

    int hasTarget(const Palette& target) const;
    void achieve(const Palette& target, int frameBudget);
    void execute(CthughaBuffer& buffer);
};

#endif
