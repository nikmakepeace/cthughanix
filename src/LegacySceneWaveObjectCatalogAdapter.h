// Legacy loader adapter for native Scene wave-object catalogs.

#ifndef CTHUGHA_LEGACY_SCENE_WAVE_OBJECT_CATALOG_ADAPTER_H
#define CTHUGHA_LEGACY_SCENE_WAVE_OBJECT_CATALOG_ADAPTER_H

class EffectControl;
class SceneWaveObjectCatalog;

/**
 * Copies legacy object option entries into a native Scene wave-object catalog.
 *
 * This quarantines the remaining EffectControl-backed object loader outside the
 * Scene selection factory while object file loading is still legacy.
 *
 * @param objectOption Legacy object EffectControl populated by init_wave().
 * @param catalog Native catalog to replace with copied entries.
 */
void loadSceneWaveObjectCatalogFromLegacy(
    EffectControl& objectOption, SceneWaveObjectCatalog& catalog);

#endif
