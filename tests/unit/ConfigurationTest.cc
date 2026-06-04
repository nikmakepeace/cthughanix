#include "Configuration.h"

#include "configuration_defaults.h"

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
    assert(result.config.logging.verbosity == LOGGING_CONFIG_DEFAULT_VERBOSITY);
    assert(result.config.app.optionsSaveEnabled == APP_CONFIG_DEFAULT_OPTIONS_SAVE_ENABLED);
    assert(result.config.app.escapeKeyEnabled == APP_CONFIG_DEFAULT_ESCAPE_KEY_ENABLED);
    assert(result.config.app.keymapFile == PATH_CONFIG_DEFAULT_KEYMAP_FILE_PATH);
    assert(result.config.paths.extraLibraryPath == PATH_CONFIG_DEFAULT_EXTRA_LIBRARY_PATH);
    assert(result.config.paths.iniFileOverride == PATH_CONFIG_DEFAULT_INI_FILE_OVERRIDE_PATH);
    assert(result.config.catalogs.doubleLoadEnabled == CATALOG_CONFIG_DEFAULT_DOUBLE_LOAD_ENABLED);
    assert(result.config.audio.inputMode == AUDIO_CONFIG_DEFAULT_INPUT_MODE);
    assert(result.config.audio.inputFile == AUDIO_CONFIG_DEFAULT_INPUT_FILE_PATH);
    assert(result.config.audio.inputLoopEnabled == AUDIO_CONFIG_DEFAULT_INPUT_LOOP_ENABLED);
    assert(result.config.audio.sampleRateHz == AUDIO_CONFIG_DEFAULT_SAMPLE_RATE_HZ);
    assert(result.config.audio.channels == AUDIO_CONFIG_DEFAULT_CHANNELS);
    assert(result.config.audio.sampleFormat == AUDIO_CONFIG_DEFAULT_FORMAT);
    assert(result.config.audio.dspMethod == AUDIO_CONFIG_DEFAULT_DSP_METHOD);
    assert(result.config.audio.dspFragments == AUDIO_CONFIG_DEFAULT_DSP_FRAGMENTS);
    assert(result.config.audio.dspFragmentSize == AUDIO_CONFIG_DEFAULT_DSP_FRAGMENT_SIZE);
    assert(result.config.audio.dspSyncEnabled == AUDIO_CONFIG_DEFAULT_DSP_SYNC_ENABLED);
    assert(result.config.audio.silentEnabled == AUDIO_CONFIG_DEFAULT_SILENT_ENABLED);
    assert(result.config.audio.minNoise == AUDIO_CONFIG_DEFAULT_MIN_NOISE);
    assert(result.config.audio.pulseLatencyMs == AUDIO_CONFIG_DEFAULT_PULSE_LATENCY_MS);
    assert(result.config.audio.pulseServer == AUDIO_CONFIG_DEFAULT_PULSE_SERVER_TEXT);
    assert(result.config.audio.outputDumpPath == AUDIO_CONFIG_DEFAULT_OUTPUT_DUMP_PATH);
    assert(result.config.audio.dspDevicePath == AUDIO_CONFIG_DEFAULT_DSP_DEVICE_PATH);
    assert(result.config.audio.mixerDevicePath == AUDIO_CONFIG_DEFAULT_MIXER_DEVICE_PATH);
    assert(result.config.display.displayMode == DISPLAY_CONFIG_DEFAULT_MODE);
    assert(!result.config.display.hasCustomDisplaySize);
    assert(result.config.display.bufferWidth == 160);
    assert(result.config.display.bufferHeight == 100);
    assert(result.config.display.maxFramesPerSecond == DISPLAY_CONFIG_DEFAULT_MAX_FRAMES_PER_SECOND);
    assert(result.config.display.showFpsEnabled == DISPLAY_CONFIG_DEFAULT_SHOW_FPS_ENABLED);
    assert(result.config.display.zoomMode == DISPLAY_CONFIG_DEFAULT_ZOOM_MODE);
    assert(result.config.display.textOnTerm == DISPLAY_CONFIG_DEFAULT_TEXT_ON_TERM);
    assert(result.config.display.ncursesEnabled == DISPLAY_CONFIG_DEFAULT_NCURSES_ENABLED);
    assert(result.config.display.screenshotFilePrefix == DISPLAY_CONFIG_DEFAULT_SCREENSHOT_FILE_PREFIX);
    assert(result.config.display.x11FontName == DISPLAY_CONFIG_DEFAULT_X11_FONT_NAME);
    assert(result.config.autoChange.quietMs == AUTO_CHANGE_CONFIG_DEFAULT_QUIET_MS);
    assert(result.config.autoChange.waitMinMs == AUTO_CHANGE_CONFIG_DEFAULT_WAIT_MIN_MS);
    assert(result.config.autoChange.waitRandomMs == AUTO_CHANGE_CONFIG_DEFAULT_WAIT_RANDOM_MS);
    assert(result.config.autoChange.waitRandomMinimumMs == AUTO_CHANGE_CONFIG_DEFAULT_WAIT_RANDOM_MIN_MS);
    assert(result.config.visual.changeMessageMs == VISUAL_CONFIG_DEFAULT_CHANGE_MESSAGE_MS);
    assert(result.config.visual.quietMessageDurationMs == VISUAL_CONFIG_DEFAULT_QUIET_MESSAGE_DURATION_MS);
    assert(result.config.visual.paletteSmoothingChance == VISUAL_CONFIG_DEFAULT_PALETTE_SMOOTHING_CHANCE);
    assert(result.config.visual.paletteSmoothSeconds == VISUAL_CONFIG_DEFAULT_PALETTE_SMOOTH_SECONDS);
    assert(result.config.visual.imageLoadingEnabled == VISUAL_CONFIG_DEFAULT_IMAGE_LOADING_ENABLED);
    assert(result.config.visual.useTranslatesEnabled == VISUAL_CONFIG_DEFAULT_USE_TRANSLATES_ENABLED);
    assert(result.config.visual.useObjectsEnabled == VISUAL_CONFIG_DEFAULT_USE_OBJECTS_ENABLED);
    assert(result.config.messages.qotdPrefetchTimeoutMs == MESSAGES_CONFIG_DEFAULT_QOTD_PREFETCH_TIMEOUT_MS);
    assert(result.config.messages.qotdServer == MESSAGES_CONFIG_DEFAULT_QOTD_SERVER_TEXT);
    assert(result.config.messages.qotdPort == MESSAGES_CONFIG_DEFAULT_QOTD_PORT_TEXT);
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
