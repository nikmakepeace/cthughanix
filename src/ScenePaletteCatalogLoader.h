// Scene palette catalog loader compatibility helpers.

#ifndef CTHUGHA_SCENE_PALETTE_CATALOG_LOADER_H
#define CTHUGHA_SCENE_PALETTE_CATALOG_LOADER_H

class EffectControl;
class ScenePaletteCatalog;

/**
 * Copies palette option entries into a native Scene palette catalog.
 *
 * This quarantines the remaining palette option loader outside Scene runtime
 * construction while palette file loading is still compatibility backed.
 *
 * @param paletteOption Palette EffectControl populated by load_palettes().
 * @param catalog Native catalog to replace with copied entries.
 */
void copyScenePaletteCatalogFromEffectControl(
    EffectControl& paletteOption, ScenePaletteCatalog& catalog);

#endif
