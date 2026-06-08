// Legacy control mirror synchronizer for SceneRuntime.

#include "LegacySceneSelectionSynchronizer.h"

#include "LegacySceneControlMirror.h"

namespace {

class LegacySceneSelectionSynchronizer : public SceneSelectionSynchronizer {
    SceneVisualSelections& selections;
    LegacySceneControlMirror& mirror;
    int syncedImageValue;

    unsigned int imageChangeFrom(int previousImageValue) {
        return (selections.images().currentValue() != previousImageValue)
            ? SceneImageChanged
            : SceneNoChange;
    }

    void syncBoundControlsFromSelections() {
        mirror.syncControlsFromSelections();
        syncedImageValue = selections.images().currentValue();
    }

public:
    LegacySceneSelectionSynchronizer(SceneVisualSelections& selections_,
        LegacySceneControlMirror& mirror_)
        : selections(selections_)
        , mirror(mirror_)
        , syncedImageValue(selections_.images().currentValue()) { }

    virtual unsigned int syncControlsFromSelections() {
        int previousImageValue = syncedImageValue;

        syncBoundControlsFromSelections();

        return imageChangeFrom(previousImageValue);
    }
};

}

std::unique_ptr<SceneSelectionSynchronizer> createLegacySceneSelectionSynchronizer(
    SceneVisualSelections& selections, LegacySceneControlMirror& mirror) {
    return std::unique_ptr<SceneSelectionSynchronizer>(
        new LegacySceneSelectionSynchronizer(selections, mirror));
}
