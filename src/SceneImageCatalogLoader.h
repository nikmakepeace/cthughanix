// Scene image catalog loader compatibility helpers.

#ifndef CTHUGHA_SCENE_IMAGE_CATALOG_LOADER_H
#define CTHUGHA_SCENE_IMAGE_CATALOG_LOADER_H

class ImageOption;
class SceneImageCatalog;

/**
 * Copies image option entries into a native Scene image catalog.
 *
 * This quarantines the remaining ImageOption-backed loader outside Scene
 * runtime construction while image file loading is still compatibility backed.
 *
 * @param imageOption Image option populated by the image file loader.
 * @param catalog Native catalog to replace with copied entries.
 */
void copySceneImageCatalogFromImageOption(
    ImageOption& imageOption, SceneImageCatalog& catalog);

#endif
