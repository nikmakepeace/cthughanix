// Factory for Scene visual selections backed by current legacy globals.

#ifndef CTHUGHA_LEGACY_GLOBAL_SCENE_SELECTION_FACTORY_H
#define CTHUGHA_LEGACY_GLOBAL_SCENE_SELECTION_FACTORY_H

#include <memory>

class ImageOption;
class LegacySceneSelectionAdapterSet;
class SceneImageCatalog;
class ScenePaletteCatalog;
class SceneTranslationCatalog;
class SceneWaveObjectCatalog;

/**
 * Builds the temporary Scene visual selection bridge from legacy globals.
 *
 * This is the only selection-construction helper that reads the current global
 * visual controls. Callers supply native catalogs plus the Application-owned
 * image option while the legacy loaders are still active.
 *
 * @param images Temporary image option owned by Application.
 * @param waveObjects Native wave-object catalog copied from the legacy loader.
 * @param imageCatalog Native image catalog copied from the image loader.
 * @param paletteCatalog Native palette catalog copied from the palette loader.
 * @param translations Native translation catalog generated at startup.
 * @return Owned native selections and explicit legacy mirror.
 */
std::unique_ptr<LegacySceneSelectionAdapterSet>
createLegacyGlobalSceneSelectionAdapters(
    ImageOption& images, const SceneWaveObjectCatalog& waveObjects,
    const SceneImageCatalog& imageCatalog,
    const ScenePaletteCatalog& paletteCatalog,
    const SceneTranslationCatalog& translations);

#endif
