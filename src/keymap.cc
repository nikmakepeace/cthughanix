#include "cthugha.h"
#include "Configuration.h"
#include "keymap.h"
#include "InterfaceRuntime.h"
#include "Interface.h"
#include "keys.h"
#include "display.h"
#include "disp-sys.h"
#include "Border.h"
#include "CthughaBuffer.h"
#include "Flashlight.h"
#include "VideoDirector.h"
#include "flames.h"
#include "waves.h"
#include "imath.h"
#include "CthughaDisplay.h"
#include "DisplayDevice.h"
#include "Scene.h"
#include "RuntimeCommandSink.h"

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

void skipSpace(const char*& str) {
    while (isspace(*str))
        str++;
}

static int keymapFeatureValue(const char* name, int& known) {
    known = 1;

#ifdef WITH_DSP
    if (strcasecmp(name, "WITH_DSP") == 0)
        return WITH_DSP;
#endif
#ifdef WITH_MIXER
    if (strcasecmp(name, "WITH_MIXER") == 0)
        return WITH_MIXER;
#endif
#ifdef WITH_PULSE
    if (strcasecmp(name, "WITH_PULSE") == 0)
        return WITH_PULSE;
#endif
#ifdef WITH_MINIMP3
    if (strcasecmp(name, "WITH_MINIMP3") == 0)
        return WITH_MINIMP3;
#endif
    known = 0;
    return 0;
}

static long keymapConditionTerm(const char*& str, int& known) {
    skipSpace(str);

    char* end = NULL;
    long value = strtol(str, &end, 0);
    if (end != str) {
        str = end;
        known = 1;
        return value;
    }

    char name[128];
    int i = 0;
    while ((isalnum(*str) || (*str == '_')) && (i < int(sizeof(name)) - 1)) {
        name[i++] = *str;
        str++;
    }
    name[i] = '\0';

    if (i == 0) {
        known = 0;
        return 0;
    }

    int featureKnown = 0;
    value = keymapFeatureValue(name, featureKnown);
    if (!featureKnown)
        CTH_WARN("Unknown keymap condition feature '%s'.\n", name);

    known = featureKnown;
    return value;
}

static int keymapConditionValue(const char* str) {
    int negate = 0;
    skipSpace(str);
    if (*str == '!') {
        negate = 1;
        str++;
    }

    int known = 0;
    long left = keymapConditionTerm(str, known);
    int result = known && (left != 0);

    skipSpace(str);
    if ((strncmp(str, "==", 2) == 0) || (strncmp(str, "!=", 2) == 0)) {
        int equals = (str[0] == '=');
        str += 2;

        int rightKnown = 0;
        long right = keymapConditionTerm(str, rightKnown);
        result = known && rightKnown && (equals ? (left == right) : (left != right));
    } else if (*str != '\0') {
        CTH_WARN("Unsupported keymap condition expression near '%s'.\n", str);
        result = 0;
    }

    return negate ? !result : result;
}

static int keymapDirective(const char*& str, const char* directive) {
    int len = strlen(directive);
    if (strncasecmp(str, directive, len) != 0)
        return 0;

    if ((str[len] != '\0') && !isspace(str[len]))
        return 0;

    str += len;
    skipSpace(str);
    return 1;
}

class KeymapConditionState {
    struct Frame {
        int parentActive;
        int conditionActive;
        int seenElse;
    };

    enum { MaxDepth = 32 };

    Frame stack[MaxDepth];
    int depth;
    int currentActive;

public:
    KeymapConditionState()
        : depth(0)
        , currentActive(1) { }

    int active() const { return currentActive; }

    int handle(const char* rawLine) {
        const char* line = rawLine;
        skipSpace(line);

        if (*line != '#')
            return 0;

        line++;

        if (keymapDirective(line, "if")) {
            if (depth >= MaxDepth) {
                CTH_ERROR("Too many nested keymap conditionals.\n");
                currentActive = 0;
                return 1;
            }

            int conditionActive = keymapConditionValue(line);
            stack[depth].parentActive = currentActive;
            stack[depth].conditionActive = conditionActive;
            stack[depth].seenElse = 0;
            currentActive = currentActive && conditionActive;
            depth++;
            return 1;
        }

        if (keymapDirective(line, "else")) {
            if (depth == 0) {
                CTH_ERROR("Unexpected #else in keymap.\n");
                return 1;
            }

            Frame& frame = stack[depth - 1];
            if (frame.seenElse)
                CTH_ERROR("Duplicate #else in keymap conditional.\n");
            frame.seenElse = 1;
            currentActive = frame.parentActive && !frame.conditionActive;
            return 1;
        }

        if (keymapDirective(line, "endif")) {
            if (depth == 0) {
                CTH_ERROR("Unexpected #endif in keymap.\n");
                return 1;
            }

            depth--;
            currentActive = stack[depth].parentActive;
            return 1;
        }

        return 0;
    }

