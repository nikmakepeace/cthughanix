// Explicit startup configuration acquisition and typed config slices.

#include "Configuration.h"

#include "cth_buffer.h"
#include "defaults.h"

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>

#ifndef CTH_LIBDIR
#define CTH_LIBDIR ""
#endif

namespace {

static const char* KEY_LOGGING_VERBOSITY = "logging.verbosity";
static const char* KEY_PATH_EXTRA_LIBRARY = "paths.extra_library";
static const char* KEY_PATH_INI_OVERRIDE = "paths.ini_file_override";
static const char* KEY_AUDIO_INPUT_MODE = "audio.input_mode";
static const char* KEY_AUDIO_INPUT_FILE = "audio.input_file";
static const char* KEY_DISPLAY_MODE = "display.mode";
static const char* KEY_DISPLAY_WIDTH = "display.width";
static const char* KEY_DISPLAY_HEIGHT = "display.height";
static const char* KEY_BUFFER_PRESET = "buffer.preset";
static const char* KEY_BUFFER_WIDTH = "buffer.width";
static const char* KEY_BUFFER_HEIGHT = "buffer.height";
static const char* KEY_BUFFER_CUSTOM_SIZE = "buffer.custom_size";

struct ConfigSize {
    int width;
    int height;
};

static const ConfigSize screenSizePresets[] = {
    { 320, 200 },
    { 640, 480 },
    { 800, 600 },
    { 1024, 768 },
    { 1152, 864 },
    { 1280, 1024 }
};

static const ConfigSize bufferSizePresets[] = {
    { 160, 100 },
    { 320, 240 },
    { 400, 300 },
    { 512, 384 },
    { 576, 450 },
    { 600, 512 }
};

static std::string trim(const std::string& value) {
    std::string::size_type begin = 0;
    std::string::size_type end = value.size();

    while (begin < end
        && std::isspace(static_cast<unsigned char>(value[begin])))
        begin++;
    while (end > begin
        && std::isspace(static_cast<unsigned char>(value[end - 1])))
        end--;

    return value.substr(begin, end - begin);
}

static std::string lowercase(const std::string& value) {
    std::string result = value;
    for (std::string::size_type i = 0; i < result.size(); i++)
        result[i] = char(std::tolower(static_cast<unsigned char>(result[i])));
    return result;
}

static bool startsWith(const std::string& value, const std::string& prefix) {
    return value.compare(0, prefix.size(), prefix) == 0;
}

static bool parseInteger(const std::string& text, int* result) {
    std::string cleaned = trim(text);
    char* end = NULL;
    errno = 0;
    long value = std::strtol(cleaned.c_str(), &end, 0);

    if (cleaned.empty() || end == cleaned.c_str() || errno != 0)
        return false;

    while (*end != '\0') {
        if (!std::isspace(static_cast<unsigned char>(*end)))
            return false;
        end++;
    }

    *result = int(value);
    return true;
}

static std::string integerText(int value) {
    return std::to_string(value);
}

static bool parseSizeSpec(const std::string& text, int* width, int* height) {
    std::string cleaned = lowercase(trim(text));
    std::string::size_type separator = cleaned.find('x');
    if (separator == std::string::npos)
        return false;

    std::string widthText = cleaned.substr(0, separator);
    std::string heightText = cleaned.substr(separator + 1);
    return parseInteger(widthText, width) && parseInteger(heightText, height);
}

static std::string withTrailingSlash(const std::string& path) {
    if (path.empty())
        return path;
    if (path[path.size() - 1] == '/')
        return path;
    return path + "/";
}

static void setDisplayMode(ConfigPatch& patch, const std::string& source,
    const std::string& value) {
    int width = 0;
    int height = 0;

    if (parseSizeSpec(value, &width, &height)) {
        patch.set(KEY_DISPLAY_MODE, "-1", source);
        patch.set(KEY_DISPLAY_WIDTH, integerText(width), source);
        patch.set(KEY_DISPLAY_HEIGHT, integerText(height), source);
        return;
    }

    patch.set(KEY_DISPLAY_MODE, trim(value), source);
}

static void setBufferSize(ConfigPatch& patch, const std::string& source,
    const std::string& value) {
    int width = 0;
    int height = 0;

    if (parseSizeSpec(value, &width, &height)) {
        patch.set(KEY_BUFFER_WIDTH, integerText(width), source);
        patch.set(KEY_BUFFER_HEIGHT, integerText(height), source);
        patch.set(KEY_BUFFER_CUSTOM_SIZE, "1", source);
        return;
    }

    patch.set(KEY_BUFFER_PRESET, trim(value), source);
}

static void setAudioMode(ConfigPatch& patch, const std::string& source,
    AudioInputMode mode, const std::string& fileName) {
    patch.set(KEY_AUDIO_INPUT_MODE, integerText(int(mode)), source);
    patch.set(KEY_AUDIO_INPUT_FILE, fileName, source);
}

static void applyIniOption(ConfigPatch& patch, DeferredLogBuffer& diagnostics,
    const std::string& source, const std::string& name,
    const std::string& value) {
    std::string key = lowercase(trim(name));
    std::string cleanedValue = trim(value);

    if (key == "verbose") {
        patch.set(KEY_LOGGING_VERBOSITY, cleanedValue, source);
    } else if (key == "no-verbose") {
        patch.set(KEY_LOGGING_VERBOSITY, "0", source);
    } else if (key == "path") {
        patch.set(KEY_PATH_EXTRA_LIBRARY, withTrailingSlash(cleanedValue), source);
    } else if (key == "ini-file") {
        diagnostics.warning(source, key,
            "cthugha.ini-file is ignored inside ini files; use --ini-file before startup config is built");
    } else if (key == "play") {
        setAudioMode(patch, source, AIM_File, cleanedValue);
    } else if (key == "random-noise") {
        setAudioMode(patch, source, AIM_Random, "");
    } else if (key == "no-sound") {
        setAudioMode(patch, source, AIM_None, "");
    } else if (key == "sound-device-number") {
        patch.set(KEY_AUDIO_INPUT_MODE, cleanedValue, source);
    } else if (key == "disp-mode") {
        setDisplayMode(patch, source, cleanedValue);
    } else if (key == "buff-size") {
        setBufferSize(patch, source, cleanedValue);
    }
}

static bool readOptionValue(const std::vector<std::string>& args, int* index,
    const std::string& optionName, std::string* value,
    DeferredLogBuffer& diagnostics) {
    if (*index + 1 >= int(args.size())) {
        diagnostics.error("command line", optionName, "missing required argument");
        return false;
    }

    (*index)++;
    *value = args[*index];
    return true;
}

static bool readShortOptionValue(const std::vector<std::string>& args, int* index,
    const std::string& arg, std::string* value,
    DeferredLogBuffer& diagnostics) {
    if (arg.size() > 2) {
        *value = arg.substr(2);
        return true;
    }

    return readOptionValue(args, index, arg, value, diagnostics);
}

static void applyCommandLineOption(ConfigPatch& patch,
    DeferredLogBuffer& diagnostics, const std::vector<std::string>& args,
    int* index) {
    std::string arg = args[*index];

    if (arg == "--verbose") {
        patch.set(KEY_LOGGING_VERBOSITY, DEFAULT_VERBOSE_COMMAND_LEVEL_TEXT,
            "command line");
    } else if (startsWith(arg, "--verbose=")) {
        patch.set(KEY_LOGGING_VERBOSITY, arg.substr(10), "command line");
    } else if (arg == "--no-verbose") {
        patch.set(KEY_LOGGING_VERBOSITY, "0", "command line");
    } else if (arg == "--path") {
        std::string value;
        if (readOptionValue(args, index, arg, &value, diagnostics))
            patch.set(KEY_PATH_EXTRA_LIBRARY, withTrailingSlash(value),
                "command line");
    } else if (startsWith(arg, "--path=")) {
        patch.set(KEY_PATH_EXTRA_LIBRARY, withTrailingSlash(arg.substr(7)),
            "command line");
    } else if (arg == "--ini-file") {
        std::string value;
        if (readOptionValue(args, index, arg, &value, diagnostics))
            patch.set(KEY_PATH_INI_OVERRIDE, value, "command line");
    } else if (startsWith(arg, "--ini-file=")) {
        patch.set(KEY_PATH_INI_OVERRIDE, arg.substr(11), "command line");
    } else if (arg == "--play") {
        std::string value;
        if (readOptionValue(args, index, arg, &value, diagnostics))
            setAudioMode(patch, "command line", AIM_File, value);
    } else if (startsWith(arg, "--play=")) {
        setAudioMode(patch, "command line", AIM_File, arg.substr(7));
    } else if (arg == "--random-noise") {
        setAudioMode(patch, "command line", AIM_Random, "");
    } else if (arg == "--no-sound" || arg == "-x") {
        setAudioMode(patch, "command line", AIM_None, "");
    } else if (arg == "--disp-mode") {
        std::string value;
        if (readOptionValue(args, index, arg, &value, diagnostics))
            setDisplayMode(patch, "command line", value);
    } else if (startsWith(arg, "--disp-mode=")) {
        setDisplayMode(patch, "command line", arg.substr(12));
    } else if (arg == "--buff-size") {
        std::string value;
        if (readOptionValue(args, index, arg, &value, diagnostics))
            setBufferSize(patch, "command line", value);
    } else if (startsWith(arg, "--buff-size=")) {
        setBufferSize(patch, "command line", arg.substr(12));
    } else if (startsWith(arg, "-E")) {
        std::string value;
        if (readShortOptionValue(args, index, arg, &value, diagnostics))
            patch.set(KEY_PATH_EXTRA_LIBRARY, withTrailingSlash(value),
                "command line");
    } else if (startsWith(arg, "-D")) {
        std::string value;
        if (readShortOptionValue(args, index, arg, &value, diagnostics))
            setDisplayMode(patch, "command line", value);
    } else if (startsWith(arg, "-S")) {
        std::string value;
        if (readShortOptionValue(args, index, arg, &value, diagnostics))
            setBufferSize(patch, "command line", value);
    }
}

static std::string removeInlineComment(const std::string& text) {
    std::string::size_type hash = text.find('#');
    std::string::size_type bang = text.find('!');
    std::string::size_type comment = std::string::npos;

    if (hash != std::string::npos)
        comment = hash;
    if (bang != std::string::npos)
        comment = std::min(comment, bang);

    if (comment == std::string::npos)
        return text;
    return text.substr(0, comment);
}

static bool parseIniLine(const std::string& line, std::string* name,
    std::string* value) {
    std::string cleaned = trim(removeInlineComment(line));
    if (cleaned.empty())
        return false;

    if (!startsWith(lowercase(cleaned), "cthugha."))
        return false;

    std::string::size_type colon = cleaned.find(':');
    if (colon == std::string::npos)
        return false;

    *name = trim(cleaned.substr(8, colon - 8));
    *value = trim(cleaned.substr(colon + 1));
    return !name->empty();
}

static bool parseAudioMode(const std::string& text, AudioInputMode* mode) {
    std::string cleaned = lowercase(trim(text));
    int value = 0;

    if (cleaned == "dsp" || cleaned == "dspin" || cleaned == "line-in"
        || cleaned == "mic" || cleaned == "microphone") {
        *mode = AIM_DSPIn;
        return true;
    }
    if (cleaned == "random" || cleaned == "random-noise") {
        *mode = AIM_Random;
        return true;
    }
    if (cleaned == "file" || cleaned == "play") {
        *mode = AIM_File;
        return true;
    }
    if (cleaned == "none" || cleaned == "no-sound" || cleaned == "off") {
        *mode = AIM_None;
        return true;
    }

    if (!parseInteger(cleaned, &value))
        return false;
    if (value < 0 || value >= AIM_Max)
        return false;

    *mode = AudioInputMode(value);
    return true;
}

static void applyIntEntry(const ConfigPatch& patch,
    DeferredLogBuffer& diagnostics, const char* key, int* target) {
    const ConfigEntry* configEntry = patch.entry(key);
    int value = 0;

    if (configEntry == NULL)
        return;

    if (!parseInteger(configEntry->value, &value)) {
        diagnostics.error(configEntry->source, configEntry->key,
            "expected an integer value");
        return;
    }

    *target = value;
}

static int clampIntWithWarning(int value, int minimum, int maximum,
    const ConfigEntry& entry, DeferredLogBuffer& diagnostics) {
    if (value < minimum) {
        diagnostics.warning(entry.source, entry.key,
            "value below supported range; clamped to minimum");
        return minimum;
    }
    if (value > maximum) {
        diagnostics.warning(entry.source, entry.key,
            "value above supported range; clamped to maximum");
        return maximum;
    }
    return value;
}

static std::string joinedPath(const std::string& directory,
    const std::string& fileName) {
    if (directory.empty())
        return fileName;
    if (directory[directory.size() - 1] == '/')
        return directory + fileName;
    return directory + "/" + fileName;
}

static void addHomeFile(std::vector<std::string>& files,
    const std::string& fileName) {
    const char* home = std::getenv("HOME");
    if (home == NULL || home[0] == '\0')
        return;

    files.push_back(joinedPath(home, fileName));
}

static std::map<std::string, std::string> processEnvironment(
    const std::vector<std::string>& names) {
    std::map<std::string, std::string> values;

    for (std::vector<std::string>::const_iterator it = names.begin();
         it != names.end(); ++it) {
        const char* value = std::getenv(it->c_str());
        if (value != NULL)
            values[*it] = value;
    }

    return values;
}

static std::vector<std::string> startupIniFiles(const ConfigPatch& commandLinePatch,
    std::string* continuationFile) {
    std::vector<std::string> files;
    const std::string* overridePath = commandLinePatch.value(KEY_PATH_INI_OVERRIDE);
    const std::string* extraPath = commandLinePatch.value(KEY_PATH_EXTRA_LIBRARY);

    if (overridePath != NULL && !overridePath->empty()) {
        files.push_back(*overridePath);
    } else {
        std::string libDir = CTH_LIBDIR;
        if (!libDir.empty())
            files.push_back(joinedPath(libDir, "cthugha.ini"));

        addHomeFile(files, ".cthugha.auto");
        addHomeFile(files, ".cthugha.ini");
        files.push_back("./cthugha.ini");

        if (extraPath != NULL && !extraPath->empty())
            files.push_back(joinedPath(*extraPath, "cthugha.ini"));
    }

    const char* home = std::getenv("HOME");
    continuationFile->clear();
    if (home != NULL && home[0] != '\0') {
        *continuationFile = joinedPath(home, ".cthugha.continue");
        files.push_back(*continuationFile);
    }

    return files;
}

}

