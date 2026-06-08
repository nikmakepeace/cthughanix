// Legacy visual selection construction over global visual options.

#include "LegacySceneSelectionAdapters.h"

#include "EffectControl.h"
#include "LegacySceneChoiceLock.h"
#include "SceneBuiltInChoiceCatalogs.h"
#include "SceneChoiceSelection.h"
#include "SceneGeneralFlameSelectionValue.h"
#include "SceneTypedVisualCatalogs.h"
#include "SceneVisualSelectionSet.h"

std::unique_ptr<LegacySceneSelectionAdapterSet>
createLegacySceneSelectionAdapters(
    EffectControl& flame, EffectControl& generalFlame, EffectControl& wave,
    EffectControl& waveScale, EffectControl& table, EffectControl& object,
    EffectControl& translation, EffectControl& palette, EffectControl& border,
    EffectControl& flashlight, EffectControl& images,
    const SceneWaveObjectCatalog& waveObjects,
    const SceneImageCatalog& imageCatalog,
    const ScenePaletteCatalog& paletteCatalog,
    const SceneTranslationCatalog& translations) {
    return createLegacySceneSelectionAdapters(flame, generalFlame, wave,
        waveScale, table, object, translation, palette, border, flashlight,
        images,
        std::unique_ptr<SceneVisualSelections>(new SceneVisualSelectionSet(
            new SceneFlameChoiceSelection(
                createSceneFlameChoiceCatalog(flame.name(),
                    new LegacySceneChoiceLock(flame.lock)),
                int(flame)),
            new SceneGeneralFlameSelectionValue(generalFlame.name(),
                new LegacySceneChoiceLock(generalFlame.lock),
                int(generalFlame)),
            new SceneWaveChoiceSelection(
                createSceneWaveChoiceCatalog(wave.name(),
                    new LegacySceneChoiceLock(wave.lock)),
                int(wave)),
            new SceneChoiceSelection(
                createSceneWaveScaleChoiceCatalog(waveScale.name(),
                    new LegacySceneChoiceLock(waveScale.lock)),
                int(waveScale)),
            new SceneChoiceSelection(
                createSceneTableChoiceCatalog(table.name(),
                    new LegacySceneChoiceLock(table.lock)),
                int(table)),
            new SceneWaveObjectChoiceSelection(
                createSceneWaveObjectChoiceCatalog(object.name(),
                    new LegacySceneChoiceLock(object.lock), waveObjects),
                int(object)),
            new SceneTranslationChoiceSelection(
                createSceneTranslationChoiceCatalog(translation.name(),
                    new LegacySceneChoiceLock(translation.lock), translations),
                int(translation)),
            new ScenePaletteChoiceSelection(
                createScenePaletteChoiceCatalog(palette.name(),
                    new LegacySceneChoiceLock(palette.lock), paletteCatalog),
                int(palette)),
            new SceneChoiceSelection(
                createSceneBorderChoiceCatalog(border.name(),
                    new LegacySceneChoiceLock(border.lock)),
                int(border)),
            new SceneChoiceSelection(
                createSceneFlashlightChoiceCatalog(flashlight.name(),
                    new LegacySceneChoiceLock(flashlight.lock)),
                int(flashlight)),
            new SceneImageChoiceSelection(
                createSceneImageChoiceCatalog(images.name(),
                    new LegacySceneChoiceLock(images.lock), imageCatalog),
                int(images)))));
}
