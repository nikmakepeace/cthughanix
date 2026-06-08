// Scene palette catalog loader compatibility helpers.

#include "ScenePaletteCatalogLoader.h"

#include "EffectControl.h"
#include "PaletteEntry.h"
#include "ScenePaletteCatalog.h"

void copyScenePaletteCatalogFromEffectControl(
    EffectControl& paletteOption, ScenePaletteCatalog& catalog) {
    catalog.clear();

    for (int i = 0; i < paletteOption.getNEntries(); i++) {
        PaletteEntry* entry = dynamic_cast<PaletteEntry*>(paletteOption[i]);
        if (entry != 0)
            catalog.addChoice(*entry, entry->inUse());
    }
}
