#include "Configuration.h"

#include "defaults.h"

#include <cassert>
#include <map>
#include <string>
#include <vector>

static const std::string* patchValue(const ConfigPatch& patch,
    const std::string& key) {
    const std::string* value = patch.value(key);
    assert(value != NULL);
    return value;
}

static void defaultsProduceTypedConfig() {
    ConfigurationBuilder builder;
    ConfigBuildResult result = builder.addDefaults().build();

    assert(result.ok());
    assert(result.config.logging.verbosity == DEFAULT_VERBOSE_LEVEL);
    assert(result.config.paths.extraLibraryPath == DEFAULT_EXTRA_LIBRARY_PATH);
    assert(result.config.paths.iniFileOverride == DEFAULT_INI_FILE_OVERRIDE_PATH);
    assert(result.config.audio.inputMode == DEFAULT_AUDIO_INPUT_MODE);
    assert(result.config.audio.inputFile == DEFAULT_AUDIO_INPUT_FILE_PATH);
    assert(result.config.display.displayMode == DEFAULT_DISPLAY_MODE);
    assert(!result.config.display.hasCustomDisplaySize);
    assert(result.config.display.bufferWidth == 160);
    assert(result.config.display.bufferHeight == 100);
}

static void iniTextSourceProducesPatchWithoutGlobals() {
    DeferredLogBuffer diagnostics;
    IniTextConfigSource source("memory",
        "cthugha.verbose: 3\n"
        "cthugha.path: /tmp/cth\n"
        "cthugha.play: song.wav\n"
        "cthugha.disp-mode: 800x600\n"
        "cthugha.buff-size: 2\n");
    ConfigPatch patch = source.acquire(diagnostics);

    assert(diagnostics.diagnostics().empty());
    assert(*patchValue(patch, "logging.verbosity") == "3");
    assert(*patchValue(patch, "paths.extra_library") == "/tmp/cth/");
    assert(*patchValue(patch, "audio.input_mode") == "2");
    assert(*patchValue(patch, "audio.input_file") == "song.wav");
    assert(*patchValue(patch, "display.mode") == "-1");
    assert(*patchValue(patch, "display.width") == "800");
    assert(*patchValue(patch, "display.height") == "600");
    assert(*patchValue(patch, "buffer.preset") == "2");
}

static void sourcePrecedenceIsDefaultsIniEnvironmentCommandLine() {
    std::map<std::string, std::string> environment;
    environment["CTH_VERBOSE"] = "5";

    ConfigurationBuilder builder;
    ConfigBuildResult result = builder.addDefaults()
        .addIniText("memory", "cthugha.verbose: 2\n")
        .addEnvironment(environment)
        .addCommandLine(std::vector<std::string>{ "cthugha", "--verbose=7" })
        .build();

    assert(result.ok());
    assert(result.config.logging.verbosity == 7);

    ConfigurationBuilder noVerboseBuilder;
    result = noVerboseBuilder.addDefaults()
                 .addIniText("memory", "cthugha.verbose: 2\n")
                 .addEnvironment(environment)
                 .addCommandLine(
                     std::vector<std::string>{ "cthugha", "--no-verbose" })
                 .build();

    assert(result.ok());
    assert(result.config.logging.verbosity == 0);
}

static void commandLineSourceHandlesAudioLastWriterWins() {
    DeferredLogBuffer diagnostics;
    CommandLineConfigSource source(std::vector<std::string>{
        "cthugha",
        "--play",
        "song.wav",
        "--random-noise",
        "--no-sound",
    });
    ConfigPatch patch = source.acquire(diagnostics);

    assert(diagnostics.diagnostics().empty());
    assert(*patchValue(patch, "audio.input_mode") == "3");
    assert(*patchValue(patch, "audio.input_file") == "");

    ConfigurationBuilder builder;
    ConfigBuildResult result = builder.addDefaults().addSource(source).build();
    assert(result.ok());
    assert(result.config.audio.inputMode == AIM_None);
    assert(result.config.audio.inputFile.empty());
}

static void commandLineSourceHandlesDisplayAndBufferSettings() {
    ConfigurationBuilder builder;
    ConfigBuildResult result = builder.addDefaults()
        .addCommandLine(std::vector<std::string>{
            "cthugha",
            "--disp-mode",
            "800x600",
            "--buff-size",
            "2",
        })
        .build();

    assert(result.ok());
    assert(result.config.display.hasCustomDisplaySize);
    assert(result.config.display.displayMode == -1);
    assert(result.config.display.displayWidth == 800);
    assert(result.config.display.displayHeight == 600);
    assert(result.config.display.bufferWidth == 400);
    assert(result.config.display.bufferHeight == 300);
    assert(!result.config.display.hasCustomBufferSize);
}

static void customBufferSizeIsClampedWithDeferredWarnings() {
    ConfigurationBuilder builder;
    ConfigBuildResult result = builder.addDefaults()
        .addCommandLine(std::vector<std::string>{
            "cthugha",
            "--buff-size=2000x12",
        })
        .build();

    assert(result.ok());
    assert(result.config.display.bufferWidth == 1024);
    assert(result.config.display.bufferHeight == 64);
    assert(result.config.display.hasCustomBufferSize);
    assert(result.diagnostics.size() == 2);
    assert(result.diagnostics[0].severity == ConfigDiagnosticWarning);
    assert(result.diagnostics[1].severity == ConfigDiagnosticWarning);
}

static void invalidTypedValueProducesDeferredError() {
    ConfigurationBuilder builder;
    ConfigBuildResult result = builder.addDefaults()
        .addIniText("broken", "cthugha.verbose: nope\n")
        .build();

    assert(!result.ok());
    assert(!result.diagnostics.empty());
    assert(result.diagnostics[0].severity == ConfigDiagnosticError);
}

int main() {
    defaultsProduceTypedConfig();
    iniTextSourceProducesPatchWithoutGlobals();
    sourcePrecedenceIsDefaultsIniEnvironmentCommandLine();
    commandLineSourceHandlesAudioLastWriterWins();
    commandLineSourceHandlesDisplayAndBufferSettings();
    customBufferSizeIsClampedWithDeferredWarnings();
    invalidTypedValueProducesDeferredError();

    return 0;
}
