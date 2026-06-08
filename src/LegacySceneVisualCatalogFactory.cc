// Legacy visual catalog factory over global visual options.

#include "LegacySceneVisualCatalogFactory.h"

#include "LegacyGlobalSceneSelectionFactory.h"
#include "LegacySceneCatalogAdapters.h"
#include "LegacySceneSelectionAdapters.h"
#include "LegacySceneSelectionSynchronizer.h"
#include "SceneImageCatalog.h"
#include "ScenePaletteCatalog.h"
#include "SceneTranslationCatalog.h"
#include "SceneVisualCatalogService.h"
#include "SceneWaveObjectCatalog.h"

#include <utility>

LegacySceneVisualCatalogFactory::LegacySceneVisualCatalogFactory(
    std::unique_ptr<LegacySceneSelectionAdapterSet> ownedAdapters_)
    : ownedAdapters(std::move(ownedAdapters_))
    , selections(*ownedAdapters->selections)
    , controlMirror(*ownedAdapters->controlMirror)
    , paletteRandomizer(createLegacyScenePaletteRandomizer()) { }

LegacySceneVisualCatalogFactory::~LegacySceneVisualCatalogFactory() { }

SceneVisualCatalogFactoryResult LegacySceneVisualCatalogFactory::create(
    SceneSelectionState& selectionState) {
    std::unique_ptr<SceneVisualCatalogs> visualCatalogs(
        new SceneVisualCatalogService(
            selectionState, selections, *paletteRandomizer));
    std::unique_ptr<SceneRuntimeControlBridge> controlBridge
        = createLegacySceneSelectionSynchronizer(selections, controlMirror);
    return SceneVisualCatalogFactoryResult(std::move(visualCatalogs),
        std::move(controlBridge), selections);
}

std::unique_ptr<SceneVisualCatalogFactory> createLegacySceneVisualCatalogFactory(
    ImageOption& images, const SceneWaveObjectCatalog& waveObjects,
    const SceneImageCatalog& imageCatalog,
    const ScenePaletteCatalog& paletteCatalog,
    const SceneTranslationCatalog& translations) {
    return std::unique_ptr<SceneVisualCatalogFactory>(
        new LegacySceneVisualCatalogFactory(
            createLegacyGlobalSceneSelectionAdapters(images, waveObjects,
                imageCatalog, paletteCatalog, translations)));
}
