// Factory for legacy EffectControl-backed Scene visual selection adapters.

#ifndef CTHUGHA_LEGACY_SCENE_SELECTION_ADAPTERS_H
#define CTHUGHA_LEGACY_SCENE_SELECTION_ADAPTERS_H

#include "SceneVisualSelections.h"

#include <memory>

class EffectControl;

/**
 * Builds legacy-backed Scene selections from current global visual controls.
 *
 * This is the compatibility factory used while catalog loading still lives in
 * legacy EffectControl instances.
 */
std::unique_ptr<SceneVisualSelections> createLegacySceneSelectionAdapters(
    EffectControl& flame, EffectControl& generalFlame, EffectControl& wave,
    EffectControl& waveScale, EffectControl& table, EffectControl& object,
    EffectControl& translation, EffectControl& palette, EffectControl& border,
    EffectControl& flashlight, EffectControl& images);

/**
 * Wraps prebuilt native selections with legacy control lookup/sync bindings.
 *
 * @param selections Owned native selection set to expose through the adapter.
 */
std::unique_ptr<SceneVisualSelections> createLegacySceneSelectionAdapters(
    EffectControl& flame, EffectControl& generalFlame, EffectControl& wave,
    EffectControl& waveScale, EffectControl& table, EffectControl& object,
    EffectControl& translation, EffectControl& palette, EffectControl& border,
    EffectControl& flashlight, EffectControl& images,
    std::unique_ptr<SceneVisualSelections> selections);

#endif