ConfigDiagnostic::ConfigDiagnostic()
    : severity(ConfigDiagnosticInfo) { }

ConfigDiagnostic::ConfigDiagnostic(ConfigDiagnosticSeverity severityValue,
    const std::string& sourceValue, const std::string& keyValue,
    const std::string& messageValue)
    : severity(severityValue)
    , source(sourceValue)
    , key(keyValue)
    , message(messageValue) { }

void DeferredLogBuffer::add(ConfigDiagnosticSeverity severity,
    const std::string& source, const std::string& key,
    const std::string& message) {
    diagnosticsValue.push_back(ConfigDiagnostic(severity, source, key, message));
}

void DeferredLogBuffer::info(const std::string& source, const std::string& key,
    const std::string& message) {
    add(ConfigDiagnosticInfo, source, key, message);
}

void DeferredLogBuffer::warning(const std::string& source,
    const std::string& key, const std::string& message) {
    add(ConfigDiagnosticWarning, source, key, message);
}

void DeferredLogBuffer::error(const std::string& source, const std::string& key,
    const std::string& message) {
    add(ConfigDiagnosticError, source, key, message);
}

const std::vector<ConfigDiagnostic>& DeferredLogBuffer::diagnostics() const {
    return diagnosticsValue;
}

bool DeferredLogBuffer::hasErrors() const {
    for (std::vector<ConfigDiagnostic>::const_iterator it
         = diagnosticsValue.begin();
         it != diagnosticsValue.end(); ++it) {
        if (it->severity == ConfigDiagnosticError)
            return true;
    }
    return false;
}

