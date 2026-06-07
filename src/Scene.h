// Current visual scene state and mutation commands.

#ifndef __SCENE_H
#define __SCENE_H

#include "Wave.h"
#include "TranslationTable.h"

#include <string>
#include <vector>

class EffectControl;
class EffectPresetCatalog;
class CthughaBuffer;
class Flame;
class FlameOption;
class GeneralFlameOption;
class ImageOption;
class IndexedImage;
class PaletteEntry;
class PaletteOption;
class RandomSource;
class SceneEffectRegistry;
class ScenePaletteRandomizer;
class SceneWaveObjectSource;
struct SceneConfig;
class TranslateOption;
class Wave;
class WaveOption;

enum SceneChange {
    SceneNoChange = 0,
    SceneFlameChanged = 1 << 0,
    SceneWaveChanged = 1 << 1,
    SceneTranslationChanged = 1 << 2,
    ScenePaletteChanged = 1 << 3,
    SceneBorderChanged = 1 << 4,
    SceneFlashlightChanged = 1 << 5,
    SceneAllChanged = 0x7fffffff
};

enum SceneCueType {
    SceneCueInjectImage,
    SceneCueInjectText
};

class SceneSettings {
public:
    const Flame* flame;
    int generalFlame;

    Wave* wave;
    WaveConfig waveConfig;

    TranslationTable translationTable;
    int translateIndex;

    PaletteEntry* palette;
    int paletteIndex;

    int borderMode;
    int flashlightEnabled;

    const char* flameName;
    const char* generalFlameName;
    const char* waveName;
    const char* waveScaleName;
    const char* tableName;
    const char* translationName;
    const char* paletteName;
    const char* objectName;
    const char* borderName;
    const char* flashlightName;

    SceneSettings();
};

class SceneCue {
public:
    SceneCueType type;
    unsigned int id;
    const IndexedImage* image;
    std::string text;
    int textFrames;
    int textInkColor;

    SceneCue();
    static SceneCue injectImage(const IndexedImage* image_);
    static SceneCue injectText(const char* text_, int frameCount, int inkColor);
};

class Scene;

class SceneCommandDependencies {
public:
    FlameOption& flame;
    GeneralFlameOption& generalFlame;
    WaveOption& wave;
    EffectControl& waveScale;
    EffectControl& table;
    EffectControl& object;
    TranslateOption& translation;
    PaletteOption& palette;
    EffectControl& border;
    EffectControl& flashlight;
    ImageOption& images;
    SceneWaveObjectSource& waveObjects;
    SceneEffectRegistry& effectRegistry;
    EffectPresetCatalog& presets;
    ScenePaletteRandomizer& paletteRandomizer;

    SceneCommandDependencies(FlameOption& flame_, GeneralFlameOption& generalFlame_,
        WaveOption& wave_, EffectControl& waveScale_, EffectControl& table_,
        EffectControl& object_, TranslateOption& translation_,
        PaletteOption& palette_, EffectControl& border_,
        EffectControl& flashlight_, ImageOption& images_,
        SceneWaveObjectSource& waveObjects_,
        SceneEffectRegistry& effectRegistry_, EffectPresetCatalog& presets_,
        ScenePaletteRandomizer& paletteRandomizer_);
};

class SceneObserver {
public:
    virtual ~SceneObserver() { }
    virtual void sceneChanged(Scene& scene, unsigned int changes) = 0;
    virtual void sceneCue(Scene& scene, const SceneCue& cue);
};

class Scene {
    SceneSettings settingsValue;
    unsigned int versionValue;
    unsigned int cueVersionValue;
    std::vector<SceneObserver*> observers;

    unsigned int compareSettings(const SceneSettings& settings) const;

public:
    Scene();

    const SceneSettings& settings() const;
    unsigned int version() const;

    void setSettings(const SceneSettings& settings, unsigned int forcedChanges = 0);
    void emitCue(SceneCue cue);
    void emitImageCue(const IndexedImage* image);
    void emitTextCue(const char* text, int frameCount, int inkColor);

    void addObserver(SceneObserver& observer);
    void removeObserver(SceneObserver& observer);
};

class SceneCommands {
    Scene& scene;
    CthughaBuffer& buffer;
    SceneCommandDependencies dependencies;
    RandomSource& randomSource;

    SceneSettings settingsFromOptions();
    Wave* selectRunnableWave(const WaveConfig& config);
    void syncFromOptions(unsigned int forcedChanges);
    void emitImageCue();
    void syncFromOptionsAndMaybeCueImage(const EffectControl& option, unsigned int forcedChanges);

public:
    /**
     * Creates a command facade over scene-related runtime option state.
     *
     * @param scene_ Scene state object to update.
     * @param buffer_ Visual buffer used to validate wave/image context.
     * @param dependencies_ Explicit scene-editing controls and support ports.
     * @param randomSource_ Application-owned source for scene randomization.
     */
    SceneCommands(Scene& scene_, CthughaBuffer& buffer_,
        const SceneCommandDependencies& dependencies_,
        RandomSource& randomSource_);

    Scene& sceneState() { return scene; }
    const Scene& sceneState() const { return scene; }

    ImageOption& imageOption() { return dependencies.images; }

    void applyStartupConfig(const SceneConfig& config);
    void initializeFromOptions();
    void refreshFromOptions(unsigned int forcedChanges = 0);

    int isSceneOption(const EffectControl& option) const;
    void change(EffectControl& option, int by, int doSave = 0);
    void change(EffectControl& option, const char* to, int doSave = 0);
    void activate(EffectControl& option, int index);

    void changeFlame(int by);
    void changeFlame(const char* to);
    void changeGeneralFlame();
    void changeWave(int by);
    void changeWave(const char* to);
    void changeWaveScale(int by);
    void changeWaveScale(const char* to);
    void changeObject(int by);
    void changeObject(const char* to);
    void changeTranslation(int by);
    void changeTranslation(const char* to);
    void changeBorder(int by);
    void changeBorder(const char* to);
    void changeFlashlight(int by);
    void changeFlashlight(const char* to);
    void changePalette(int by);
    void changePalette(const char* to);
    void randomPalette();
    void addRandomPalette();
    void changeTable(int by);
    void changeTable(const char* to);
    void changeImage(int by);
    void changeImage(const char* to);

    void changeAll();
    void changeOne();
    void restore();
    void restorePreset(int slot);
    void savePreset(int slot);
};

#endif