    void finish() const {
        if (depth != 0)
            CTH_ERROR("Unterminated #if in keymap.\n");
    }
};

Keymap::Keymap(const char* n)
    : next(NULL)
    , name(NULL)
    , bindingList(NULL) {
    name = new char[strlen(n) + 1];
    strcpy(name, n);
}

Keymap::~Keymap() {
    delete[] name;

    while (bindingList != NULL) {
        BindingList* binding = bindingList;
        bindingList = bindingList->next;

        Binding::ActionList* action = binding->actionList;
        while (action != NULL) {
            Binding::ActionList* nextAction = action->next;
            delete[] action->param;
            delete action;
            action = nextAction;
        }

        delete binding;
    }
}

CommandRegistry::CommandRegistry()
    : firstValue(NULL) { }

CommandRegistry::~CommandRegistry() {
    while (firstValue != NULL) {
        Action* action = firstValue;
        firstValue = firstValue->next;
        delete action;
    }
}

void CommandRegistry::registerAction(Action* action) {
    if (action == NULL)
        return;

    for (Action* existing = firstValue; existing != NULL;
         existing = existing->next) {
        if (strcasecmp(existing->name, action->name) == 0) {
            delete action;
            return;
        }
    }

    action->next = firstValue;
    firstValue = action;
}

Action* CommandRegistry::findLongestPrefix(const char* text) const {
    Action* found = NULL;
    int length = -1;

    for (Action* action = firstValue; action != NULL;
         action = action->next) {
        if (((*action) == text) && (action->length() > length)) {
            found = action;
            length = action->length();
        }
    }

    return found;
}

KeymapRegistry::KeymapRegistry()
    : firstValue(NULL) { }

KeymapRegistry::~KeymapRegistry() {
    while (firstValue != NULL) {
        Keymap* keymap = firstValue;
        firstValue = firstValue->next;
        delete keymap;
    }
}

void KeymapRegistry::readFile(const char* fileName,
    CommandRegistry& commands) {
    FILE* file = fopen(fileName, "r");
    if (file == NULL) {
        CTH_ERRNO(errno, "Can not open keymap file '%s'.", fileName);
        return;
    }

    CTH_DEBUG("   starting reading keymap file '%s'.\n", fileName);

    int lineNr = 0;
    char line[4096];
    Keymap* keymap = find("default", 1);
    KeymapConditionState conditions;

    while (fgets(line, 4096, file) != NULL) {
        const char* lineP = line;

        char* N = strchr(line, '\n'); // delete the trailing newline
        if (N)
            *N = '\0';
        lineNr++;

        if (conditions.handle(line))
            continue;

        if (!conditions.active())
            continue;

        if (strncasecmp("#keymap", line, 7) == 0) {
            lineP = line + 7;
            skipSpace(lineP);
            keymap = find(lineP, 1);

            CTH_DEBUG("      defining keymap '%s'.\n", keymap->name);
        } else {
            keymap->add(line, commands);
        }
    }

    if (ferror(file)) {
        CTH_ERRNO(errno, "Error while reading keymap file '%s' at line %d.", fileName, lineNr);
        fclose(file);
        return;
    }

    conditions.finish();

    CTH_DEBUG("   finished reading keymap file '%s'.\n", fileName);
    fclose(file);
}

void KeymapRegistry::addList(CommandRegistry& commands, int dummy, ...) {

    va_list ap;
    va_start(ap, dummy); /* Initialize the argument list. */

    Keymap* keymap = find("default", 1);
    KeymapConditionState conditions;

    const char* line = va_arg(ap, const char*);
    while (line != NULL) {
        const char* lineP;

        if (conditions.handle(line)) {
            line = va_arg(ap, const char*);
            continue;
        }

        if (!conditions.active()) {
            line = va_arg(ap, const char*);
            continue;
        }

        if (strncasecmp("#keymap", line, 7) == 0) {
            lineP = line + 7;
            skipSpace(lineP);
            keymap = find(lineP, 1);

            CTH_DEBUG("      defining keymap '%s'.\n", keymap->name);
        } else {
            keymap->add(line, commands);
        }

        line = va_arg(ap, const char*);
    }

    conditions.finish();

    va_end(ap);
}

