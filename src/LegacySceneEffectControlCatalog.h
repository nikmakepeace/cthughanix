// Factory for legacy EffectControl Scene routing adapters.

#ifndef CTHUGHA_LEGACY_SCENE_EFFECT_CONTROL_CATALOG_H
#define CTHUGHA_LEGACY_SCENE_EFFECT_CONTROL_CATALOG_H

#include "SceneRuntimeDependencies.h"
#include "SceneVisualSelections.h"

#include <memory>

class EffectControl;
class RandomSource;
class SceneOptionSelection;

/**
 * Compatibility routing for legacy EffectControl commands during migration.
 */
class SceneEffectControlCatalog : public SceneRuntimeControlBridge {
public:
    virtual ~SceneEffectControlCatalog() { }

    virtual int isSceneOption(const EffectControl& option) const = 0;
    virtual SceneOptionSelection* selectionFor(EffectControl& option) = 0;
    virtual const SceneOptionSelection* selectionFor(
        const EffectControl& option) const = 0;
    virtual unsigned int change(
        SceneOptionSelection& selection, int by, RandomSource& randomSource) = 0;
    virtual unsigned int change(SceneOptionSelection& selection, const char* to,
        RandomSource& randomSource) = 0;
    virtual unsigned int activate(SceneOptionSelection& selection, int index) = 0;

    /** Toggles the Scene-owned lock corresponding to a legacy control. */
    virtual void toggleLock(SceneOptionSelection& selection) = 0;

    /** Toggles Scene-owned choice availability for a legacy control. */
    virtual void toggleChoiceUse(SceneOptionSelection& selection, int index) = 0;

    virtual RuntimeEffectControlOwner* createEffectControlOwner(
        SceneCommands& sceneCommands, SceneEffectRegistry& effectRegistry,
        RandomSource& randomSource);
};

std::unique_ptr<SceneEffectControlCatalog> createLegacySceneEffectControlCatalog(
    SceneVisualSelections& selections);

#endif
