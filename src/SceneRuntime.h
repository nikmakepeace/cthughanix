// Scene module ownership root.

#ifndef CTHUGHA_SCENE_RUNTIME_H
#define CTHUGHA_SCENE_RUNTIME_H

#include "Scene.h"
#include "SceneRuntimeDependencies.h"
#include "SceneSerializer.h"

#include <memory>

class RandomSource;
class RuntimeEffectControlOwner;
struct EffectPolicy;

/**
 * Owns the Scene state, command facade, selection registry, and persistence
 * contributor as one explicitly wired runtime module.
 */
class SceneRuntime {
    SceneSelectionRegistry sceneEffectRegistryValue;
    SceneSelectionPresetCatalog sceneSelectionPresetCatalogValue;
    SceneSelectionPolicyApplier sceneSelectionPolicyApplierValue;
    SceneSelectionState selectionStateValue;
    SceneVisualCatalogFactoryResult visualCatalogFactoryResultValue;
    Scene sceneValue;
    SceneCommands commandsValue;
    SceneCommandsTarget commandTargetValue;
    std::unique_ptr<RuntimeEffectControlOwner> effectControlOwnerValue;
    SceneSerializer serializerValue;

public:
    SceneRuntime(SceneGeometry& geometry,
        SceneVisualCatalogFactory& visualCatalogFactory,
        RandomSource& randomSource);
    ~SceneRuntime();

    void configureEffectPolicy(const EffectPolicy& policy);
    void applyStartupConfig(const SceneConfig& config);

    Scene& scene();
    const Scene& scene() const;
    SceneSnapshot snapshot() const;

    SceneCommandTarget& commandTarget();
    RuntimeEffectControlOwner& effectControlOwner();
    SceneSerializer& serializer();
};

#endif
