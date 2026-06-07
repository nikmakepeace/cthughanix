// Legacy EffectControl-backed Scene visual selection adapters.

#include "LegacySceneSelectionAdapters.h"

#include "EffectControl.h"
#include "Image.h"
#include "LegacySceneEffectControlSelection.h"
#include "ProcessServices.h"
#include "SceneChoiceSelection.h"
#include "TranslationOptions.h"
#include "display.h"
#include "flames.h"
#include "waves.h"

#include <cstdlib>
#include <memory>
#include <vector>

namespace {

static const int generalFlameStates = 9 * 9 * 9 * 9 * 9;

static int modInt(int value, int modulo) {
    int result = value % modulo;
    return result < 0 ? result + modulo : result;
}

class EffectControlSceneChoice : public SceneChoice {
    EffectChoice& choice;

public:
    explicit EffectControlSceneChoice(EffectChoice& choice_);

    EffectChoice& effectChoice();
    const EffectChoice& effectChoice() const;
    virtual const char* name() const;
    virtual int sameName(const char* other) const;
    virtual int inUse() const;
    virtual void setUse(int inUse);
};

class EffectControlSceneChoiceLock : public SceneChoiceLock {
    OptionOnOff& lockValue;

public:
    explicit EffectControlSceneChoiceLock(OptionOnOff& lockValue_);

    virtual int enabled() const;
    virtual void change(const char* to);
};

class EffectControlSceneChoiceCatalog : public SceneChoiceCatalog {
    EffectControl& option;
    EffectControlSceneChoiceLock lockValue;
    mutable std::vector<std::unique_ptr<EffectControlSceneChoice> > choices;

public:
    explicit EffectControlSceneChoiceCatalog(EffectControl& option_);

