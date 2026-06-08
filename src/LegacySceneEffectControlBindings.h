// Compatibility bindings from legacy EffectControls to Scene selections.

#ifndef CTHUGHA_LEGACY_SCENE_EFFECT_CONTROL_BINDINGS_H
#define CTHUGHA_LEGACY_SCENE_EFFECT_CONTROL_BINDINGS_H

class SceneVisualSelections;

/**
 * Adapter-level table for legacy EffectControl synchronization.
 *
 * Native Scene selections should not implement legacy EffectControl identity
 * APIs. This table is the temporary compatibility surface that mirrors the
 * Scene-owned selection values back to legacy controls still read by older
 * catalog/display code.
 */
class LegacySceneEffectControlBindings {
public:
    /** Destroys the compatibility binding table. */
    virtual ~LegacySceneEffectControlBindings() { }

    /** Synchronizes all legacy control values from their bound selections. */
    virtual void syncControlsFromSelections() = 0;
};

LegacySceneEffectControlBindings* legacySceneEffectControlBindings(
    SceneVisualSelections& selections);
const LegacySceneEffectControlBindings* legacySceneEffectControlBindings(
    const SceneVisualSelections& selections);

#endif
