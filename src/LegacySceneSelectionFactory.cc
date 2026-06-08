// Legacy visual selection construction over global visual options.

#include "LegacySceneSelectionAdapters.h"

#include "EffectControl.h"
#include "Image.h"
#include "LegacySceneChoiceLock.h"
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

#include <cstdio>

namespace {

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

}

std::unique_ptr<SceneVisualSelections> createLegacySceneSelectionAdapters(
    EffectControl& flame, EffectControl& generalFlame, EffectControl& wave,
    EffectControl& waveScale, EffectControl& table, EffectControl& object,
    EffectControl& translation, EffectControl& palette, EffectControl& border,
    EffectControl& flashlight, EffectControl& images) {
    return createLegacySceneSelectionAdapters(flame, generalFlame, wave,
        waveScale, table, object, translation, palette, border, flashlight,
        images,
        std::unique_ptr<SceneVisualSelections>(new SceneVisualSelectionSet(
            new SceneFlameChoiceSelection(
                createSceneFlameChoiceCatalog(flame), int(flame)),
            new SceneGeneralFlameSelectionValue(generalFlame.name(),
                new LegacySceneChoiceLock(generalFlame.lock),
                int(generalFlame)),
            new SceneWaveChoiceSelection(
                createSceneWaveChoiceCatalog(wave), int(wave)),
            new SceneChoiceSelection(
                createWaveScaleChoiceCatalog(waveScale), int(waveScale)),
            new SceneChoiceSelection(createTableChoiceCatalog(table),
                int(table)),
            new SceneWaveObjectChoiceSelection(
                createSceneWaveObjectChoiceCatalog(object), int(object)),
            new SceneTranslationChoiceSelection(
                createSceneTranslationChoiceCatalog(translation),
                int(translation)),
            new ScenePaletteChoiceSelection(
                createScenePaletteChoiceCatalog(palette), int(palette)),
            new SceneChoiceSelection(createBorderChoiceCatalog(border),
                int(border)),
            new SceneChoiceSelection(
                createFlashlightChoiceCatalog(flashlight), int(flashlight)),
            new SceneImageChoiceSelection(
                createSceneImageChoiceCatalog(images), int(images)))));
}
