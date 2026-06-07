// Compatibility port for EffectControl-backed Scene selections.

#ifndef CTHUGHA_LEGACY_SCENE_EFFECT_CONTROL_SELECTION_H
#define CTHUGHA_LEGACY_SCENE_EFFECT_CONTROL_SELECTION_H

#include "SceneVisualSelections.h"

class EffectControl;

/**
 * Optional compatibility operations for legacy EffectControl-backed selections.
 */
class SceneEffectControlSelection : public virtual SceneOptionSelection {
public:
    virtual ~SceneEffectControlSelection() { }

    virtual int isOption(const EffectControl& option) const = 0;
    virtual void syncFromControl() = 0;
    virtual void activate(int index) = 0;
};

#endif
