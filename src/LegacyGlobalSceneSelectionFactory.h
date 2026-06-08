// Factory for Scene visual selections backed by current legacy globals.

#ifndef CTHUGHA_LEGACY_GLOBAL_SCENE_SELECTION_FACTORY_H
#define CTHUGHA_LEGACY_GLOBAL_SCENE_SELECTION_FACTORY_H

#include <memory>

class ImageOption;
class SceneImageCatalog;
class ScenePaletteCatalog;
class SceneTranslationCatalog;
class SceneVisualSelections;
class SceneWaveObjectCatalog;

/**
 * Builds Scene visual selections from legacy global startup state.
 *
 * This is the only selection-construction helper that reads the current global
 * visual controls. It seeds native Scene selections from legacy startup values
 * without creating a reverse mirror back into those controls.
 *
 * @param images Temporary image option owned by Application.
 * @param waveObjects Native wave-object catalog copied from the legacy loader.
 * @param imageCatalog Native image catalog copied from the image loader.
 * @param paletteCatalog Native palette catalog copied from the palette loader.
 * @param translations Native translation catalog generated at startup.
 * @return Owned native selections initialized from legacy globals.
 */
std::unique_ptr<SceneVisualSelections>
createLegacyGlobalSceneVisualSelections(
    ImageOption& images, const SceneWaveObjectCatalog& waveObjects,
    const SceneImageCatalog& imageCatalog,
    const ScenePaletteCatalog& paletteCatalog,
    const SceneTranslationCatalog& translations);

#endif
