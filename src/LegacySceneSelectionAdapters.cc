// Legacy EffectControl-backed Scene visual selection adapters.

#include "LegacySceneSelectionAdapters.h"

#include "EffectControl.h"
#include "LegacySceneEffectControlBindings.h"

#include <memory>
#include <utility>

namespace {

class LegacySceneSelectionAdapters : public SceneVisualSelections,
    public LegacySceneEffectControlBindings {
    EffectControl& flameControl;
    EffectControl& generalFlameControl;
    EffectControl& waveControl;
    EffectControl& waveScaleControl;
    EffectControl& tableControl;
    EffectControl& objectControl;
    EffectControl& translationControl;
    EffectControl& paletteControl;
    EffectControl& borderControl;
    EffectControl& flashlightControl;
    EffectControl& imagesControl;
    std::unique_ptr<SceneVisualSelections> selections;

public:
    LegacySceneSelectionAdapters(EffectControl& flame_,
        EffectControl& generalFlame_, EffectControl& wave_,
        EffectControl& waveScale_, EffectControl& table_,
        EffectControl& object_, EffectControl& translation_,
        EffectControl& palette_, EffectControl& border_,
        EffectControl& flashlight_, EffectControl& images_,
        std::unique_ptr<SceneVisualSelections> selections_);

    virtual SceneFlameSelection& flame();
    virtual SceneGeneralFlameSelection& generalFlame();
    virtual SceneWaveSelection& wave();
    virtual SceneOptionSelection& waveScale();
    virtual SceneOptionSelection& table();
    virtual SceneOptionSelection& object();
    virtual SceneTranslationSelection& translation();
    virtual ScenePaletteSelection& palette();
    virtual SceneOptionSelection& border();
    virtual SceneOptionSelection& flashlight();
    virtual SceneImageSelection& images();

    virtual SceneOptionSelection* selectionFor(EffectControl& option);
    virtual const SceneOptionSelection* selectionFor(
        const EffectControl& option) const;
    virtual void syncFromControls();
    virtual void syncControlsFromSelections();
};

LegacySceneSelectionAdapters::LegacySceneSelectionAdapters(EffectControl& flame_,
    EffectControl& generalFlame_, EffectControl& wave_,
    EffectControl& waveScale_, EffectControl& table_, EffectControl& object_,
    EffectControl& translation_, EffectControl& palette_,
    EffectControl& border_, EffectControl& flashlight_, EffectControl& images_,
    std::unique_ptr<SceneVisualSelections> selections_)
    : flameControl(flame_)
    , generalFlameControl(generalFlame_)
    , waveControl(wave_)
    , waveScaleControl(waveScale_)
    , tableControl(table_)
    , objectControl(object_)
    , translationControl(translation_)
    , paletteControl(palette_)
    , borderControl(border_)
    , flashlightControl(flashlight_)
    , imagesControl(images_)
    , selections(std::move(selections_)) { }

SceneFlameSelection& LegacySceneSelectionAdapters::flame() {
    return selections->flame();
}

SceneGeneralFlameSelection& LegacySceneSelectionAdapters::generalFlame() {
    return selections->generalFlame();
}

SceneWaveSelection& LegacySceneSelectionAdapters::wave() {
    return selections->wave();
}

SceneOptionSelection& LegacySceneSelectionAdapters::waveScale() {
    return selections->waveScale();
}

SceneOptionSelection& LegacySceneSelectionAdapters::table() {
    return selections->table();
}

SceneOptionSelection& LegacySceneSelectionAdapters::object() {
    return selections->object();
}

SceneTranslationSelection& LegacySceneSelectionAdapters::translation() {
    return selections->translation();
}

ScenePaletteSelection& LegacySceneSelectionAdapters::palette() {
    return selections->palette();
}

SceneOptionSelection& LegacySceneSelectionAdapters::border() {
    return selections->border();
}

SceneOptionSelection& LegacySceneSelectionAdapters::flashlight() {
    return selections->flashlight();
}

SceneImageSelection& LegacySceneSelectionAdapters::images() {
    return selections->images();
}

SceneOptionSelection* LegacySceneSelectionAdapters::selectionFor(
    EffectControl& option) {
    return const_cast<SceneOptionSelection*>(
        static_cast<const LegacySceneSelectionAdapters*>(this)->selectionFor(
            option));
}

const SceneOptionSelection* LegacySceneSelectionAdapters::selectionFor(
    const EffectControl& option) const {
    if (&option == &flameControl)
        return &selections->flame();
    if (&option == &generalFlameControl)
        return &selections->generalFlame();
    if (&option == &waveControl)
        return &selections->wave();
    if (&option == &waveScaleControl)
        return &selections->waveScale();
    if (&option == &objectControl)
        return &selections->object();
    if (&option == &translationControl)
        return &selections->translation();
    if (&option == &borderControl)
        return &selections->border();
    if (&option == &flashlightControl)
        return &selections->flashlight();
    if (&option == &paletteControl)
        return &selections->palette();
    if (&option == &tableControl)
        return &selections->table();
    if (&option == &imagesControl)
        return &selections->images();

    return 0;
}

void LegacySceneSelectionAdapters::syncFromControls() {
    selections->flame().setValue(int(flameControl));
    selections->generalFlame().setValue(int(generalFlameControl));
    selections->wave().setValue(int(waveControl));
    selections->waveScale().setValue(int(waveScaleControl));
    selections->table().setValue(int(tableControl));
    selections->object().setValue(int(objectControl));
    selections->translation().setValue(int(translationControl));
    selections->palette().setValue(int(paletteControl));
    selections->border().setValue(int(borderControl));
    selections->flashlight().setValue(int(flashlightControl));
    selections->images().setValue(int(imagesControl));
}

void LegacySceneSelectionAdapters::syncControlsFromSelections() {
    flameControl.setValue(selections->flame().currentValue());
    generalFlameControl.setValue(selections->generalFlame().currentValue());
    waveControl.setValue(selections->wave().currentValue());
    waveScaleControl.setValue(selections->waveScale().currentValue());
    tableControl.setValue(selections->table().currentValue());
    objectControl.setValue(selections->object().currentValue());
    translationControl.setValue(selections->translation().currentValue());
    paletteControl.setValue(selections->palette().currentValue());
    borderControl.setValue(selections->border().currentValue());
    flashlightControl.setValue(selections->flashlight().currentValue());
    imagesControl.setValue(selections->images().currentValue());
}

}

LegacySceneEffectControlBindings* legacySceneEffectControlBindings(
    SceneVisualSelections& selections) {
    return dynamic_cast<LegacySceneEffectControlBindings*>(&selections);
}

const LegacySceneEffectControlBindings* legacySceneEffectControlBindings(
    const SceneVisualSelections& selections) {
    return dynamic_cast<const LegacySceneEffectControlBindings*>(&selections);
}

std::unique_ptr<SceneVisualSelections> createLegacySceneSelectionAdapters(
    EffectControl& flame, EffectControl& generalFlame, EffectControl& wave,
    EffectControl& waveScale, EffectControl& table, EffectControl& object,
    EffectControl& translation, EffectControl& palette, EffectControl& border,
    EffectControl& flashlight, EffectControl& images,
    std::unique_ptr<SceneVisualSelections> selections) {
    return std::unique_ptr<SceneVisualSelections>(
        new LegacySceneSelectionAdapters(flame, generalFlame, wave, waveScale,
            table, object, translation, palette, border, flashlight, images,
            std::move(selections)));
}
