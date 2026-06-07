// Legacy visual catalog adapter for SceneRuntime.

#ifndef CTHUGHA_LEGACY_SCENE_VISUAL_CATALOGS_H
#define CTHUGHA_LEGACY_SCENE_VISUAL_CATALOGS_H

#include "LegacySceneCatalogAdapters.h"
#include "SceneRuntimeDependencies.h"
#include "SceneVisualSelections.h"

#include <memory>

class EffectControl;

/**
 * Compatibility visual catalog adapter over legacy global EffectControls.
 */
class LegacySceneVisualCatalogs : public SceneVisualCatalogs {
    SceneSelectionState& selectionState;
    SceneVisualSelections& selections;
    SceneWaveObjectSource& waveObjects;
    ScenePaletteRandomizer& paletteRandomizer;

    Wave* selectRunnableWave(const WaveConfig& config);

public:
    LegacySceneVisualCatalogs(SceneSelectionState& selectionState_,
        SceneVisualSelections& selections_,
        SceneWaveObjectSource& waveObjects_,
        ScenePaletteRandomizer& paletteRandomizer_);

    virtual const SceneSettings& currentSettings(SceneGeometry& geometry);
    virtual const IndexedImage* currentImage();

    virtual void applyStartupConfig(
        const SceneConfig& config, RandomSource& randomSource);
    virtual unsigned int change(
        SceneSelectionTarget target, int by, RandomSource& randomSource);
    virtual unsigned int change(SceneSelectionTarget target, const char* to,
        RandomSource& randomSource);
    virtual unsigned int randomPalette(RandomSource& randomSource);
    virtual unsigned int addRandomPalette(RandomSource& randomSource);
};

class LegacySceneVisualCatalogFactory : public SceneVisualCatalogFactory {
    std::unique_ptr<SceneVisualSelections> ownedSelections;
    SceneVisualSelections& selections;
    std::unique_ptr<SceneWaveObjectSource> waveObjects;
    std::unique_ptr<ScenePaletteRandomizer> paletteRandomizer;

public:
    explicit LegacySceneVisualCatalogFactory(SceneVisualSelections& selections_);
    explicit LegacySceneVisualCatalogFactory(
        std::unique_ptr<SceneVisualSelections> ownedSelections_);

    virtual SceneVisualCatalogFactoryResult create(
        SceneSelectionState& selectionState);
};

std::unique_ptr<SceneVisualCatalogFactory> createLegacySceneVisualCatalogFactory(
    EffectControl& flame, EffectControl& generalFlame, EffectControl& wave,
    EffectControl& waveScale, EffectControl& table, EffectControl& object,
    EffectControl& translation, EffectControl& palette, EffectControl& border,
    EffectControl& flashlight, EffectControl& images);

#endif
