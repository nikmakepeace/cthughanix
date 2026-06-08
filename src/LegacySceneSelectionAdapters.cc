// Legacy EffectControl-backed Scene visual selection adapters.

#include "LegacySceneSelectionAdapters.h"

#include "EffectControl.h"
#include "Image.h"
#include "LegacySceneChoiceLock.h"
#include "LegacySceneEffectControlBindings.h"
#include "SceneChoiceListCatalog.h"
#include "SceneChoiceSelection.h"
#include "SceneGeneralFlameSelectionValue.h"
#include "SceneTypedVisualCatalogs.h"
#include "SceneVisualSelectionSet.h"
#include "TranslationOptions.h"
#include "WaveObject.h"
#include "display.h"
#include "flames.h"
#include "waves.h"

#include <memory>

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
        EffectControl& flashlight_, EffectControl& images_);

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

static int legacyChoiceInUse(EffectControl& option, int index, int defaultUse) {
    EffectChoice* choice = option[index];
    return (choice != 0) ? choice->inUse() : defaultUse;
}

static SceneChoiceListCatalog* createSceneChoiceListCatalog(
    EffectControl& option) {
    return new SceneChoiceListCatalog(
        option.name(), new LegacySceneChoiceLock(option.lock));
}

static SceneChoiceCatalog* createWaveScaleChoiceCatalog(
    EffectControl& option) {
    static const char* names[] = { "scale0", "scale1", "scale2" };
    SceneChoiceListCatalog* catalog = createSceneChoiceListCatalog(option);

    for (unsigned int i = 0; i < sizeof(names) / sizeof(names[0]); i++)
        catalog->addChoice(names[i], legacyChoiceInUse(option, int(i), 1));

    return catalog;
}

static SceneChoiceCatalog* createTableChoiceCatalog(EffectControl& option) {
    SceneChoiceListCatalog* catalog = createSceneChoiceListCatalog(option);

    for (int i = 0; i < 10; i++) {
        char name[16];
        snprintf(name, sizeof(name), "table%d", i);
        catalog->addChoice(name, legacyChoiceInUse(option, i, 1));
    }

    return catalog;
}

static SceneChoiceCatalog* createBorderChoiceCatalog(EffectControl& option) {
    SceneChoiceListCatalog* catalog = createSceneChoiceListCatalog(option);

    for (int i = 0; i < 4; i++) {
        char name[16];
        snprintf(name, sizeof(name), "border%d", i);
        catalog->addChoice(name, legacyChoiceInUse(option, i, 1));
    }

    return catalog;
}

static SceneChoiceCatalog* createFlashlightChoiceCatalog(
    EffectControl& option) {
    SceneChoiceListCatalog* catalog = createSceneChoiceListCatalog(option);
    SceneChoiceListEntry& off = catalog->addChoice(
        "off", legacyChoiceInUse(option, 0, 1));
    off.addAlias("no");
    off.addAlias("0");
    SceneChoiceListEntry& on = catalog->addChoice(
        "on", legacyChoiceInUse(option, 1, 1));
    on.addAlias("yes");
    on.addAlias("1");
    return catalog;
}

static SceneChoiceCatalog* createSceneFlameChoiceCatalog(
    EffectControl& option) {
    SceneFlameChoiceCatalog* catalog = new SceneFlameChoiceCatalog(
        option.name(), new LegacySceneChoiceLock(option.lock));

    for (int i = 0; i < nFlameCatalogEntries; i++) {
        const Flame* flame = flameByIndex(i);
        if (flame != 0)
            catalog->addChoice(
                flame, flame->name(), legacyChoiceInUse(option, i, i != 0));
    }

    return catalog;
}

static SceneChoiceCatalog* createSceneWaveChoiceCatalog(
    EffectControl& option) {
    SceneWaveChoiceCatalog* catalog = new SceneWaveChoiceCatalog(
        option.name(), new LegacySceneChoiceLock(option.lock));

    for (int i = 0; i < nWaveCatalogEntries; i++) {
        Wave* wave = waveByIndex(i);
        if (wave != 0)
            catalog->addChoice(
                wave, wave->name(), legacyChoiceInUse(option, i, i < 33));
    }

    return catalog;
}

