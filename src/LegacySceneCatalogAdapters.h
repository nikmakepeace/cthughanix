// Factory functions for legacy global Scene catalog adapters.

#ifndef CTHUGHA_LEGACY_SCENE_CATALOG_ADAPTERS_H
#define CTHUGHA_LEGACY_SCENE_CATALOG_ADAPTERS_H

#include "ScenePaletteRandomizer.h"

#include <memory>

std::unique_ptr<ScenePaletteRandomizer> createLegacyScenePaletteRandomizer();

#endif
