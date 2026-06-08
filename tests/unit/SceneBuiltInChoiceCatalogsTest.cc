#include "SceneBuiltInChoiceCatalogs.h"

#include "SceneChoiceSelection.h"

#include <assert.h>
#include <string.h>

int cth_log_enabled(int) { return 0; }
int cth_log(int, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }

class RecordingLock : public SceneChoiceLock {
public:
    virtual int enabled() const { return 0; }
    virtual void change(const char*) { }
};

static void testFlameCatalogUsesNativeDefaultAvailability() {
    assert(sceneBuiltInFlameChoiceInUse(-1) == 0);
    assert(sceneBuiltInFlameChoiceInUse(0) == 0);
    assert(sceneBuiltInFlameChoiceInUse(1) == 1);
    assert(sceneBuiltInFlameChoiceInUse(18) == 1);
}

static void testWaveCatalogUsesNativeDefaultAvailability() {
    assert(sceneBuiltInWaveChoiceInUse(-1) == 0);
    assert(sceneBuiltInWaveChoiceInUse(0) == 1);
    assert(sceneBuiltInWaveChoiceInUse(32) == 1);
    assert(sceneBuiltInWaveChoiceInUse(33) == 0);
    assert(sceneBuiltInWaveChoiceInUse(34) == 0);
}

static void assertChoice(SceneChoiceCatalog& catalog, int index,
    const char* name) {
    SceneChoice* choice = catalog.choiceAt(index);
    assert(choice != 0);
    assert(strcmp(choice->name(), name) == 0);
    assert(choice->sameName(name));
    assert(choice->inUse() == 1);
}

static void testWaveScaleCatalogUsesNativeFixedChoices() {
    SceneChoiceCatalog* catalog = createSceneWaveScaleChoiceCatalog(
        "wave-scale", new RecordingLock());

    assert(strcmp(catalog->optionName(), "wave-scale") == 0);
    assert(catalog->entryCount() == 3);
    assertChoice(*catalog, 0, "scale0");
    assertChoice(*catalog, 1, "scale1");
    assertChoice(*catalog, 2, "scale2");

    delete catalog;
}

static void testTableCatalogUsesNativeFixedChoices() {
    SceneChoiceCatalog* catalog = createSceneTableChoiceCatalog(
        "table", new RecordingLock());

    assert(catalog->entryCount() == 10);
    assertChoice(*catalog, 0, "table0");
    assertChoice(*catalog, 9, "table9");

    delete catalog;
}

static void testBorderCatalogUsesNativeFixedChoices() {
    SceneChoiceCatalog* catalog = createSceneBorderChoiceCatalog(
        "border", new RecordingLock());

    assert(catalog->entryCount() == 4);
    assertChoice(*catalog, 0, "border0");
    assertChoice(*catalog, 3, "border3");

    delete catalog;
}

static void testFlashlightCatalogUsesNativeAliases() {
    SceneChoiceCatalog* catalog = createSceneFlashlightChoiceCatalog(
        "flashlight", new RecordingLock());

    assert(catalog->entryCount() == 2);
    assertChoice(*catalog, 0, "off");
    assertChoice(*catalog, 1, "on");
    assert(catalog->choiceAt(0)->sameName("no"));
    assert(catalog->choiceAt(0)->sameName("0"));
    assert(catalog->choiceAt(1)->sameName("yes"));
    assert(catalog->choiceAt(1)->sameName("1"));

    delete catalog;
}

int main() {
    testFlameCatalogUsesNativeDefaultAvailability();
    testWaveCatalogUsesNativeDefaultAvailability();
    testWaveScaleCatalogUsesNativeFixedChoices();
    testTableCatalogUsesNativeFixedChoices();
    testBorderCatalogUsesNativeFixedChoices();
    testFlashlightCatalogUsesNativeAliases();
    return 0;
}
