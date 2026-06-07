#include "cthugha.h"
#include "Scene.h"
#include "SceneDependencies.h"
#include "EffectPresetCatalog.h"
#include "Border.h"
#include "Configuration.h"
#include "CthughaBuffer.h"
#include "Flashlight.h"
#include "Image.h"
#include "ProcessServices.h"
#include "Screen.h"
#include "display.h"
#include "flames.h"
#include "TranslationOptions.h"
#include "waves.h"

#include <unistd.h>

SceneSettings::SceneSettings()
    : flame(0)
    , generalFlame(0)
    , wave(0)
    , waveConfig()
    , translationTable()
    , translateIndex(0)
    , palette(0)
    , paletteIndex(0)
    , borderMode(0)
    , flashlightEnabled(0)
    , flameName("unknown")
    , generalFlameName("unknown")
    , waveName("unknown")
    , waveScaleName("unknown")
    , tableName("unknown")
    , translationName("unknown")
    , paletteName("unknown")
    , objectName("unknown")
    , borderName("unknown")
    , flashlightName("unknown") { }

SceneCue::SceneCue()
    : type(SceneCueInjectImage)
    , id(0)
    , image(0)
    , text()
    , textFrames(0)
    , textInkColor(-1) { }

SceneCue SceneCue::injectImage(const IndexedImage* image_) {
    SceneCue cue;
    cue.type = SceneCueInjectImage;
    cue.image = image_;
    return cue;
}

SceneCue SceneCue::injectText(const char* text_, int frameCount, int inkColor) {
    SceneCue cue;
    cue.type = SceneCueInjectText;
    if (text_ != 0)
        cue.text = text_;
    cue.textFrames = frameCount;
    cue.textInkColor = inkColor;
    return cue;
}

void SceneObserver::sceneCue(Scene& scene, const SceneCue& cue) {
    (void)scene;
    (void)cue;
}

Scene::Scene()
    : settingsValue()
    , versionValue(0)
    , cueVersionValue(0) { }

const SceneSettings& Scene::settings() const {
    return settingsValue;
}

unsigned int Scene::version() const {
    return versionValue;
}

unsigned int Scene::compareSettings(const SceneSettings& settings) const {
    unsigned int changes = SceneNoChange;

    if ((settingsValue.flame != settings.flame)
        || (settingsValue.generalFlame != settings.generalFlame))
        changes |= SceneFlameChanged;

    if ((settingsValue.wave != settings.wave)
        || !settingsValue.waveConfig.sameAs(settings.waveConfig))
        changes |= SceneWaveChanged;

    if (!settingsValue.translationTable.sameTable(settings.translationTable)
        || (settingsValue.translateIndex != settings.translateIndex))
        changes |= SceneTranslationChanged;

    if ((settingsValue.palette != settings.palette)
        || (settingsValue.paletteIndex != settings.paletteIndex))
        changes |= ScenePaletteChanged;

    if (settingsValue.borderMode != settings.borderMode)
        changes |= SceneBorderChanged;

    if (settingsValue.flashlightEnabled != settings.flashlightEnabled)
        changes |= SceneFlashlightChanged;

    return changes;
}

void Scene::setSettings(const SceneSettings& settings, unsigned int forcedChanges) {
    unsigned int changes = compareSettings(settings) | forcedChanges;
    if (changes == SceneNoChange)
        return;

    settingsValue = settings;
    versionValue++;

    std::vector<SceneObserver*> snapshot = observers;
    for (unsigned int i = 0; i < snapshot.size(); i++)
        snapshot[i]->sceneChanged(*this, changes);
}

void Scene::emitCue(SceneCue cue) {
    cue.id = ++cueVersionValue;

    std::vector<SceneObserver*> snapshot = observers;
    for (unsigned int i = 0; i < snapshot.size(); i++)
        snapshot[i]->sceneCue(*this, cue);
}

void Scene::emitImageCue(const IndexedImage* image) {
    if (image != 0)
        emitCue(SceneCue::injectImage(image));
}

void Scene::emitTextCue(const char* text, int frameCount, int inkColor) {
    if (text != 0 && text[0] != '\0' && frameCount > 0)
        emitCue(SceneCue::injectText(text, frameCount, inkColor));
}

void Scene::addObserver(SceneObserver& observer) {
    for (unsigned int i = 0; i < observers.size(); i++)
        if (observers[i] == &observer)
            return;
    observers.push_back(&observer);
}

void Scene::removeObserver(SceneObserver& observer) {
    for (std::vector<SceneObserver*>::iterator it = observers.begin(); it != observers.end();
         ++it) {
        if (*it == &observer) {
            observers.erase(it);
            return;
        }
    }
}

