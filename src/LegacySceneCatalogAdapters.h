// Factory functions for legacy global Scene catalog adapters.

#ifndef CTHUGHA_LEGACY_SCENE_CATALOG_ADAPTERS_H
#define CTHUGHA_LEGACY_SCENE_CATALOG_ADAPTERS_H

#include "Wave.h"

#include <memory>

class RandomSource;

class SceneWaveObjectSource {
public:
    virtual ~SceneWaveObjectSource();
    virtual WObject* currentObject() = 0;
};

class ScenePaletteRandomizer {
public:
    virtual ~ScenePaletteRandomizer();
    virtual int randomizeLast(RandomSource& randomSource) = 0;
    virtual int addRandom(RandomSource& randomSource) = 0;
};

std::unique_ptr<SceneWaveObjectSource> createLegacySceneWaveObjectSource();
std::unique_ptr<ScenePaletteRandomizer> createLegacyScenePaletteRandomizer();

#endif
