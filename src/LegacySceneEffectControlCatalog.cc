// Legacy EffectControl routing adapter for SceneRuntime.

#include "LegacySceneEffectControlCatalog.h"

#include "LegacySceneEffectControlBindings.h"

namespace {

class LegacySceneEffectControlCatalog : public SceneEffectControlCatalog {
    SceneVisualSelections& selections;
    LegacySceneEffectControlBindings* bindings;
    int syncedImageValue;

    unsigned int imageChangeFrom(int previousImageValue) {
        return (selections.images().currentValue() != previousImageValue)
            ? SceneImageChanged
            : SceneNoChange;
    }

    void syncBoundControlsFromSelections() {
        if (bindings != 0)
            bindings->syncControlsFromSelections();
        syncedImageValue = selections.images().currentValue();
    }

public:
    explicit LegacySceneEffectControlCatalog(SceneVisualSelections& selections_)
        : selections(selections_)
        , bindings(legacySceneEffectControlBindings(selections_))
        , syncedImageValue(selections_.images().currentValue()) { }

    virtual unsigned int syncControlsFromSelections() {
        int previousImageValue = syncedImageValue;

        syncBoundControlsFromSelections();

        return imageChangeFrom(previousImageValue);
    }
};

}

std::unique_ptr<SceneEffectControlCatalog> createLegacySceneEffectControlCatalog(
    SceneVisualSelections& selections) {
    return std::unique_ptr<SceneEffectControlCatalog>(
        new LegacySceneEffectControlCatalog(selections));
}
