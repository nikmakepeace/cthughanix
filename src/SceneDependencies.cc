// Explicit Scene dependency adapters over legacy visual catalogs.

#include "SceneDependencies.h"

#include "EffectControl.h"
#include "ProcessServices.h"
#include "display.h"
#include "waves.h"

SceneWaveObjectSource::~SceneWaveObjectSource() { }

SceneEffectRegistry::~SceneEffectRegistry() { }

ScenePaletteRandomizer::~ScenePaletteRandomizer() { }

WObject* LegacySceneWaveObjectSource::currentObject() {
    return currentWaveObject();
}

void LegacySceneEffectRegistry::saveAll() {
    EffectControl::save();
}

void LegacySceneEffectRegistry::restoreAll() {
    EffectControl::restore();
}

void LegacySceneEffectRegistry::changeAll(RandomSource& randomSource) {
    EffectControl::changeAll(randomSource);
}

EffectControl* LegacySceneEffectRegistry::changeOne(RandomSource& randomSource) {
    return EffectControl::changeOne(randomSource);
}

int LegacyScenePaletteRandomizer::randomizeLast(RandomSource& randomSource) {
    PaletteEntry::randomizeLast(randomSource);
    return PaletteEntry::lastRandomPos;
}

int LegacyScenePaletteRandomizer::addRandom(RandomSource& randomSource) {
    PaletteEntry::addRandom(randomSource);
    return PaletteEntry::lastRandomPos;
}
