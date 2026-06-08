// Scene image catalog loader compatibility helpers.

#include "SceneImageCatalogLoader.h"

#include "Image.h"
#include "SceneImageCatalog.h"

void copySceneImageCatalogFromImageOption(
    ImageOption& imageOption, SceneImageCatalog& catalog) {
    catalog.clear();

    for (int i = 0; i < imageOption.getNEntries(); i++) {
        ImageEntry* entry = dynamic_cast<ImageEntry*>(imageOption[i]);
        if (entry != 0)
            catalog.addChoice(entry->Name(), entry->image(), entry->inUse());
    }
}