void DeferredLogBuffer::append(const DeferredLogBuffer& other) {
    diagnosticsValue.insert(diagnosticsValue.end(), other.diagnosticsValue.begin(),
        other.diagnosticsValue.end());
}

ConfigEntry::ConfigEntry() { }

ConfigEntry::ConfigEntry(const std::string& keyValue,
    const std::string& valueValue, const std::string& sourceValue)
    : key(keyValue)
    , value(valueValue)
    , source(sourceValue) { }

void ConfigPatch::set(const std::string& key, const std::string& value,
    const std::string& source) {
    entriesValue[key] = ConfigEntry(key, value, source);
}

bool ConfigPatch::has(const std::string& key) const {
    return entriesValue.find(key) != entriesValue.end();
}

const ConfigEntry* ConfigPatch::entry(const std::string& key) const {
    std::map<std::string, ConfigEntry>::const_iterator it
        = entriesValue.find(key);
    if (it == entriesValue.end())
        return NULL;
    return &it->second;
}

const std::string* ConfigPatch::value(const std::string& key) const {
    const ConfigEntry* configEntry = entry(key);
    if (configEntry == NULL)
        return NULL;
    return &configEntry->value;
}

const std::map<std::string, ConfigEntry>& ConfigPatch::entries() const {
    return entriesValue;
}

