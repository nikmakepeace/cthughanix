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
    int holdCurrentFrameValue;

public:
    PaletteTransition();

    int hasTarget(const ColorPalette& target) const;

    /**
     * Transitions from the current internal palette toward a target palette.
     *
     * @param target Palette to converge on.
     * @param frameBudget Number of visual frames over which to transition.
     * @param strategy Per-frame palette interpolation strategy.
     */
    void achieve(const ColorPalette& target, int frameBudget,
        const PaletteTransitionStrategy& strategy);

    /**
     * Snaps to one palette for the next execution, then transitions to a target.
     *
     * @param current Palette to publish on the next execute() call.
     * @param target Palette to converge on after the snap frame.
     * @param frameBudget Number of visual frames over which to transition.
     * @param strategy Per-frame palette interpolation strategy.
     */
    void snapThenAchieve(const ColorPalette& current, const ColorPalette& target,
        int frameBudget, const PaletteTransitionStrategy& strategy);

    /**
     * Advances and publishes the transition state.
     *
     * @param framePalette Display-facing frame palette to update.
     */
    void execute(FramePalette& framePalette);
};

const PaletteTransitionStrategy& linearPaletteTransitionStrategy();
const PaletteTransitionStrategy& squaredPaletteTransitionStrategy();
const PaletteTransitionStrategy& hslPaletteTransitionStrategy();
const PaletteTransitionStrategy& randomPaletteTransitionStrategy();

#endif