Keymap* KeymapRegistry::find(const char* name, int create) {
    for (Keymap* k = firstValue; k; k = k->next) {
        if (strcasecmp(name, k->name) == 0) {
            return k;
        }
    }

    if (create) {
        CTH_DEBUG("      adding new keymap '%s'.\n", name);
        Keymap* keymap = new Keymap(name);
        keymap->next = firstValue;
        firstValue = keymap;
        return keymap;
    } else {
        CTH_ERROR("Could not find keymap '%s'.\n", name);
        return firstValue;
    }
}

void Keymap::add(Binding& b) {
    // try to replace an existing key
    BindingList* l = bindingList;
    while (l) {
        if (l->key == b.key) {
            l->actionList = b.actionList;
            //	    delete l->param;
            //	    l->param = b.param;
            return;
        }
        l = l->next;
    }
    // add a new key
    bindingList = new BindingList(b, bindingList);
}

Keymap::Binding Keymap::parseBinding(const char* line,
    CommandRegistry& commands) {
    const char* fullLine = line;
    Binding b;

    skipSpace(line);

    switch (*line) {
    case '\0':
    case '#':
    case '!':
        return b;
    case '\\':
        if (line[1] == '\0')
            return b;

        b.key = line[1];
        line++;
        break;

    default:
        b.key = *line;
    }

    line++;
    if ((*line != '\0') && !isspace(*line)) { // a multi-character thing
        line--;

        int consumed = 0;
        int namedKey = keyCodeForNamePrefix(line, &consumed);
        if (consumed > 0) {
            b.key = namedKey;
            line += consumed;
        }

        if ((*line != '\0') && !isspace(*line)) {
            CTH_ERROR("Unknown key symbol in keymap line: '%s'\n", fullLine);
            b.key = 0;
            return b;
        }
    }
    skipSpace(line);

    b.actionList = NULL; // no actions so far
    while ((*line) != '\0') {

        // try to match an action
        Action* A = commands.findLongestPrefix(line);
        if (A) { // found some match
            line += A->length(); // skip command name

            skipSpace(line);

            if (*line != '(') { // check and skip '('
                CTH_ERROR("Missing '(' in binding: '%s'\n", fullLine);
                b.key = 0;
                return b;
            }
            line++;

            if (strchr(line, ')') == NULL) { // check and skip ')'
                CTH_ERROR("Missing ')' in binding: '%s'\n", fullLine);
                b.key = 0;
                return b;
            }

            int n = strchr(line, ')') - line; // parse parameter
            char* p = new char[n + 1];
            strncpy(p, line, n);
            p[n] = '\0';

            line += n + 1; // skip parameter and ')'

            // add new action to List
            b.actionList = new Binding::ActionList(A, p, b.actionList);
        } else {
            CTH_ERROR("Error in keymap. Unknown command: '%s'\n", line);
            b.key = 0;
            return b;
        }
        skipSpace(line);
    }
    CTH_DEBUG("keymap: parsed new keymap entry line '%s'\n", fullLine);
    return b;
}

int Keymap::action(int key, InterfaceRuntime& runtime) {

    BindingList* l = bindingList;
    while (l) {
        if (l->key == key) {
            for (Binding::ActionList* a = l->actionList; a != NULL; a = a->next) {
                double value;
                if (a->param)
                    sscanf(a->param, "%lf", &value);

                a->action->act(a->param, value, runtime);
            }
            return 0;
        }
        l = l->next;
    }
    return 1;
}

int KeymapRegistry::action(const char* keymap, int key,
    InterfaceRuntime& runtime) {

    for (Keymap* k = firstValue; k; k = k->next) {
        if (strcasecmp(keymap, k->name) == 0) {
            return k->action(key, runtime);
        }
    }
    return 1;
}

void Keymap::add(const char* line, CommandRegistry& commands) {
    Binding b = parseBinding(line, commands);
    if (b.key != 0)
        add(b);
}

