/** @file
 * Builds JSON control state and catalog snapshots from app-owned runtime data.
 */

#include "ControlSnapshot.h"

#include "Configuration.h"
#include "RuntimeConfigRegistry.h"
#include "SceneChoiceSelection.h"
#include "SceneVisualSelections.h"

#include <stdio.h>

namespace {

static ControlJsonValue stringMember(const std::string& value) {
    return ControlJsonValue::stringValueOf(value);
}

static void addSceneState(ControlJsonValue& scene, const Config& config) {
    scene.set("flame", stringMember(config.scene.flame));
    scene.set("translation", stringMember(config.scene.translation));
    scene.set("image", stringMember(config.scene.image));
    scene.set("object", stringMember(config.scene.object));
    scene.set("table", stringMember(config.scene.table));
    scene.set("waveScale", stringMember(config.scene.waveScale));
    scene.set("palette", stringMember(config.scene.palette));
    scene.set("flashlight", stringMember(config.scene.flashlight));
}

static ControlJsonValue catalogEntryFor(
    const SceneOptionSelection& selection, int index) {
    const SceneChoice* choice = selection.choiceAt(index);
    const char* name = choice != 0 ? choice->name() : "";
    char fallback[32];
    snprintf(fallback, sizeof(fallback), "%d", index);
    if (name == 0 || name[0] == '\0')
        name = fallback;

    ControlJsonValue entry = ControlJsonValue::objectValueOf();
    entry.set("index", ControlJsonValue::numberValueOf(index));
    entry.set("name", ControlJsonValue::stringValueOf(name));
    entry.set("label", ControlJsonValue::stringValueOf(name));
    entry.set("inUse", ControlJsonValue::boolValueOf(
        choice == 0 ? true : choice->inUse() != 0));
    entry.set("current", ControlJsonValue::boolValueOf(
        selection.currentValue() == index));
    return entry;
}

static ControlJsonValue catalogFor(const SceneOptionSelection& selection) {
    ControlJsonValue entries = ControlJsonValue::arrayValueOf();
    int count = selection.choiceCount();
    if (count <= 0)
        count = selection.entryCount();
    for (int i = 0; i < count; i++)
        entries.append(catalogEntryFor(selection, i));
    return entries;
}

static void addCatalog(ControlJsonValue& targets, const char* name,
    const SceneOptionSelection& selection) {
    targets.set(name, catalogFor(selection));
}

static ControlJsonValue audioProcessingCatalog() {
    ControlJsonValue entries = ControlJsonValue::arrayValueOf();
    const char* names[] = { "none", "Filter1", "Filter2", "FFT" };
    for (int i = 0; i < int(sizeof(names) / sizeof(names[0])); i++) {
        ControlJsonValue entry = ControlJsonValue::objectValueOf();
        entry.set("index", ControlJsonValue::numberValueOf(i));
        entry.set("name", ControlJsonValue::stringValueOf(names[i]));
        entry.set("label", ControlJsonValue::stringValueOf(names[i]));
        entry.set("inUse", ControlJsonValue::boolValueOf(true));
        entries.append(entry);
    }
    return entries;
}

}

ControlJsonValue buildControlStateSnapshot(
    const RuntimeConfigRegistry& registry, int revision) {
    Config config = registry.currentConfig();

    ControlJsonValue scene = ControlJsonValue::objectValueOf();
    addSceneState(scene, config);

    ControlJsonValue display = ControlJsonValue::objectValueOf();
    display.set("maxFps",
        ControlJsonValue::numberValueOf(config.display.maxFramesPerSecond));
    display.set("showFps",
        ControlJsonValue::boolValueOf(config.display.showFpsEnabled != 0));
    display.set("zoom",
        ControlJsonValue::numberValueOf(config.display.zoomMode));

    ControlJsonValue audio = ControlJsonValue::objectValueOf();
    audio.set("processing",
        ControlJsonValue::stringValueOf(config.scene.audioProcessing));

    ControlJsonValue autoChange = ControlJsonValue::objectValueOf();
    autoChange.set("locked",
        ControlJsonValue::boolValueOf(config.autoChange.locked != 0));
    autoChange.set("changeLittle",
        ControlJsonValue::boolValueOf(config.autoChange.changeLittle != 0));

    ControlJsonValue state = ControlJsonValue::objectValueOf();
    state.set("v", ControlJsonValue::numberValueOf(1));
    state.set("type", ControlJsonValue::stringValueOf("state"));
    state.set("rev", ControlJsonValue::numberValueOf(revision));
    state.set("scene", scene);
    state.set("display", display);
    state.set("audio", audio);
    state.set("autoChange", autoChange);
    return state;
}

ControlJsonValue buildControlCatalogSnapshot(
    SceneVisualSelections& selections) {
    ControlJsonValue targets = ControlJsonValue::objectValueOf();
    addCatalog(targets, "scene.flame", selections.flame());
    addCatalog(targets, "scene.translation", selections.translation());
    addCatalog(targets, "scene.image", selections.images());
    addCatalog(targets, "scene.object", selections.object());
    addCatalog(targets, "scene.table", selections.table());
    addCatalog(targets, "scene.waveScale", selections.waveScale());
    addCatalog(targets, "scene.palette", selections.palette());
    addCatalog(targets, "scene.flashlight", selections.flashlight());
    targets.set("audio.processing", audioProcessingCatalog());

    ControlJsonValue message = ControlJsonValue::objectValueOf();
    message.set("v", ControlJsonValue::numberValueOf(1));
    message.set("type", ControlJsonValue::stringValueOf("catalogs"));
    message.set("targets", targets);
    return message;
}
