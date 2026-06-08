// Legacy loader adapter for native Scene wave-object catalogs.

#include "LegacySceneWaveObjectCatalogAdapter.h"

#include "EffectControl.h"
#include "SceneWaveObjectCatalog.h"
#include "WaveObject.h"

void loadSceneWaveObjectCatalogFromLegacy(
    EffectControl& objectOption, SceneWaveObjectCatalog& catalog) {
    catalog.clear();

    for (int i = 0; i < objectOption.getNEntries(); i++) {
        EffectChoice* choice = objectOption[i];
        if (choice != 0)
            catalog.addChoice(choice->Name(), waveObjectEntryObject(choice),
                choice->inUse());
    }
}
