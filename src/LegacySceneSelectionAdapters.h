// Factory for legacy EffectControl-backed Scene visual selection adapters.

#ifndef CTHUGHA_LEGACY_SCENE_SELECTION_ADAPTERS_H
#define CTHUGHA_LEGACY_SCENE_SELECTION_ADAPTERS_H

#include "SceneVisualSelections.h"

#include <memory>

class LegacySceneControlMirror;
class SceneSelectionSynchronizer;

/**
 * Owned native selections plus their explicit legacy control mirror.
 *
 * The mirror is intentionally separate from the selection set so native Scene
 * selections do not implement legacy EffectControl identity or synchronization
 * behavior.
 */
class LegacySceneSelectionAdapterSet {
public:
    std::unique_ptr<SceneVisualSelections> selections;

private:
    std::unique_ptr<LegacySceneControlMirror> controlMirror;

public:
    LegacySceneSelectionAdapterSet(
        std::unique_ptr<SceneVisualSelections> selections_,
        std::unique_ptr<LegacySceneControlMirror> controlMirror_);
    ~LegacySceneSelectionAdapterSet();

    /**
     * Creates the temporary one-way synchronizer for these adapters.
     *
     * @return Synchronizer that mirrors Scene selection values into the bound
     *         legacy controls.
     */
    std::unique_ptr<SceneSelectionSynchronizer> createSelectionSynchronizer();
};

#endif