SceneCommandDependencies::SceneCommandDependencies(FlameOption& flame_,
    GeneralFlameOption& generalFlame_, WaveOption& wave_,
    EffectControl& waveScale_, EffectControl& table_, EffectControl& object_,
    TranslateOption& translation_, PaletteOption& palette_,
    EffectControl& border_, EffectControl& flashlight_, ImageOption& images_,
    SceneWaveObjectSource& waveObjects_,
    SceneEffectRegistry& effectRegistry_, EffectPresetCatalog& presets_,
    ScenePaletteRandomizer& paletteRandomizer_)
    : flame(flame_)
    , generalFlame(generalFlame_)
    , wave(wave_)
    , waveScale(waveScale_)
    , table(table_)
    , object(object_)
    , translation(translation_)
    , palette(palette_)
    , border(border_)
    , flashlight(flashlight_)
    , images(images_)
    , waveObjects(waveObjects_)
    , effectRegistry(effectRegistry_)
    , presets(presets_)
    , paletteRandomizer(paletteRandomizer_) { }

SceneCommands::SceneCommands(Scene& scene_, CthughaBuffer& buffer_,
    const SceneCommandDependencies& dependencies_, RandomSource& randomSource_)
    : scene(scene_)
    , buffer(buffer_)
    , dependencies(dependencies_)
    , randomSource(randomSource_) { }

Wave* SceneCommands::selectRunnableWave(const WaveConfig& config) {
    int nEntries = dependencies.wave.getNEntries();

    for (int i = 0; i < nEntries; i++) {
        Wave* selectedWave = dependencies.wave.currentWave();
        if (selectedWave == 0 || selectedWave->canRun(config))
            return selectedWave;

        dependencies.wave.change(+1, 0);
    }

    return 0;
}

SceneSettings SceneCommands::settingsFromOptions() {
    SceneSettings settings;

    settings.flame = dependencies.flame.currentFlame();
    settings.generalFlame = int(dependencies.generalFlame);
    settings.flameName = (settings.flame != 0) ? settings.flame->name() : "unknown";
    settings.generalFlameName = dependencies.generalFlame.text();

    settings.waveConfig = WaveConfig(int(dependencies.waveScale),
        int(dependencies.table), dependencies.waveObjects.currentObject(),
        buffer.width(), buffer.height());
    settings.wave = selectRunnableWave(settings.waveConfig);
    settings.waveName = (settings.wave != 0) ? settings.wave->name() : "unknown";
    settings.waveScaleName = dependencies.waveScale.currentName();
    settings.tableName = dependencies.table.currentName();
    settings.objectName = dependencies.object.currentName();

    settings.translationTable = dependencies.translation.currentTranslationTable();
    settings.translateIndex = dependencies.translation.currentN();
    settings.translationName = dependencies.translation.currentName();

    settings.palette = dependencies.palette.currentPaletteEntry();
    settings.paletteIndex = dependencies.palette.currentN();
    settings.paletteName = dependencies.palette.currentName();

    settings.borderMode = int(dependencies.border);
    settings.flashlightEnabled = int(dependencies.flashlight) != 0;
    settings.borderName = dependencies.border.currentName();
    settings.flashlightName = dependencies.flashlight.currentName();

    return settings;
}

void SceneCommands::syncFromOptions(unsigned int forcedChanges) {
    scene.setSettings(settingsFromOptions(), forcedChanges);
}

void SceneCommands::emitImageCue() {
    scene.emitImageCue(dependencies.images.currentImage());
}

void SceneCommands::syncFromOptionsAndMaybeCueImage(
    const EffectControl& option, unsigned int forcedChanges) {
    syncFromOptions(forcedChanges);
    if (&option == &dependencies.images)
        emitImageCue();
}

static void applyStartupChoice(EffectControl& option, const std::string& choice,
    RandomSource& randomSource) {
    option.change(choice.c_str(), randomSource, 0);
}

void SceneCommands::applyStartupConfig(const SceneConfig& config) {
    applyStartupChoice(dependencies.waveScale, config.waveScale, randomSource);
    applyStartupChoice(dependencies.table, config.table, randomSource);
    applyStartupChoice(dependencies.object, config.object, randomSource);
    applyStartupChoice(dependencies.wave, config.wave, randomSource);
    applyStartupChoice(dependencies.flame, config.flame, randomSource);
    applyStartupChoice(dependencies.generalFlame, config.generalFlame,
        randomSource);
    applyStartupChoice(dependencies.translation, config.translation, randomSource);
    applyStartupChoice(dependencies.palette, config.palette, randomSource);
    applyStartupChoice(dependencies.border, config.border, randomSource);
    applyStartupChoice(dependencies.flashlight, config.flashlight, randomSource);
    applyStartupChoice(dependencies.images, config.image, randomSource);

    initializeFromOptions();
}

