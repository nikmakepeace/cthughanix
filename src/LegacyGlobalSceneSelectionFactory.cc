// Scene visual selection bridge over current legacy global controls.

#include "LegacyGlobalSceneSelectionFactory.h"

#include "BorderOption.h"
#include "FlashlightOption.h"
#include "FlameOptions.h"
#include "Image.h"
#include "PaletteOption.h"
#include "SceneVisualSelections.h"
#include "TranslationOption.h"
#include "WaveOptions.h"

std::unique_ptr<SceneVisualSelections>
createLegacySceneVisualSelections(
    EffectControl& flame, EffectControl& generalFlame, EffectControl& wave,
    EffectControl& waveScale, EffectControl& table, EffectControl& object,
    EffectControl& translation, EffectControl& palette, EffectControl& border,
    EffectControl& flashlight, EffectControl& images,
    const SceneWaveObjectCatalog& waveObjects,
    const SceneImageCatalog& imageCatalog,
    const ScenePaletteCatalog& paletteCatalog,
    const SceneTranslationCatalog& translations);

std::unique_ptr<SceneVisualSelections>
createLegacyGlobalSceneVisualSelections(
    ImageOption& images, const SceneWaveObjectCatalog& waveObjects,
    const SceneImageCatalog& imageCatalog,
    const ScenePaletteCatalog& paletteCatalog,
    const SceneTranslationCatalog& translations) {
    return createLegacySceneVisualSelections(flame, flameGeneral, wave,
        waveScale, table, object, translation, palette, border, flashlight,
        images, waveObjects, imageCatalog, paletteCatalog, translations);
}
