// Scene visual selection bridge over current legacy global controls.

#include "LegacyGlobalSceneSelectionFactory.h"

#include "BorderOption.h"
#include "FlashlightOption.h"
#include "FlameOptions.h"
#include "Image.h"
#include "PaletteOption.h"
#include "SceneVisualSelectionFactory.h"
#include "SceneVisualSelections.h"
#include "TranslationOption.h"
#include "WaveOptions.h"

static SceneOptionSelectionSeed seedFromLegacyControl(EffectControl& control) {
    return SceneOptionSelectionSeed(control.name(), int(control),
        int(control.lock));
}

std::unique_ptr<SceneVisualSelections>
createLegacyGlobalSceneVisualSelections(
    ImageOption& images, const SceneWaveObjectCatalog& waveObjects,
    const SceneImageCatalog& imageCatalog,
    const ScenePaletteCatalog& paletteCatalog,
    const SceneTranslationCatalog& translations) {
    SceneVisualSelectionSeeds seeds;
    seeds.flame = seedFromLegacyControl(flame);
    seeds.generalFlame = seedFromLegacyControl(flameGeneral);
    seeds.wave = seedFromLegacyControl(wave);
    seeds.waveScale = seedFromLegacyControl(waveScale);
    seeds.table = seedFromLegacyControl(table);
    seeds.object = seedFromLegacyControl(object);
    seeds.translation = seedFromLegacyControl(translation);
    seeds.palette = seedFromLegacyControl(palette);
    seeds.border = seedFromLegacyControl(border);
    seeds.flashlight = seedFromLegacyControl(flashlight);
    seeds.images = seedFromLegacyControl(images);

    return createSceneVisualSelections(seeds, waveObjects, imageCatalog,
        paletteCatalog, translations);
}
