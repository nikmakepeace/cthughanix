// Legacy loader adapter for native Scene image catalogs.

#ifndef CTHUGHA_LEGACY_SCENE_IMAGE_CATALOG_ADAPTER_H
#define CTHUGHA_LEGACY_SCENE_IMAGE_CATALOG_ADAPTER_H

class ImageOption;
class SceneImageCatalog;

/**
 * Copies legacy image option entries into a native Scene image catalog.
 *
 * This quarantines the remaining ImageOption-backed loader outside the Scene
 * selection factory while image file loading is still legacy.
 *
 * @param imageOption Legacy image option populated by FrameGeneratorRuntime.
 * @param catalog Native catalog to replace with copied entries.
 */
void loadSceneImageCatalogFromLegacy(
    ImageOption& imageOption, SceneImageCatalog& catalog);

#endif
