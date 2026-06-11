/** @file
 * Runtime frame-generator control adapter.
 */

#include "RuntimeFrameGeneratorControls.h"

#include "FrameGeneratorRuntime.h"

DefaultRuntimeFrameGeneratorControls::DefaultRuntimeFrameGeneratorControls(
    FrameGeneratorRuntime& frameGenerator_)
    : frameGenerator(frameGenerator_) { }

void DefaultRuntimeFrameGeneratorControls::changePaletteSmoothingChanceTo(
    double chance) {
    frameGenerator.setPaletteSmoothingChance(chance);
}
