#include "EffectControl.h"
#include "EffectChoiceLoader.h"
#include "Flame.h"
#include "Image.h"
#include "SceneImageCatalog.h"
#include "ScenePaletteCatalog.h"
#include "SceneTranslationCatalog.h"
#include "SceneVisualSelections.h"
#include "SceneWaveObjectCatalog.h"
#include "Wave.h"
#include "pcx.h"
#include "png.h"

#include <cassert>
#include <memory>

std::unique_ptr<SceneVisualSelections> createLegacySceneVisualSelections(
    EffectControl& flame, EffectControl& generalFlame, EffectControl& wave,
    EffectControl& waveScale, EffectControl& table, EffectControl& object,
    EffectControl& translation, EffectControl& palette, EffectControl& border,
    EffectControl& flashlight, EffectControl& images,
    const SceneWaveObjectCatalog& waveObjects,
    const SceneImageCatalog& imageCatalog,
    const ScenePaletteCatalog& paletteCatalog,
    const SceneTranslationCatalog& translations);

int cth_log_enabled(int) { return 0; }
int cth_log(int, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }

const int nFlameCatalogEntries = 0;
const int nWaveCatalogEntries = 0;

const char* Flame::name() const { return ""; }
const char* Wave::name() const { return ""; }

const Flame* flameByIndex(int) { return 0; }
Wave* waveByIndex(int) { return 0; }

EffectChoice* read_pcx_image(FILE*, const char*, const char*, const char*,
    const ImageLoadTarget&) {
    return 0;
}

EffectChoice* read_png_image(FILE*, const char*, const char*, const char*,
    const ImageLoadTarget&) {
    return 0;
}

int loadEffectChoices(EffectControl&, const PathConfig&, const char*[],
    const char*, const char*, EffectChoiceContextLoader, void*) {
    return 0;
}

static void seedControl(EffectControl& control, int value, int lockValue) {
    control.setValue(value);
    control.lock.setValue(lockValue);
}

static void assertSelectionState(
    const SceneOptionSelection& selection, int value, int lockValue) {
    assert(selection.currentValue() == value);
    assert(selection.lockEnabled() == lockValue);
}

static void testFactorySeedsSelectionsFromLegacyControls() {
    EffectChoiceList choices;
    EffectControl flame(0, "flame", choices);
    EffectControl generalFlame(0, "flame-general", choices);
    EffectControl wave(0, "wave", choices);
    EffectControl waveScale(0, "wave-scale", choices);
    EffectControl table(0, "table", choices);
    EffectControl object(0, "object", choices);
    EffectControl translation(0, "translation", choices);
    EffectControl palette(0, "palette", choices);
    EffectControl border(0, "border", choices);
    EffectControl flashlight(0, "flashlight", choices);
    EffectControl images(0, "images", choices);
    SceneWaveObjectCatalog waveObjects;
    SceneImageCatalog imageCatalog;
    ScenePaletteCatalog paletteCatalog;
    SceneTranslationCatalog translations;

    seedControl(flame, 1, 1);
    seedControl(generalFlame, 2, 0);
    seedControl(wave, 3, 1);
    seedControl(waveScale, 1, 0);
    seedControl(table, 4, 1);
    seedControl(object, 0, 0);
    seedControl(translation, 0, 1);
    seedControl(palette, 0, 0);
    seedControl(border, 2, 1);
    seedControl(flashlight, 1, 0);
    seedControl(images, 0, 1);

    std::unique_ptr<SceneVisualSelections> selections
        = createLegacySceneVisualSelections(flame, generalFlame, wave,
            waveScale, table, object, translation, palette, border,
            flashlight, images, waveObjects, imageCatalog, paletteCatalog,
            translations);

    assertSelectionState(selections->flame(), 1, 1);
    assertSelectionState(selections->generalFlame(), 2, 0);
    assertSelectionState(selections->wave(), 3, 1);
    assertSelectionState(selections->waveScale(), 1, 0);
    assertSelectionState(selections->table(), 4, 1);
    assertSelectionState(selections->object(), 0, 0);
    assertSelectionState(selections->translation(), 0, 1);
    assertSelectionState(selections->palette(), 0, 0);
    assertSelectionState(selections->border(), 2, 1);
    assertSelectionState(selections->flashlight(), 1, 0);
    assertSelectionState(selections->images(), 0, 1);
}

static void testSelectionsDoNotMirrorBackToLegacyControls() {
    EffectChoiceList choices;
    EffectControl flame(0, "flame", choices);
    EffectControl generalFlame(0, "flame-general", choices);
    EffectControl wave(0, "wave", choices);
    EffectControl waveScale(0, "wave-scale", choices);
    EffectControl table(0, "table", choices);
    EffectControl object(0, "object", choices);
    EffectControl translation(0, "translation", choices);
    EffectControl palette(0, "palette", choices);
    EffectControl border(0, "border", choices);
    EffectControl flashlight(0, "flashlight", choices);
    EffectControl images(0, "images", choices);
    SceneWaveObjectCatalog waveObjects;
    SceneImageCatalog imageCatalog;
    ScenePaletteCatalog paletteCatalog;
    SceneTranslationCatalog translations;

    seedControl(flame, 1, 1);

    std::unique_ptr<SceneVisualSelections> selections
        = createLegacySceneVisualSelections(flame, generalFlame, wave,
            waveScale, table, object, translation, palette, border,
            flashlight, images, waveObjects, imageCatalog, paletteCatalog,
            translations);

    selections->flame().setValue(2);
    selections->flame().toggleLock();

    assert(int(flame) == 1);
    assert(int(flame.lock) == 1);
}

int main() {
    testFactorySeedsSelectionsFromLegacyControls();
    testSelectionsDoNotMirrorBackToLegacyControls();
    return 0;
}