void ConfigPatch::mergeFrom(const ConfigPatch& patch) {
    for (std::map<std::string, ConfigEntry>::const_iterator it
         = patch.entriesValue.begin();
         it != patch.entriesValue.end(); ++it) {
        entriesValue[it->first] = it->second;
    }
}

LoggingConfig::LoggingConfig()
    : verbosity(DEFAULT_VERBOSE_LEVEL) { }

PathConfig::PathConfig()
    : extraLibraryPath(DEFAULT_EXTRA_LIBRARY_PATH)
    , iniFileOverride(DEFAULT_INI_FILE_OVERRIDE_PATH) { }

AudioConfig::AudioConfig()
    : inputMode(DEFAULT_AUDIO_INPUT_MODE)
    , inputFile(DEFAULT_AUDIO_INPUT_FILE_PATH) { }

DisplayConfig::DisplayConfig()
    : displayMode(DEFAULT_DISPLAY_MODE)
    , hasCustomDisplaySize(false)
    , displayWidth(0)
    , displayHeight(0)
    , bufferWidth(160)
    , bufferHeight(100)
    , hasCustomBufferSize(false) { }

bool ConfigBuildResult::ok() const {
    for (std::vector<ConfigDiagnostic>::const_iterator it = diagnostics.begin();
         it != diagnostics.end(); ++it) {
        if (it->severity == ConfigDiagnosticError)
            return false;
    }
    return true;
}

