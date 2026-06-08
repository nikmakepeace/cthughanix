// Native Scene choice catalogs for fixed built-in visual selections.

#include "SceneBuiltInChoiceCatalogs.h"

#include "SceneChoiceListCatalog.h"

#include <cstdio>

static SceneChoiceListCatalog* createSceneChoiceListCatalog(
    const char* catalogName, SceneChoiceLock* lock) {
    return new SceneChoiceListCatalog(catalogName, lock);
}

SceneChoiceCatalog* createSceneWaveScaleChoiceCatalog(
    const char* catalogName, SceneChoiceLock* lock) {
    static const char* names[] = { "scale0", "scale1", "scale2" };
    SceneChoiceListCatalog* catalog = createSceneChoiceListCatalog(
        catalogName, lock);

    for (unsigned int i = 0; i < sizeof(names) / sizeof(names[0]); i++)
        catalog->addChoice(names[i], 1);

    return catalog;
}

SceneChoiceCatalog* createSceneTableChoiceCatalog(
    const char* catalogName, SceneChoiceLock* lock) {
    SceneChoiceListCatalog* catalog = createSceneChoiceListCatalog(
        catalogName, lock);

    for (int i = 0; i < 10; i++) {
        char name[16];
        snprintf(name, sizeof(name), "table%d", i);
        catalog->addChoice(name, 1);
    }

    return catalog;
}

SceneChoiceCatalog* createSceneBorderChoiceCatalog(
    const char* catalogName, SceneChoiceLock* lock) {
    SceneChoiceListCatalog* catalog = createSceneChoiceListCatalog(
        catalogName, lock);

    for (int i = 0; i < 4; i++) {
        char name[16];
        snprintf(name, sizeof(name), "border%d", i);
        catalog->addChoice(name, 1);
    }

    return catalog;
}

SceneChoiceCatalog* createSceneFlashlightChoiceCatalog(
    const char* catalogName, SceneChoiceLock* lock) {
    SceneChoiceListCatalog* catalog = createSceneChoiceListCatalog(
        catalogName, lock);
    SceneChoiceListEntry& off = catalog->addChoice("off", 1);
    off.addAlias("no");
    off.addAlias("0");
    SceneChoiceListEntry& on = catalog->addChoice("on", 1);
    on.addAlias("yes");
    on.addAlias("1");
    return catalog;
}
