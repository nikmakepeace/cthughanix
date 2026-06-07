// Factory for legacy EffectControl-backed Scene visual selection adapters.

#ifndef CTHUGHA_LEGACY_SCENE_SELECTION_ADAPTERS_H
#define CTHUGHA_LEGACY_SCENE_SELECTION_ADAPTERS_H

#include "SceneVisualSelections.h"

#include <memory>

class EffectControl;

std::unique_ptr<SceneVisualSelections> createLegacySceneSelectionAdapters(
    EffectControl& flame, EffectControl& generalFlame, EffectControl& wave,
    EffectControl& waveScale, EffectControl& table, EffectControl& object,
    EffectControl& translation, EffectControl& palette, EffectControl& border,
    EffectControl& flashlight, EffectControl& images);

#endif
