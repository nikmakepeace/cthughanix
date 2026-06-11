/** @file
 * Frame-generator runtime configuration contributor.
 */

#include "RuntimeConfigRegistry.h"

#include "FrameGeneratorRuntime.h"

FrameGeneratorRuntimeConfigContributor::FrameGeneratorRuntimeConfigContributor(
    const FrameGeneratorRuntime& frameGenerator_)
    : frameGenerator(frameGenerator_) { }

void FrameGeneratorRuntimeConfigContributor::contribute(Config& config) const {
    config.sceneTransition.paletteSmoothingChance
        = frameGenerator.paletteSmoothingChance();
    config.sceneTransition.paletteSmoothSeconds
        = frameGenerator.paletteSmoothSeconds();
}