    virtual int entryCount() const;
    virtual SceneChoice* choiceAt(int index) const;
    virtual SceneChoiceLock& lock();
    virtual const SceneChoiceLock& lock() const;
    virtual const char* optionName() const;
};

class LegacySceneEffectControlSelection : public SceneChoiceSelection,
    public SceneEffectControlSelection {
    EffectControl& option;

protected:
    virtual void selectionChanged();
    virtual void syncSelectedValue(int value);
    EffectChoice* currentEffectChoice();
    const EffectChoice* currentEffectChoice() const;

public:
    explicit LegacySceneEffectControlSelection(EffectControl& option_);

    virtual int isOption(const EffectControl& option_) const;
    virtual void syncFromControl();
    virtual void activate(int index);
};

class LegacySceneFlameSelection : public LegacySceneEffectControlSelection,
    public SceneFlameSelection {
public:
    explicit LegacySceneFlameSelection(EffectControl& flameOption_);

    virtual const Flame* currentFlame();
};

class LegacySceneGeneralFlameSelection : public LegacySceneEffectControlSelection,
    public SceneGeneralFlameSelection {
protected:
    virtual void syncSelectedValue(int value);

public:
    explicit LegacySceneGeneralFlameSelection(
        EffectControl& generalFlameControl_);

    virtual const char* currentName() const;
    virtual int currentValue() const;
    virtual int entryCount() const;
    virtual void change(int by);
    virtual void change(
        const char* to, RandomSource& randomSource);
    virtual void setValue(int index);
    virtual int encodedValue() const;
    virtual const char* selectionText() const;
    virtual int changeRandom(RandomSource& randomSource);
};

class LegacySceneWaveSelection : public LegacySceneEffectControlSelection,
    public SceneWaveSelection {
public:
    explicit LegacySceneWaveSelection(EffectControl& waveOption_);

    virtual Wave* currentWave();
};

class LegacySceneTranslationSelection : public LegacySceneEffectControlSelection,
    public SceneTranslationSelection {
public:
    explicit LegacySceneTranslationSelection(
        EffectControl& translationOption_);

    virtual TranslationTable currentTranslationTable();
};

class LegacyScenePaletteSelection : public LegacySceneEffectControlSelection,
    public ScenePaletteSelection {
public:
    explicit LegacyScenePaletteSelection(EffectControl& paletteOption_);

    virtual PaletteEntry* currentPaletteEntry();
};

class LegacySceneImageSelection : public LegacySceneEffectControlSelection,
    public SceneImageSelection {
public:
    explicit LegacySceneImageSelection(EffectControl& imageOption_);

    virtual const IndexedImage* currentImage();
};

class LegacySceneSelectionAdapters : public SceneVisualSelections {
    LegacySceneFlameSelection flameValue;
    LegacySceneGeneralFlameSelection generalFlameValue;
    LegacySceneWaveSelection waveValue;
    LegacySceneEffectControlSelection waveScaleValue;
    LegacySceneEffectControlSelection tableValue;
    LegacySceneEffectControlSelection objectValue;
    LegacySceneTranslationSelection translationValue;
    LegacyScenePaletteSelection paletteValue;
    LegacySceneEffectControlSelection borderValue;
    LegacySceneEffectControlSelection flashlightValue;
    LegacySceneImageSelection imagesValue;

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
};

EffectControlSceneChoice::EffectControlSceneChoice(EffectChoice& choice_)
    : choice(choice_) { }

EffectChoice& EffectControlSceneChoice::effectChoice() {
    return choice;
}

const EffectChoice& EffectControlSceneChoice::effectChoice() const {
    return choice;
}

const char* EffectControlSceneChoice::name() const {
    return choice.Name();
}

int EffectControlSceneChoice::sameName(const char* other) const {
    return const_cast<EffectChoice&>(choice).sameName(other);
}

int EffectControlSceneChoice::inUse() const {
    return choice.inUse();
}

void EffectControlSceneChoice::setUse(int inUse_) {
    choice.setUse(inUse_);
}

EffectControlSceneChoiceLock::EffectControlSceneChoiceLock(
    OptionOnOff& lockValue_)
    : lockValue(lockValue_) { }

int EffectControlSceneChoiceLock::enabled() const {
    return int(lockValue);
}

void EffectControlSceneChoiceLock::change(const char* to) {
    lockValue.change(to);
}

EffectControlSceneChoiceCatalog::EffectControlSceneChoiceCatalog(
    EffectControl& option_)
    : option(option_)
    , lockValue(option.lock)
    , choices() { }

int EffectControlSceneChoiceCatalog::entryCount() const {
    return option.getNEntries();
}

SceneChoice* EffectControlSceneChoiceCatalog::choiceAt(int index) const {
    EffectChoice* choice = const_cast<EffectControl&>(option)[index];
    if (choice == 0)
        return 0;

    while (int(choices.size()) <= index)
        choices.push_back(std::unique_ptr<EffectControlSceneChoice>());

    if (choices[index].get() == 0
        || &choices[index]->effectChoice() != choice)
        choices[index].reset(new EffectControlSceneChoice(*choice));

    return choices[index].get();
}

SceneChoiceLock& EffectControlSceneChoiceCatalog::lock() {
    return lockValue;
}

const SceneChoiceLock& EffectControlSceneChoiceCatalog::lock() const {
    return lockValue;
}

const char* EffectControlSceneChoiceCatalog::optionName() const {
    return option.name();
}

LegacySceneEffectControlSelection::LegacySceneEffectControlSelection(
    EffectControl& option_)
    : SceneChoiceSelection(
          new EffectControlSceneChoiceCatalog(option_), int(option_))
    , option(option_) { }

void LegacySceneEffectControlSelection::selectionChanged() {
    option.setValue(currentValue());
}

void LegacySceneEffectControlSelection::syncSelectedValue(int value) {
    setSelectedValue(value);
}

EffectChoice* LegacySceneEffectControlSelection::currentEffectChoice() {
    EffectControlSceneChoice* choice
        = dynamic_cast<EffectControlSceneChoice*>(currentChoice());
    return (choice != 0) ? &choice->effectChoice() : 0;
}

const EffectChoice* LegacySceneEffectControlSelection::currentEffectChoice()
    const {
    const EffectControlSceneChoice* choice
        = dynamic_cast<const EffectControlSceneChoice*>(currentChoice());
    return (choice != 0) ? &choice->effectChoice() : 0;
}

int LegacySceneEffectControlSelection::isOption(
    const EffectControl& option_) const {
    return &option_ == &option;
}

void LegacySceneEffectControlSelection::syncFromControl() {
    syncSelectedValue(int(option));
}

void LegacySceneEffectControlSelection::activate(int index) {
    SceneChoiceSelection::activate(index);
}

LegacySceneFlameSelection::LegacySceneFlameSelection(
    EffectControl& flameOption_)
    : LegacySceneEffectControlSelection(flameOption_) { }

const Flame* LegacySceneFlameSelection::currentFlame() {
    FlameEntry* entry = dynamic_cast<FlameEntry*>(currentEffectChoice());
    return (entry != 0) ? &entry->flame() : 0;
}

LegacySceneGeneralFlameSelection::LegacySceneGeneralFlameSelection(
    EffectControl& generalFlameControl_)
    : LegacySceneEffectControlSelection(generalFlameControl_) { }

void LegacySceneGeneralFlameSelection::syncSelectedValue(int value) {
    setSelectedValue(modInt(value, generalFlameStates));
}

const char* LegacySceneGeneralFlameSelection::currentName() const {
    return selectionText();
}

int LegacySceneGeneralFlameSelection::currentValue() const {
    return encodedValue();
}

int LegacySceneGeneralFlameSelection::entryCount() const {
    return generalFlameStates;
}

void LegacySceneGeneralFlameSelection::change(int by) {
    setValue(modInt(encodedValue() + by, generalFlameStates));
}

void LegacySceneGeneralFlameSelection::change(
    const char* to, RandomSource& randomSource) {
    char* pos;

    if ((to == 0) || (to[0] == '\0'))
        return;

    to = applySelectionLockPrefix(to);

    int newValue = std::strtol(to, &pos, 0);
    if (pos == to) {
        CTH_WARN("Unknown entry `%s' for option `%s'\n", to,
            optionName());
        changeRandom(randomSource);
        return;
    }

    setValue(modInt(newValue, generalFlameStates));
}

void LegacySceneGeneralFlameSelection::setValue(int index) {
    setSelectedValue(modInt(index, generalFlameStates));
    selectionChanged();
}

int LegacySceneGeneralFlameSelection::encodedValue() const {
    return SceneChoiceSelection::currentValue();
}

const char* LegacySceneGeneralFlameSelection::selectionText() const {
    static char str[32];

    if (selectionLock().enabled())
        std::snprintf(str, sizeof(str), "locked:%d", encodedValue());
    else
        std::snprintf(str, sizeof(str), "%d", encodedValue());

    return str;
}

int LegacySceneGeneralFlameSelection::changeRandom(
    RandomSource& randomSource) {
    if (selectionLock().enabled())
        return 0;

    int previousValue = encodedValue();
    setValue(randomSource.uniformInt(generalFlameStates));
    return encodedValue() != previousValue;
}

LegacySceneWaveSelection::LegacySceneWaveSelection(EffectControl& waveOption_)
    : LegacySceneEffectControlSelection(waveOption_) { }

Wave* LegacySceneWaveSelection::currentWave() {
    WaveEntry* entry = dynamic_cast<WaveEntry*>(currentEffectChoice());
    return (entry != 0) ? &entry->wave() : 0;
}

LegacySceneTranslationSelection::LegacySceneTranslationSelection(
    EffectControl& translationOption_)
    : LegacySceneEffectControlSelection(translationOption_) { }

TranslationTable LegacySceneTranslationSelection::currentTranslationTable() {
    TranslateEntry* entry = dynamic_cast<TranslateEntry*>(currentEffectChoice());
    return (entry != 0) ? entry->table() : TranslationTable();
}

LegacyScenePaletteSelection::LegacyScenePaletteSelection(
    EffectControl& paletteOption_)
    : LegacySceneEffectControlSelection(paletteOption_) { }

PaletteEntry* LegacyScenePaletteSelection::currentPaletteEntry() {
    return dynamic_cast<PaletteEntry*>(currentEffectChoice());
}

LegacySceneImageSelection::LegacySceneImageSelection(EffectControl& imageOption_)
    : LegacySceneEffectControlSelection(imageOption_) { }

const IndexedImage* LegacySceneImageSelection::currentImage() {
    ImageEntry* entry = dynamic_cast<ImageEntry*>(currentEffectChoice());
    return (entry != 0) ? entry->image() : 0;
}

LegacySceneSelectionAdapters::LegacySceneSelectionAdapters(EffectControl& flame_,
    EffectControl& generalFlame_, EffectControl& wave_,
    EffectControl& waveScale_, EffectControl& table_, EffectControl& object_,
    EffectControl& translation_, EffectControl& palette_,
    EffectControl& border_, EffectControl& flashlight_, EffectControl& images_)
    : flameValue(flame_)
    , generalFlameValue(generalFlame_)
    , waveValue(wave_)
    , waveScaleValue(waveScale_)
    , tableValue(table_)
    , objectValue(object_)
    , translationValue(translation_)
    , paletteValue(palette_)
    , borderValue(border_)
    , flashlightValue(flashlight_)
    , imagesValue(images_) { }

SceneFlameSelection& LegacySceneSelectionAdapters::flame() {
    return flameValue;
}

SceneGeneralFlameSelection& LegacySceneSelectionAdapters::generalFlame() {
    return generalFlameValue;
}

SceneWaveSelection& LegacySceneSelectionAdapters::wave() {
    return waveValue;
}

SceneOptionSelection& LegacySceneSelectionAdapters::waveScale() {
    return waveScaleValue;
}

SceneOptionSelection& LegacySceneSelectionAdapters::table() {
    return tableValue;
}

SceneOptionSelection& LegacySceneSelectionAdapters::object() {
    return objectValue;
}

SceneTranslationSelection& LegacySceneSelectionAdapters::translation() {
    return translationValue;
}

ScenePaletteSelection& LegacySceneSelectionAdapters::palette() {
    return paletteValue;
}

SceneOptionSelection& LegacySceneSelectionAdapters::border() {
    return borderValue;
}

SceneOptionSelection& LegacySceneSelectionAdapters::flashlight() {
    return flashlightValue;
}

SceneImageSelection& LegacySceneSelectionAdapters::images() {
    return imagesValue;
}

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
