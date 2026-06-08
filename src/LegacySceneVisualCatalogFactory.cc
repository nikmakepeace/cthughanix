// Legacy visual catalog factory over global visual options.

#include "LegacySceneVisualCatalogFactory.h"

#include "LegacyGlobalSceneSelectionFactory.h"
#include "LegacyScenePaletteRandomizer.h"
#include "SceneDependencies.h"
#include "SceneImageCatalog.h"
#include "ScenePaletteCatalog.h"
#include "SceneTranslationCatalog.h"
#include "SceneVisualCatalogService.h"
#include "SceneVisualSelections.h"
#include "SceneWaveObjectCatalog.h"

#include <utility>

namespace {

/**
 * Compatibility synchronizer used after Scene selections stop mirroring back.
 */
class NoopSceneSelectionSynchronizer : public SceneSelectionSynchronizer {
public:
    /** Returns no forced Scene changes and performs no legacy control writes. */
    virtual unsigned int syncControlsFromSelections() {
        return SceneNoChange;
    }
};

}

LegacySceneVisualCatalogFactory::LegacySceneVisualCatalogFactory(
    std::unique_ptr<SceneVisualSelections> ownedSelections_)
    : ownedSelections(std::move(ownedSelections_))
    , selections(*ownedSelections)
    , paletteRandomizer(createLegacyScenePaletteRandomizer()) { }

LegacySceneVisualCatalogFactory::~LegacySceneVisualCatalogFactory() { }

SceneVisualCatalogFactoryResult LegacySceneVisualCatalogFactory::create(
    SceneSelectionState& selectionState) {
    std::unique_ptr<SceneVisualCatalogs> visualCatalogs(
        new SceneVisualCatalogService(
            selectionState, selections, *paletteRandomizer));
    return SceneVisualCatalogFactoryResult(std::move(visualCatalogs),
        std::unique_ptr<SceneSelectionSynchronizer>(
            new NoopSceneSelectionSynchronizer()),
        selections);
}

std::unique_ptr<SceneVisualCatalogFactory> createLegacySceneVisualCatalogFactory(
    ImageOption& images, const SceneWaveObjectCatalog& waveObjects,
    const SceneImageCatalog& imageCatalog,
    const ScenePaletteCatalog& paletteCatalog,
    const SceneTranslationCatalog& translations) {
    return std::unique_ptr<SceneVisualCatalogFactory>(
        new LegacySceneVisualCatalogFactory(
            createLegacyGlobalSceneVisualSelections(images, waveObjects,
                imageCatalog, paletteCatalog, translations)));
}
