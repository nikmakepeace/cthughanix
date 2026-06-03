#include "EffectControl.h"
#include "EffectPresetCatalog.h"

#include <assert.h>
#include <string.h>

int cth_log_enabled(int) {
    return 0;
}

int cth_log(int, const char*, ...) {
    return 0;
}

int cth_log_context(int, const char*, const char*, ...) {
    return 0;
}

int cth_log_error(const char*, ...) {
    return 0;
}

int cth_log_errno(int, const char*, ...) {
    return 0;
}

static void testPresetSlotsOwnWholeEffectSnapshots() {
    EffectChoice displayUp("up", "");
    EffectChoice displayDown("down", "");
    EffectChoice displayMirror("mirror", "");
    EffectChoice* displayEntries[] = { &displayUp, &displayDown, &displayMirror };
    EffectChoiceList displayList(displayEntries, 3);
    EffectControl display(-1, "display", displayList);

    EffectChoice paletteRed("red", "");
    EffectChoice paletteBlue("blue", "");
    EffectChoice paletteGreen("green", "");
    EffectChoice* paletteEntries[] = { &paletteRed, &paletteBlue, &paletteGreen };
    EffectChoiceList paletteList(paletteEntries, 3);
    EffectControl palette(-1, "palette", paletteList);

    EffectPresetCatalog catalog;

    display.change("down", 0);
    palette.change("blue", 0);
    catalog.save(3);

    display.change("up", 0);
    palette.change("red", 0);
    catalog.restore(3);
    assert(strcmp(display.currentName(), "down") == 0);
    assert(strcmp(palette.currentName(), "blue") == 0);

    catalog.setValue(4, display, display.optNr("mirror"));
    catalog.setValue(4, palette, palette.optNr("green"));
    catalog.restore(4);
    assert(strcmp(display.currentName(), "mirror") == 0);
    assert(strcmp(palette.currentName(), "green") == 0);

    catalog.restore(5);
    assert(strcmp(display.currentName(), "up") == 0);
    assert(strcmp(palette.currentName(), "red") == 0);
}

int main() {
    testPresetSlotsOwnWholeEffectSnapshots();
    return 0;
}