//
// intializiation of the default keymaps
//
void KeymapRegistry::init(const InputConfig& config,
    CommandRegistry& commands) {

    addList(commands, 0,
#include <default.keymap.str>
        NULL);

    if (!config.keymapFile.empty())
        readFile(config.keymapFile.c_str(), commands);
}

//
// all the actions implemented
//

static void applyRuntimeCommand(const RuntimeCommand& command,
    InterfaceRuntime& runtime) {
    RuntimeCommandSink* sink = runtime.runtimeCommandSink();
    if (sink != NULL)
        sink->apply(command);
}

ACTION(quit) { applyRuntimeCommand(RuntimeCommand::requestClose(), runtime); }
ACTION(stopAndContinue) {
    applyRuntimeCommand(RuntimeCommand::stopAndContinue(), runtime);
}

ACTION(screenChg) { applyRuntimeCommand(RuntimeCommand::changeScreenBy(int(v)), runtime); }
ACTION(zoomChg) { applyRuntimeCommand(RuntimeCommand::changeZoomBy(int(v)), runtime); }
ACTION(flameChg) { applyRuntimeCommand(RuntimeCommand::changeSceneBy(RuntimeSceneFlame, int(v)), runtime); }
ACTION(flameGeneral) { applyRuntimeCommand(RuntimeCommand::changeSceneBy(RuntimeSceneGeneralFlame, 0), runtime); }
ACTION(waveChg) { applyRuntimeCommand(RuntimeCommand::changeSceneBy(RuntimeSceneWave, int(v)), runtime); }
ACTION(waveScaleChg) { applyRuntimeCommand(RuntimeCommand::changeSceneBy(RuntimeSceneWaveScale, int(v)), runtime); }
ACTION(objectChg) { applyRuntimeCommand(RuntimeCommand::changeSceneBy(RuntimeSceneObject, int(v)), runtime); }
ACTION(translateChg) { applyRuntimeCommand(RuntimeCommand::changeSceneBy(RuntimeSceneTranslation, int(v)), runtime); }
ACTION(soundProcessChg) { applyRuntimeCommand(RuntimeCommand::changeSoundProcessingBy(int(v)), runtime); }
ACTION(borderChg) { applyRuntimeCommand(RuntimeCommand::changeSceneBy(RuntimeSceneBorder, int(v)), runtime); }
ACTION(flashlightChg) { applyRuntimeCommand(RuntimeCommand::changeSceneBy(RuntimeSceneFlashlight, int(v)), runtime); }
ACTION(paletteChg) { applyRuntimeCommand(RuntimeCommand::changeSceneBy(RuntimeScenePalette, int(v)), runtime); }
ACTION(tableChg) { applyRuntimeCommand(RuntimeCommand::changeSceneBy(RuntimeSceneTable, int(v)), runtime); }
ACTION(imageChg) { applyRuntimeCommand(RuntimeCommand::changeSceneBy(RuntimeSceneImage, int(v)), runtime); }
ACTION(lockChg) { applyRuntimeCommand(RuntimeCommand::toggleAutoChangeLock(), runtime); }

ACTION(screen) { applyRuntimeCommand(RuntimeCommand::changeScreenTo(p), runtime); }
ACTION(zoom) { applyRuntimeCommand(RuntimeCommand::changeZoomTo(p), runtime); }
ACTION(flame) { applyRuntimeCommand(RuntimeCommand::changeSceneTo(RuntimeSceneFlame, p), runtime); }
ACTION(wave) { applyRuntimeCommand(RuntimeCommand::changeSceneTo(RuntimeSceneWave, p), runtime); }
ACTION(waveScale) { applyRuntimeCommand(RuntimeCommand::changeSceneTo(RuntimeSceneWaveScale, p), runtime); }
ACTION(object) { applyRuntimeCommand(RuntimeCommand::changeSceneTo(RuntimeSceneObject, p), runtime); }
ACTION(translate) { applyRuntimeCommand(RuntimeCommand::changeSceneTo(RuntimeSceneTranslation, p), runtime); }
ACTION(soundProcess) { applyRuntimeCommand(RuntimeCommand::changeSoundProcessingTo(p), runtime); }
ACTION(border) { applyRuntimeCommand(RuntimeCommand::changeSceneTo(RuntimeSceneBorder, p), runtime); }
ACTION(flashlight) { applyRuntimeCommand(RuntimeCommand::changeSceneTo(RuntimeSceneFlashlight, p), runtime); }
ACTION(palette) { applyRuntimeCommand(RuntimeCommand::changeSceneTo(RuntimeScenePalette, p), runtime); }
ACTION(table) { applyRuntimeCommand(RuntimeCommand::changeSceneTo(RuntimeSceneTable, p), runtime); }
ACTION(image) { applyRuntimeCommand(RuntimeCommand::changeSceneTo(RuntimeSceneImage, p), runtime); }
ACTION(lock) { applyRuntimeCommand(RuntimeCommand::toggleAutoChangeLock(), runtime); }

