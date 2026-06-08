// Scene wave-object catalog loader compatibility helpers.

#ifndef CTHUGHA_SCENE_WAVE_OBJECT_CATALOG_LOADER_H
#define CTHUGHA_SCENE_WAVE_OBJECT_CATALOG_LOADER_H

class EffectControl;
class SceneWaveObjectCatalog;

/**
 * Copies object option entries into a native Scene wave-object catalog.
 *
 * This quarantines the remaining EffectControl-backed object loader outside
 * Scene runtime construction while object file loading is still compatibility
 * backed.
 *
 * @param objectOption Object EffectControl populated by init_wave().
 * @param catalog Native catalog to replace with copied entries.
 */
void copySceneWaveObjectCatalogFromEffectControl(
    EffectControl& objectOption, SceneWaveObjectCatalog& catalog);

#endif