Config ConfigSchema::build(const ConfigPatch& patch,
    DeferredLogBuffer& diagnostics) const {
    Config config;

    applyIntEntry(patch, diagnostics, KEY_LOGGING_VERBOSITY,
        &config.logging.verbosity);

    if (const std::string* value = patch.value(KEY_PATH_EXTRA_LIBRARY))
        config.paths.extraLibraryPath = *value;
    if (const std::string* value = patch.value(KEY_PATH_INI_OVERRIDE))
        config.paths.iniFileOverride = *value;

    if (const ConfigEntry* entry = patch.entry(KEY_AUDIO_INPUT_MODE)) {
        AudioInputMode mode = DEFAULT_AUDIO_INPUT_MODE;
        if (parseAudioMode(entry->value, &mode)) {
            config.audio.inputMode = mode;
        } else {
            diagnostics.error(entry->source, entry->key,
                "expected a supported audio input mode");
        }
    }

    if (const std::string* value = patch.value(KEY_AUDIO_INPUT_FILE))
        config.audio.inputFile = *value;

    if (const ConfigEntry* entry = patch.entry(KEY_DISPLAY_MODE)) {
        int mode = DEFAULT_DISPLAY_MODE;
        if (parseInteger(entry->value, &mode)) {
            int maxMode = int(sizeof(screenSizePresets) / sizeof(screenSizePresets[0])) - 1;
            if (mode != -1)
                mode = clampIntWithWarning(mode, 0, maxMode, *entry,
                    diagnostics);
            config.display.displayMode = mode;
        } else {
            diagnostics.error(entry->source, entry->key,
                "expected a display mode number or WIDTHxHEIGHT");
        }
    }

    const ConfigEntry* displayWidthEntry = patch.entry(KEY_DISPLAY_WIDTH);
    const ConfigEntry* displayHeightEntry = patch.entry(KEY_DISPLAY_HEIGHT);
    if (displayWidthEntry != NULL || displayHeightEntry != NULL) {
        int width = 0;
        int height = 0;
        if (displayWidthEntry == NULL || displayHeightEntry == NULL) {
            diagnostics.error("configuration", "display.size",
                "display width and height must be supplied together");
        } else if (!parseInteger(displayWidthEntry->value, &width)
            || !parseInteger(displayHeightEntry->value, &height)) {
            diagnostics.error(displayWidthEntry->source, "display.size",
                "expected WIDTHxHEIGHT display dimensions");
        } else if (width <= 0 || height <= 0) {
            diagnostics.error(displayWidthEntry->source, "display.size",
                "display dimensions must be positive");
        } else {
            config.display.hasCustomDisplaySize = true;
            config.display.displayMode = -1;
            config.display.displayWidth = width;
            config.display.displayHeight = height;
        }
    }

    if (const ConfigEntry* entry = patch.entry(KEY_BUFFER_PRESET)) {
        int preset = 0;
        if (parseInteger(entry->value, &preset)) {
            int maxPreset
                = int(sizeof(bufferSizePresets) / sizeof(bufferSizePresets[0])) - 1;
            preset = clampIntWithWarning(preset, 0, maxPreset, *entry,
                diagnostics);
            config.display.bufferWidth = bufferSizePresets[preset].width;
            config.display.bufferHeight = bufferSizePresets[preset].height;
            if (!config.display.hasCustomDisplaySize)
                config.display.displayMode
                    = std::max(config.display.displayMode, preset);
        } else {
            diagnostics.error(entry->source, entry->key,
                "expected a buffer preset number or WIDTHxHEIGHT");
        }
    }

    const ConfigEntry* bufferWidthEntry = patch.entry(KEY_BUFFER_WIDTH);
    const ConfigEntry* bufferHeightEntry = patch.entry(KEY_BUFFER_HEIGHT);
    if (bufferWidthEntry != NULL || bufferHeightEntry != NULL) {
        int width = 0;
        int height = 0;
        if (bufferWidthEntry == NULL || bufferHeightEntry == NULL) {
            diagnostics.error("configuration", "buffer.size",
                "buffer width and height must be supplied together");
        } else if (!parseInteger(bufferWidthEntry->value, &width)
            || !parseInteger(bufferHeightEntry->value, &height)) {
            diagnostics.error(bufferWidthEntry->source, "buffer.size",
                "expected WIDTHxHEIGHT buffer dimensions");
        } else {
            width = clampIntWithWarning(width, 64, MAX_BUFF_WIDTH,
                *bufferWidthEntry, diagnostics);
            height = clampIntWithWarning(height, 64, MAX_BUFF_HEIGHT,
                *bufferHeightEntry, diagnostics);
            config.display.bufferWidth = width;
            config.display.bufferHeight = height;
            config.display.hasCustomBufferSize
                = patch.has(KEY_BUFFER_CUSTOM_SIZE);
        }
    }

    return config;
}