void SceneCommands::initializeFromOptions() {
    syncFromOptions(SceneAllChanged);
    emitImageCue();
}

void SceneCommands::refreshFromOptions(unsigned int forcedChanges) {
    syncFromOptions(forcedChanges);
}

int SceneCommands::isSceneOption(const EffectControl& option) const {
    return (&option == &dependencies.flame)
        || (&option == &dependencies.generalFlame)
        || (&option == &dependencies.wave)
        || (&option == &dependencies.waveScale)
        || (&option == &dependencies.object)
        || (&option == &dependencies.translation)
        || (&option == &dependencies.border)
        || (&option == &dependencies.flashlight)
        || (&option == &dependencies.palette)
        || (&option == &dependencies.table)
        || (&option == &dependencies.images);
}

void SceneCommands::change(EffectControl& option, int by, int doSave) {
    option.change(by, doSave);
    syncFromOptionsAndMaybeCueImage(option, SceneNoChange);
}

void SceneCommands::change(EffectControl& option, const char* to, int doSave) {
    option.change(to, randomSource, doSave);
    syncFromOptionsAndMaybeCueImage(option, SceneNoChange);
}

void SceneCommands::activate(EffectControl& option, int index) {
    if ((index < 0) || (index >= option.getNEntries()))
        return;

    option[index]->setUse(1);
    option.setValue(index);
    option.change(0, 0);
    syncFromOptionsAndMaybeCueImage(option, SceneNoChange);
}

void SceneCommands::changeFlame(int by) { change(dependencies.flame, by, 0); }
void SceneCommands::changeFlame(const char* to) { change(dependencies.flame, to, 0); }

void SceneCommands::changeGeneralFlame() {
    dependencies.generalFlame.changeRandom(randomSource);
    syncFromOptions(SceneFlameChanged);
}

void SceneCommands::changeWave(int by) { change(dependencies.wave, by, 0); }
void SceneCommands::changeWave(const char* to) { change(dependencies.wave, to, 0); }
void SceneCommands::changeWaveScale(int by) { change(dependencies.waveScale, by, 0); }
void SceneCommands::changeWaveScale(const char* to) { change(dependencies.waveScale, to, 0); }
void SceneCommands::changeObject(int by) { change(dependencies.object, by, 0); }
void SceneCommands::changeObject(const char* to) { change(dependencies.object, to, 0); }
void SceneCommands::changeTranslation(int by) { change(dependencies.translation, by, 0); }
void SceneCommands::changeTranslation(const char* to) { change(dependencies.translation, to, 0); }
void SceneCommands::changeBorder(int by) { change(dependencies.border, by, 0); }
void SceneCommands::changeBorder(const char* to) { change(dependencies.border, to, 0); }
void SceneCommands::changeFlashlight(int by) { change(dependencies.flashlight, by, 0); }
void SceneCommands::changeFlashlight(const char* to) { change(dependencies.flashlight, to, 0); }
void SceneCommands::changePalette(int by) { change(dependencies.palette, by, 0); }
void SceneCommands::changePalette(const char* to) { change(dependencies.palette, to, 0); }

void SceneCommands::randomPalette() {
    dependencies.palette.setValue(
        dependencies.paletteRandomizer.randomizeLast(randomSource));
    syncFromOptions(ScenePaletteChanged);
}

void SceneCommands::addRandomPalette() {
    dependencies.palette.setValue(
        dependencies.paletteRandomizer.addRandom(randomSource));
    syncFromOptions(ScenePaletteChanged);
}

void SceneCommands::changeTable(int by) { change(dependencies.table, by, 0); }
void SceneCommands::changeTable(const char* to) { change(dependencies.table, to, 0); }
void SceneCommands::changeImage(int by) { change(dependencies.images, by, 0); }
void SceneCommands::changeImage(const char* to) { change(dependencies.images, to, 0); }

void SceneCommands::changeAll() {
    dependencies.effectRegistry.changeAll(randomSource);
    syncFromOptions(SceneAllChanged);
    emitImageCue();
}

void SceneCommands::changeOne() {
    EffectControl* changedOption = dependencies.effectRegistry.changeOne(randomSource);
    syncFromOptions(SceneNoChange);
    if (changedOption == &dependencies.images)
        emitImageCue();
}

void SceneCommands::restore() {
    dependencies.effectRegistry.restoreAll();
    syncFromOptions(SceneAllChanged);
    emitImageCue();
}

void SceneCommands::restorePreset(int slot) {
    dependencies.presets.restore(slot);
    syncFromOptions(SceneAllChanged);
    emitImageCue();
}

void SceneCommands::savePreset(int slot) {
    dependencies.presets.save(slot);
}
