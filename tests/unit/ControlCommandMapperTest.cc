/** @file
 * Unit coverage for JSON control command mapping.
 */

#include "ControlCommandMapper.h"
#include "ControlProtocol.h"

#include <assert.h>
#include <string.h>

static ControlJsonValue parseCommand(const char* text) {
    ControlJsonValue value;
    std::string error;
    assert(parseControlJson(text, &value, &error));
    return value;
}

static void testMapsSceneSetCommand() {
    ControlMappedCommand mapped;
    std::string code;
    std::string message;

    assert(controlCommandFromJson(parseCommand(
        "{\"v\":1,\"id\":12,\"op\":\"set\",\"target\":\"scene.flame\","
        "\"value\":\"fire\"}"),
        &mapped, &code, &message));

    assert(mapped.command.type == RuntimeCommandChangeSceneTo);
    assert(mapped.command.sceneTarget == RuntimeSceneFlame);
    assert(strcmp(mapped.command.text, "fire") == 0);
    assert(mapped.textStorage == "fire");
}

static void testMapsAudioProcessingCommand() {
    ControlMappedCommand mapped;
    std::string code;
    std::string message;

    assert(controlCommandFromJson(parseCommand(
        "{\"v\":1,\"op\":\"set\",\"target\":\"audio.processing\","
        "\"value\":\"FFT\"}"),
        &mapped, &code, &message));

    assert(mapped.command.type == RuntimeCommandChangeSoundProcessingTo);
    assert(strcmp(mapped.command.text, "FFT") == 0);
}

static void testMapsMaxFpsCommand() {
    ControlMappedCommand mapped;
    std::string code;
    std::string message;

    assert(controlCommandFromJson(parseCommand(
        "{\"v\":1,\"op\":\"set\",\"target\":\"display.maxFps\","
        "\"value\":72}"),
        &mapped, &code, &message));

    assert(mapped.command.type == RuntimeCommandChangeMaxFpsTo);
    assert(mapped.command.value == 72);
}

static void testRejectsUnknownTargets() {
    ControlMappedCommand mapped;
    std::string code;
    std::string message;

    assert(!controlCommandFromJson(parseCommand(
        "{\"v\":1,\"op\":\"set\",\"target\":\"scene.nope\","
        "\"value\":\"x\"}"),
        &mapped, &code, &message));

    assert(code == "bad-target");
    assert(!message.empty());
}

static void testRejectsUnsupportedVersion() {
    ControlMappedCommand mapped;
    std::string code;
    std::string message;

    assert(!controlCommandFromJson(parseCommand(
        "{\"v\":2,\"op\":\"set\",\"target\":\"scene.flame\","
        "\"value\":\"fire\"}"),
        &mapped, &code, &message));

    assert(code == "bad-version");
}

int main() {
    testMapsSceneSetCommand();
    testMapsAudioProcessingCommand();
    testMapsMaxFpsCommand();
    testRejectsUnknownTargets();
    testRejectsUnsupportedVersion();
    return 0;
}