ConfigAcquisitionStrategy::~ConfigAcquisitionStrategy() { }

DefaultsConfigSource::DefaultsConfigSource()
    : defaultsValue(hardcodedDefaultConfigPatch()) { }

DefaultsConfigSource::DefaultsConfigSource(const ConfigPatch& defaults)
    : defaultsValue(defaults) { }

ConfigPatch DefaultsConfigSource::acquire(DeferredLogBuffer& /* diagnostics */) const {
    return defaultsValue;
}

IniTextConfigSource::IniTextConfigSource(const std::string& sourceName,
    const std::string& text)
    : sourceNameValue(sourceName)
    , textValue(text) { }

ConfigPatch IniTextConfigSource::acquire(DeferredLogBuffer& diagnostics) const {
    ConfigPatch patch;
    std::istringstream input(textValue);
    std::string line;
    int lineNumber = 0;

    while (std::getline(input, line)) {
        std::string name;
        std::string value;
        lineNumber++;
        if (parseIniLine(line, &name, &value)) {
            applyIniOption(patch, diagnostics,
                sourceNameValue + ":" + integerText(lineNumber), name, value);
        }
    }

    return patch;
}

IniFileConfigSource::IniFileConfigSource(const std::string& path, bool optional)
    : pathValue(path)
    , optionalValue(optional) { }

