// Legacy EffectControl command target facade for SceneCommands.

#include "LegacySceneEffectControlTarget.h"

#include "LegacySceneEffectControlCatalog.h"
#include "Scene.h"
#include "SceneDependencies.h"

RuntimeEffectControlOwner* SceneEffectControlCatalog::createEffectControlOwner(
    SceneCommands& sceneCommands, SceneEffectRegistry& effectRegistry,
    RandomSource& randomSource) {
    return new SceneCommandsEffectControlOwner(
        sceneCommands, *this, effectRegistry, randomSource);
}

SceneCommandsEffectControlOwner::SceneCommandsEffectControlOwner(
    SceneCommands& sceneCommands_, SceneEffectControlCatalog& effectControls_,
    SceneEffectRegistry& effectRegistry_, RandomSource& randomSource_)
    : sceneCommands(sceneCommands_)
    , effectControls(effectControls_)
    , effectRegistry(effectRegistry_)
    , randomSource(randomSource_) { }

int SceneCommandsEffectControlOwner::ownsEffectControl(
    const EffectControl& option) const {
    return effectControls.isSceneOption(option);
}

void SceneCommandsEffectControlOwner::changeEffectControlBy(
    EffectControl& option, int by, int doSave) {
    SceneOptionSelection* selection = effectControls.selectionFor(option);
    if (selection == 0)
        return;

    if (doSave)
        effectRegistry.saveAll();
    sceneCommands.refreshFromOptionsAndMaybeCueImage(
        effectControls.change(*selection, by, randomSource));
}

void SceneCommandsEffectControlOwner::changeEffectControlTo(
    EffectControl& option, const char* to, int doSave) {
    SceneOptionSelection* selection = effectControls.selectionFor(option);
    if (selection == 0)
        return;

    if (doSave)
        effectRegistry.saveAll();
    sceneCommands.refreshFromOptionsAndMaybeCueImage(
        effectControls.change(*selection, to, randomSource));
}

void SceneCommandsEffectControlOwner::activateEffectControl(
    EffectControl& option, int index) {
    SceneOptionSelection* selection = effectControls.selectionFor(option);
    if (selection == 0)
        return;

    sceneCommands.refreshFromOptionsAndMaybeCueImage(
        effectControls.activate(*selection, index));
}

void SceneCommandsEffectControlOwner::toggleEffectControlLock(
    EffectControl& option) {
    SceneOptionSelection* selection = effectControls.selectionFor(option);
    if (selection != 0)
        effectControls.toggleLock(*selection);
}

void SceneCommandsEffectControlOwner::toggleEffectChoiceUse(
    EffectControl& option, int index) {
    SceneOptionSelection* selection = effectControls.selectionFor(option);
    if (selection != 0)
        effectControls.toggleChoiceUse(*selection, index);
}
