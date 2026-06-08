// Legacy EffectControl routing adapter for SceneRuntime.

#include "LegacySceneEffectControlCatalog.h"

#include "EffectControl.h"
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

    unsigned int changeForSelection(
        SceneOptionSelection* selection, int previousImageValue) {
        if (selection == &selections.images())
            return imageChangeFrom(previousImageValue);
        return SceneNoChange;
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

    virtual unsigned int syncFromControls() {
        int previousImageValue = selections.images().currentValue();

        if (bindings != 0)
            bindings->syncFromControls();
        syncedImageValue = selections.images().currentValue();

        return imageChangeFrom(previousImageValue);
    }

    virtual unsigned int syncControlsFromSelections() {
        int previousImageValue = syncedImageValue;

        syncBoundControlsFromSelections();

        return imageChangeFrom(previousImageValue);
    }

    virtual SceneOptionSelection* selectionFor(EffectControl& option) {
        return const_cast<SceneOptionSelection*>(
            static_cast<const LegacySceneEffectControlCatalog*>(this)
                ->selectionFor(option));
    }

    virtual const SceneOptionSelection* selectionFor(
        const EffectControl& option) const {
        if (bindings == 0)
            return 0;

        return bindings->selectionFor(option);
    }

    virtual unsigned int change(
        SceneOptionSelection& selection, int by, RandomSource& randomSource) {
        (void)randomSource;
        int previousImageValue = selections.images().currentValue();
        selection.change(by);
        syncBoundControlsFromSelections();
        return changeForSelection(&selection, previousImageValue);
    }

    virtual unsigned int change(SceneOptionSelection& selection, const char* to,
        RandomSource& randomSource) {
        int previousImageValue = selections.images().currentValue();
        selection.change(to, randomSource);
        syncBoundControlsFromSelections();
        return changeForSelection(&selection, previousImageValue);
    }

    virtual unsigned int activate(SceneOptionSelection& selection, int index) {
        int previousImageValue = selections.images().currentValue();
        selection.activate(index);
        syncBoundControlsFromSelections();
        return changeForSelection(&selection, previousImageValue);
    }

    virtual void toggleLock(SceneOptionSelection& selection) {
        selection.toggleLock();
    }

    virtual void toggleChoiceUse(SceneOptionSelection& selection, int index) {
        selection.toggleChoiceUse(index);
    }
};

}

std::unique_ptr<SceneEffectControlCatalog> createLegacySceneEffectControlCatalog(
    SceneVisualSelections& selections) {
    return std::unique_ptr<SceneEffectControlCatalog>(
        new LegacySceneEffectControlCatalog(selections));
}