ACTION(writeIni) { applyRuntimeCommand(RuntimeCommand::writeIni(), runtime); }

ACTION(restore) { applyRuntimeCommand(RuntimeCommand::restoreScene(), runtime); }
ACTION(toggleSave) {
    runtime.toggleSaveToPreset();
}
ACTION(save) { applyRuntimeCommand(RuntimeCommand::savePreset(int(v)), runtime); }
ACTION(saveOrRestore) {
    if (runtime.saveToPreset()) {
        applyRuntimeCommand(RuntimeCommand::savePreset(int(v)), runtime);
        runtime.clearSaveToPreset();
    } else {
        applyRuntimeCommand(RuntimeCommand::restorePreset(int(v)), runtime);
    }
}

ACTION(toggleStatus) {
    runtime.toggleStatus();
}
ACTION(toggleFPS) { applyRuntimeCommand(RuntimeCommand::toggleShowFps(), runtime); }

ACTION(changeAll) { applyRuntimeCommand(RuntimeCommand::changeAll(), runtime); }
ACTION(changeOne) { applyRuntimeCommand(RuntimeCommand::changeOne(), runtime); }

ACTION(credits) {
    runtime.set("credits");
}
ACTION(setInterface) {
    runtime.set(p);
}

ACTION(randomPalette) {
    applyRuntimeCommand(RuntimeCommand::randomPalette(), runtime);
}
ACTION(newRandomPalette) {
    applyRuntimeCommand(RuntimeCommand::addRandomPalette(), runtime);
}

void registerDefaultKeyActions(CommandRegistry& registry) {
#define REGISTER_ACTION(a) registry.registerAction(new a##Action())
    REGISTER_ACTION(quit);
    REGISTER_ACTION(stopAndContinue);
    REGISTER_ACTION(screenChg);
    REGISTER_ACTION(zoomChg);
    REGISTER_ACTION(flameChg);
    REGISTER_ACTION(flameGeneral);
    REGISTER_ACTION(waveChg);
    REGISTER_ACTION(waveScaleChg);
    REGISTER_ACTION(objectChg);
    REGISTER_ACTION(translateChg);
    REGISTER_ACTION(soundProcessChg);
    REGISTER_ACTION(borderChg);
    REGISTER_ACTION(flashlightChg);
    REGISTER_ACTION(paletteChg);
    REGISTER_ACTION(tableChg);
    REGISTER_ACTION(imageChg);
    REGISTER_ACTION(lockChg);
    REGISTER_ACTION(screen);
    REGISTER_ACTION(zoom);
    REGISTER_ACTION(flame);
    REGISTER_ACTION(wave);
    REGISTER_ACTION(waveScale);
    REGISTER_ACTION(object);
    REGISTER_ACTION(translate);
    REGISTER_ACTION(soundProcess);
    REGISTER_ACTION(border);
    REGISTER_ACTION(flashlight);
    REGISTER_ACTION(palette);
    REGISTER_ACTION(table);
    REGISTER_ACTION(image);
    REGISTER_ACTION(lock);
    REGISTER_ACTION(writeIni);
    REGISTER_ACTION(restore);
    REGISTER_ACTION(toggleSave);
    REGISTER_ACTION(save);
    REGISTER_ACTION(saveOrRestore);
    REGISTER_ACTION(toggleStatus);
    REGISTER_ACTION(toggleFPS);
    REGISTER_ACTION(changeAll);
    REGISTER_ACTION(changeOne);
    REGISTER_ACTION(credits);
    REGISTER_ACTION(setInterface);
    REGISTER_ACTION(randomPalette);
    REGISTER_ACTION(newRandomPalette);
#undef REGISTER_ACTION
}