static SceneChoiceCatalog* createSceneWaveObjectChoiceCatalog(
    EffectControl& option) {
    SceneWaveObjectChoiceCatalog* catalog = new SceneWaveObjectChoiceCatalog(
        option.name(), new LegacySceneChoiceLock(option.lock));

    for (int i = 0; i < option.getNEntries(); i++) {
        EffectChoice* choice = option[i];
        if (choice != 0)
            catalog->addChoice(choice->Name(), waveObjectEntryObject(choice),
                choice->inUse());
    }

    return catalog;
}

static SceneChoiceCatalog* createSceneTranslationChoiceCatalog(
    EffectControl& option) {
    SceneTranslationChoiceCatalog* catalog = new SceneTranslationChoiceCatalog(
        option.name(), new LegacySceneChoiceLock(option.lock));
    TranslateOption* translateOption = dynamic_cast<TranslateOption*>(&option);

    for (int i = 0; i < option.getNEntries(); i++) {
        EffectChoice* choice = option[i];
        if (choice != 0) {
            TranslationTable table = (translateOption != 0)
                ? translateOption->translationTable(i)
                : TranslationTable(choice->Name(), 0, 0, 0);
            catalog->addChoice(table, choice->inUse());
        }
    }

    return catalog;
}

static SceneChoiceCatalog* createScenePaletteChoiceCatalog(
    EffectControl& option) {
    ScenePaletteChoiceCatalog* catalog = new ScenePaletteChoiceCatalog(
        option.name(), new LegacySceneChoiceLock(option.lock));

    for (int i = 0; i < option.getNEntries(); i++) {
        PaletteEntry* entry = dynamic_cast<PaletteEntry*>(option[i]);
        if (entry != 0)
            catalog->addChoice(*entry, entry->inUse());
    }

    return catalog;
}

static SceneChoiceCatalog* createSceneImageChoiceCatalog(EffectControl& option) {
    SceneImageChoiceCatalog* catalog = new SceneImageChoiceCatalog(
        option.name(), new LegacySceneChoiceLock(option.lock));

    for (int i = 0; i < option.getNEntries(); i++) {
        ImageEntry* entry = dynamic_cast<ImageEntry*>(option[i]);
        if (entry != 0)
            catalog->addChoice(entry->Name(), entry->image(),
                entry->inUse());
    }

    return catalog;
}

LegacySceneSelectionAdapters::LegacySceneSelectionAdapters(EffectControl& flame_,
    EffectControl& generalFlame_, EffectControl& wave_,
    EffectControl& waveScale_, EffectControl& table_, EffectControl& object_,
    EffectControl& translation_, EffectControl& palette_,
    EffectControl& border_, EffectControl& flashlight_, EffectControl& images_)
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
    , selections(new SceneVisualSelectionSet(
          new SceneFlameChoiceSelection(
              createSceneFlameChoiceCatalog(flame_), int(flame_)),
          new SceneGeneralFlameSelectionValue(generalFlame_.name(),
              new LegacySceneChoiceLock(generalFlame_.lock),
              int(generalFlame_)),
          new SceneWaveChoiceSelection(
              createSceneWaveChoiceCatalog(wave_), int(wave_)),
          new SceneChoiceSelection(
              createWaveScaleChoiceCatalog(waveScale_), int(waveScale_)),
          new SceneChoiceSelection(createTableChoiceCatalog(table_),
              int(table_)),
          new SceneWaveObjectChoiceSelection(
              createSceneWaveObjectChoiceCatalog(object_), int(object_)),
          new SceneTranslationChoiceSelection(
              createSceneTranslationChoiceCatalog(translation_),
              int(translation_)),
          new ScenePaletteChoiceSelection(
              createScenePaletteChoiceCatalog(palette_), int(palette_)),
          new SceneChoiceSelection(
              createBorderChoiceCatalog(border_), int(border_)),
          new SceneChoiceSelection(
              createFlashlightChoiceCatalog(flashlight_), int(flashlight_)),
          new SceneImageChoiceSelection(
              createSceneImageChoiceCatalog(images_), int(images_)))) { }

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
    EffectControl& flashlight, EffectControl& images) {
    return std::unique_ptr<SceneVisualSelections>(
        new LegacySceneSelectionAdapters(flame, generalFlame, wave, waveScale,
            table, object, translation, palette, border, flashlight, images));
}
