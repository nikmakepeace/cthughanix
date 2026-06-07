// Factory for legacy EffectControl Scene routing adapters.

#ifndef CTHUGHA_LEGACY_SCENE_EFFECT_CONTROL_CATALOG_H
#define CTHUGHA_LEGACY_SCENE_EFFECT_CONTROL_CATALOG_H

#include "SceneRuntimeDependencies.h"
#include "SceneVisualSelections.h"

#include <memory>

class EffectControl;
class RandomSource;

/**
 * Compatibility routing for legacy EffectControl commands during migration.
 */
class SceneEffectControlCatalog : public SceneRuntimeControlBridge {
public:
    virtual ~SceneEffectControlCatalog() { }

    virtual int isSceneOption(const EffectControl& option) const = 0;
    virtual unsigned int change(
        EffectControl& option, int by, RandomSource& randomSource) = 0;
    virtual unsigned int change(EffectControl& option, const char* to,
        RandomSource& randomSource) = 0;
    virtual unsigned int activate(EffectControl& option, int index) = 0;
    virtual RuntimeEffectControlOwner* createEffectControlOwner(
        SceneCommands& sceneCommands, SceneEffectRegistry& effectRegistry,
        RandomSource& randomSource);
};

std::unique_ptr<SceneEffectControlCatalog> createLegacySceneEffectControlCatalog(
    SceneVisualSelections& selections);

#endif
