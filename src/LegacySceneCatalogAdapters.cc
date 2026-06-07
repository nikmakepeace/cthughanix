// Legacy global Scene catalog adapters.

#include "LegacySceneCatalogAdapters.h"

#include "display.h"
#include "waves.h"

namespace {

class LegacySceneWaveObjectSource : public SceneWaveObjectSource {
public:
    virtual WObject* currentObject() {
        return currentWaveObject();
    }
};

class LegacyScenePaletteRandomizer : public ScenePaletteRandomizer {
public:
    virtual int randomizeLast(RandomSource& randomSource) {
        PaletteEntry::randomizeLast(randomSource);
        return PaletteEntry::lastRandomPos;
    }

    virtual int addRandom(RandomSource& randomSource) {
        PaletteEntry::addRandom(randomSource);
        return PaletteEntry::lastRandomPos;
    }
};

}

SceneWaveObjectSource::~SceneWaveObjectSource() { }

ScenePaletteRandomizer::~ScenePaletteRandomizer() { }

std::unique_ptr<SceneWaveObjectSource> createLegacySceneWaveObjectSource() {
    return std::unique_ptr<SceneWaveObjectSource>(
        new LegacySceneWaveObjectSource());
}

std::unique_ptr<ScenePaletteRandomizer> createLegacyScenePaletteRandomizer() {
    return std::unique_ptr<ScenePaletteRandomizer>(
        new LegacyScenePaletteRandomizer());
}