ConfigPatch IniFileConfigSource::acquire(DeferredLogBuffer& diagnostics) const {
    std::ifstream input(pathValue.c_str(), std::ios::in | std::ios::binary);
    std::stringstream text;

    if (!input) {
        if (!optionalValue)
            diagnostics.error(pathValue, "ini-file", "could not open ini file");
        return ConfigPatch();
    }

    text << input.rdbuf();
    return IniTextConfigSource(pathValue, text.str()).acquire(diagnostics);
}

EnvironmentConfigSource::EnvironmentConfigSource(
    const std::map<std::string, std::string>& environment)
    : environmentValue(environment) { }

ConfigPatch EnvironmentConfigSource::acquire(
    DeferredLogBuffer& /* diagnostics */) const {
    ConfigPatch patch;
    std::map<std::string, std::string>::const_iterator verbose
        = environmentValue.find("CTH_VERBOSE");

    if (verbose != environmentValue.end())
        patch.set(KEY_LOGGING_VERBOSITY, verbose->second, "environment:CTH_VERBOSE");

    return patch;
}

CommandLineConfigSource::CommandLineConfigSource(
    const std::vector<std::string>& arguments)
    : argumentsValue(arguments) { }

CommandLineConfigSource::CommandLineConfigSource(int argc, char* argv[])
    : argumentsValue(configArgumentsFromArgv(argc, argv)) { }

ConfigPatch CommandLineConfigSource::acquire(DeferredLogBuffer& diagnostics) const {
    ConfigPatch patch;

    for (int i = 1; i < int(argumentsValue.size()); i++)
        applyCommandLineOption(patch, diagnostics, argumentsValue, &i);

    return patch;
}

ConfigurationBuilder::ConfigurationBuilder() { }

ConfigurationBuilder& ConfigurationBuilder::addSource(
    const ConfigAcquisitionStrategy& source) {
    ConfigPatch sourcePatch = source.acquire(diagnosticsValue);
    patchValue.mergeFrom(sourcePatch);
    return *this;
}

ConfigurationBuilder& ConfigurationBuilder::addDefaults(
    const ConfigPatch& defaults) {
    DefaultsConfigSource source(defaults);
    return addSource(source);
}

ConfigurationBuilder& ConfigurationBuilder::addDefaults() {
    DefaultsConfigSource source;
    return addSource(source);
}

