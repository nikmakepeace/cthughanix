#ifndef __PALETTE_TRANSITION_H
#define __PALETTE_TRANSITION_H

#include "ColorPalette.h"

class FramePalette;

class PaletteTransitionStrategy {
public:
    typedef void (*Function)(ColorPalette& current, const ColorPalette& target,
        int remainingFrames);

private:
    const char* nameValue;
    Function functionValue;

public:
    PaletteTransitionStrategy(const char* name, Function function);

    const char* name() const;
    void step(ColorPalette& current, const ColorPalette& target, int remainingFrames) const;
};

class PaletteTransition {
    ColorPalette currentPalette;
    ColorPalette targetPalette;
    const PaletteTransitionStrategy* strategyValue;
    int hasTargetValue;
    int remainingFramesValue;

public:
    PaletteTransition();

    int hasTarget(const ColorPalette& target) const;
    void achieve(const ColorPalette& target, int frameBudget,
        const PaletteTransitionStrategy& strategy);
    void execute(FramePalette& framePalette);
};

const PaletteTransitionStrategy& linearPaletteTransitionStrategy();
const PaletteTransitionStrategy& squaredPaletteTransitionStrategy();
const PaletteTransitionStrategy& hslPaletteTransitionStrategy();
const PaletteTransitionStrategy& randomPaletteTransitionStrategy();

#endif
