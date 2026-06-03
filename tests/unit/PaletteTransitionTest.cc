#include "ColorPalette.h"
#include "FramePalette.h"
#include "PaletteTransition.h"

#include <assert.h>

int cth_log_enabled(int) {
    return 0;
}

int cth_log(int, const char*, ...) {
    return 0;
}

static ColorPalette paletteWithColor(int red, int green, int blue) {
    ColorPalette palette;
    palette.setColor(0, red, green, blue);
    return palette;
}

static void testImmediateTarget() {
    ColorPalette target = paletteWithColor(10, 20, 30);
    PaletteTransition transition;
    FramePalette framePalette;

    transition.achieve(target, 0, linearPaletteTransitionStrategy());
    transition.execute(framePalette);

    assert(framePalette.currentPalette().equals(target));
}

static void testSnapThenTransitionHoldsSnapForOneExecution() {
    ColorPalette snap = paletteWithColor(120, 0, 0);
    ColorPalette target = paletteWithColor(0, 60, 0);
    PaletteTransition transition;
    FramePalette framePalette;

    transition.snapThenAchieve(snap, target, 3, linearPaletteTransitionStrategy());

    transition.execute(framePalette);
    assert(framePalette.currentPalette().equals(snap));

    transition.execute(framePalette);
    assert(!framePalette.currentPalette().equals(snap));
    assert(!framePalette.currentPalette().equals(target));

    transition.execute(framePalette);
    assert(!framePalette.currentPalette().equals(snap));
    assert(!framePalette.currentPalette().equals(target));

    transition.execute(framePalette);
    assert(framePalette.currentPalette().equals(target));
}

int main() {
    testImmediateTarget();
    testSnapThenTransitionHoldsSnapForOneExecution();
    return 0;
}