ConfigurationBuilder& ConfigurationBuilder::addIniText(
    const std::string& sourceName, const std::string& text) {
    IniTextConfigSource source(sourceName, text);
    return addSource(source);
}

ConfigurationBuilder& ConfigurationBuilder::addIniFile(const std::string& path,
    bool optional) {
    IniFileConfigSource source(path, optional);
    return addSource(source);
}

ConfigurationBuilder& ConfigurationBuilder::addEnvironment(
    const std::map<std::string, std::string>& environment) {
    EnvironmentConfigSource source(environment);
    return addSource(source);
}

ConfigurationBuilder& ConfigurationBuilder::addEnvironmentVariables(
    const std::vector<std::string>& names) {
    return addEnvironment(processEnvironment(names));
}

ConfigurationBuilder& ConfigurationBuilder::addCommandLine(
    const std::vector<std::string>& args) {
    CommandLineConfigSource source(args);
    return addSource(source);
}

ConfigurationBuilder& ConfigurationBuilder::addCommandLine(int argc,
    char* argv[]) {
    CommandLineConfigSource source(argc, argv);
    return addSource(source);
}

const ConfigPatch& ConfigurationBuilder::patch() const {
    return patchValue;
}

ConfigBuildResult ConfigurationBuilder::build() const {
    DeferredLogBuffer diagnostics = diagnosticsValue;
    ConfigBuildResult result;

    result.config = schemaValue.build(patchValue, diagnostics);
    result.diagnostics = diagnostics.diagnostics();
    return result;
}

ConfigPatch hardcodedDefaultConfigPatch() {
    ConfigPatch defaults;

    defaults.set(KEY_LOGGING_VERBOSITY, integerText(DEFAULT_VERBOSE_LEVEL),
        "defaults");
    defaults.set(KEY_PATH_EXTRA_LIBRARY, DEFAULT_EXTRA_LIBRARY_PATH, "defaults");
    defaults.set(KEY_PATH_INI_OVERRIDE, DEFAULT_INI_FILE_OVERRIDE_PATH,
        "defaults");
    defaults.set(KEY_AUDIO_INPUT_MODE, integerText(int(DEFAULT_AUDIO_INPUT_MODE)),
        "defaults");
    defaults.set(KEY_AUDIO_INPUT_FILE, DEFAULT_AUDIO_INPUT_FILE_PATH,
        "defaults");
    defaults.set(KEY_DISPLAY_MODE, integerText(DEFAULT_DISPLAY_MODE), "defaults");

    return defaults;
}

std::vector<std::string> configArgumentsFromArgv(int argc, char* argv[]) {
    std::vector<std::string> args;
    for (int i = 0; i < argc; i++)
        args.push_back(argv[i] ? argv[i] : "");
    return args;
}

ConfigBuildResult buildStartupConfig(int argc, char* argv[]) {
    std::vector<std::string> args = configArgumentsFromArgv(argc, argv);
    DeferredLogBuffer bootstrapDiagnostics;
    ConfigPatch commandLinePatch
        = CommandLineConfigSource(args).acquire(bootstrapDiagnostics);
    std::string continuationFile;
    std::vector<std::string> iniFiles
        = startupIniFiles(commandLinePatch, &continuationFile);
    ConfigurationBuilder builder;
    ConfigBuildResult result;

    builder.addDefaults();
    for (std::vector<std::string>::const_iterator it = iniFiles.begin();
         it != iniFiles.end(); ++it) {
        bool optional = true;
        if (commandLinePatch.has(KEY_PATH_INI_OVERRIDE) && it == iniFiles.begin())
            optional = false;
        builder.addIniFile(*it, optional);
    }

    builder.addEnvironmentVariables(std::vector<std::string>(1, "CTH_VERBOSE"));
    builder.addCommandLine(args);

    result = builder.build();
    result.config.paths.iniFiles = iniFiles;
    result.config.paths.continuationIniFile = continuationFile;
    return result;
}
