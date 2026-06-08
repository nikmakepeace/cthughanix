// Factory for legacy EffectControl Scene routing adapters.

#ifndef CTHUGHA_LEGACY_SCENE_EFFECT_CONTROL_CATALOG_H
#define CTHUGHA_LEGACY_SCENE_EFFECT_CONTROL_CATALOG_H

#include "SceneRuntimeDependencies.h"
#include "SceneVisualSelections.h"

#include <memory>

/**
 * Compatibility synchronizer for legacy EffectControl state during migration.
 */
class SceneEffectControlCatalog : public SceneRuntimeControlBridge {
public:
    virtual ~SceneEffectControlCatalog() { }
};

std::unique_ptr<SceneEffectControlCatalog> createLegacySceneEffectControlCatalog(
    SceneVisualSelections& selections);

#endif
