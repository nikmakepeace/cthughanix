#include "SceneImageCatalogLoader.h"
#include "ScenePaletteCatalogLoader.h"
#include "SceneWaveObjectCatalogLoader.h"

#include "EffectChoiceLoader.h"
#include "Image.h"
#include "PaletteEntry.h"
#include "SceneImageCatalog.h"
#include "ScenePaletteCatalog.h"
#include "SceneWaveObjectCatalog.h"
#include "WaveObject.h"
#include "pcx.h"
#include "png.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>

int cth_log_enabled(int) { return 0; }
int cth_log(int, const char*, ...) { return 0; }
int cth_log_context(int, const char*, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }
int cth_log_errno(int, const char*, ...) { return 0; }

int loadEffectChoices(EffectControl&, const PathConfig&, const char*[],
    const char*, const char*, EffectChoiceLoader) {
    return 0;
}

int loadEffectChoices(EffectControl&, const PathConfig&, const char*[],
    const char*, const char*, EffectChoiceContextLoader, void*) {
    return 0;
}

EffectChoice* read_pcx_image(FILE*, const char*, const char*, const char*,
    const ImageLoadTarget&) {
    return 0;
}

EffectChoice* read_png_image(FILE*, const char*, const char*, const char*,
    const ImageLoadTarget&) {
    return 0;
}

static int catalogIndexByName(
    const SceneImageCatalog& catalog, const char* name) {
    for (int i = 0; i < catalog.entryCount(); i++)
        if (strcmp(catalog.nameAt(i), name) == 0)
            return i;

    return -1;
}

static int catalogIndexByName(
    const SceneWaveObjectCatalog& catalog, const char* name) {
    for (int i = 0; i < catalog.entryCount(); i++)
        if (strcmp(catalog.nameAt(i), name) == 0)
            return i;

    return -1;
}

static int catalogIndexByName(
    const ScenePaletteCatalog& catalog, const char* name) {
    for (int i = 0; i < catalog.entryCount(); i++) {
        const PaletteEntry* entry = catalog.paletteAt(i);
        if (entry != 0 && strcmp(entry->Name(), name) == 0)
            return i;
    }

    return -1;
}

static void testCopiesWaveObjectsFromEffectControl() {
    EffectChoiceList objectEntries;
    EffectControl objectOption(-1, "object-test", objectEntries);
    objectOption.add(_objects, _nObjects);

    SceneWaveObjectCatalog catalog;
    copySceneWaveObjectCatalogFromEffectControl(objectOption, catalog);

    int index = catalogIndexByName(catalog, "bigH");
    assert(index >= 0);
    assert(catalog.inUseAt(index) == 1);
    assert(catalog.objectAt(index) != 0);
}

static void testCopiesImagesFromImageOption() {
    ColorPalette* palette = new ColorPalette();
    palette->setColor(1, 10, 20, 30);
    IndexedImage* image = new IndexedImage("fixture-source", 1, 1, palette);
    image->mutablePixels()[0] = 7;

    ImageOption images(-1, "image-test");
    images.add(new ImageEntry("fixture-image", "", image));

    SceneImageCatalog catalog;
    copySceneImageCatalogFromImageOption(images, catalog);

    int index = catalogIndexByName(catalog, "fixture-image");
    assert(index >= 0);
    assert(catalog.inUseAt(index) == 1);

    const IndexedImage* copied = catalog.imageAt(index);
    assert(copied != 0);
    assert(strcmp(copied->name(), "fixture-source") == 0);
    assert(copied->pixels()[0] == 7);
    assert(copied->palette()->component(1, 0) == 10);
}

static void testCopiesPalettesFromEffectControl() {
    EffectChoiceList paletteEntries;
    EffectControl paletteOption(-1, "palette-test", paletteEntries);
    PaletteEntry* entry = new PaletteEntry("fixture-palette", "");
    entry->colors().setColor(2, 40, 50, 60);
    paletteOption.add(entry);

    ScenePaletteCatalog catalog;
    copyScenePaletteCatalogFromEffectControl(paletteOption, catalog);

    int index = catalogIndexByName(catalog, "fixture-palette");
    assert(index >= 0);
    assert(catalog.inUseAt(index) == 1);
    assert(catalog.paletteAt(index)->colors().component(2, 0) == 40);
    assert(catalog.paletteAt(index)->colors().component(2, 1) == 50);
    assert(catalog.paletteAt(index)->colors().component(2, 2) == 60);
}

int main() {
    testCopiesWaveObjectsFromEffectControl();
    testCopiesImagesFromImageOption();
    testCopiesPalettesFromEffectControl();
    return 0;
}
