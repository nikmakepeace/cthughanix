#include "cthugha.h"
#include "SilenceMessage.h"

#include <dirent.h>
#include <fstream>
#include <string.h>

static const char* defaultSilenceMessages[] = {
    "Where is the music?",
    "JOLT !",
    "Turn The Music On",
    "Lets Party!!!",
    "Pink Floyd Rules",
    "Sounds of Silence ?",
    "The Torps",
    "Study Mathematics",
    "Visit Linz",
    "Press ? for help",
    "Number 5 is ALIVE!!",
    "Spooky Mulder.....",
    "Math is Power",
    "Subliminal Ads",
    "Read a book",
    "Get a life...",
    "SMILE!",
    "Cthugha-L",
    "Don't Panic!",
    "@fortune"
};

static const unsigned int cp437Unicode[128] = {
    0x00C7, 0x00FC, 0x00E9, 0x00E2, 0x00E4, 0x00E0, 0x00E5, 0x00E7,
    0x00EA, 0x00EB, 0x00E8, 0x00EF, 0x00EE, 0x00EC, 0x00C4, 0x00C5,
    0x00C9, 0x00E6, 0x00C6, 0x00F4, 0x00F6, 0x00F2, 0x00FB, 0x00F9,
    0x00FF, 0x00D6, 0x00DC, 0x00A2, 0x00A3, 0x00A5, 0x20A7, 0x0192,
    0x00E1, 0x00ED, 0x00F3, 0x00FA, 0x00F1, 0x00D1, 0x00AA, 0x00BA,
    0x00BF, 0x2310, 0x00AC, 0x00BD, 0x00BC, 0x00A1, 0x00AB, 0x00BB,
    0x2591, 0x2592, 0x2593, 0x2502, 0x2524, 0x2561, 0x2562, 0x2556,
    0x2555, 0x2563, 0x2551, 0x2557, 0x255D, 0x255C, 0x255B, 0x2510,
    0x2514, 0x2534, 0x252C, 0x251C, 0x2500, 0x253C, 0x255E, 0x255F,
    0x255A, 0x2554, 0x2569, 0x2566, 0x2560, 0x2550, 0x256C, 0x2567,
    0x2568, 0x2564, 0x2565, 0x2559, 0x2558, 0x2552, 0x2553, 0x256B,
    0x256A, 0x2518, 0x250C, 0x2588, 0x2584, 0x258C, 0x2590, 0x2580,
    0x03B1, 0x00DF, 0x0393, 0x03C0, 0x03A3, 0x03C3, 0x00B5, 0x03C4,
    0x03A6, 0x0398, 0x03A9, 0x03B4, 0x221E, 0x03C6, 0x03B5, 0x2229,
    0x2261, 0x00B1, 0x2265, 0x2264, 0x2320, 0x2321, 0x00F7, 0x2248,
    0x00B0, 0x2219, 0x00B7, 0x221A, 0x207F, 0x00B2, 0x25A0, 0x00A0
};

static std::string fortuneMessage() {
    FILE* msgPipe = popen("fortune", "r");
    if (msgPipe == 0)
        return "Can not open fortune pipe";

    std::string message;
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), msgPipe) != 0)
        message += buffer;

    pclose(msgPipe);

    if (message.empty())
        return "Fortune had nothing to say";

    return message;
}

static int cp437ByteForUnicode(unsigned int codepoint) {
    if (codepoint >= 0x20 && codepoint <= 0x7e)
        return int(codepoint);
    if (codepoint == '\t')
        return ' ';

    for (unsigned int i = 0; i < 128; i++)
        if (cp437Unicode[i] == codepoint)
            return int(i) + 128;

    return -1;
}

static int decodeUtf8(const std::string& line, unsigned int& offset, unsigned int& codepoint) {
    const unsigned char c = (unsigned char)line[offset];
    unsigned int remaining = 0;

    if (c < 0x80) {
        codepoint = c;
        offset++;
        return 1;
    }

    if (c >= 0xc2 && c <= 0xdf) {
        codepoint = c & 0x1f;
        remaining = 1;
    } else if (c >= 0xe0 && c <= 0xef) {
        codepoint = c & 0x0f;
        remaining = 2;
    } else if (c >= 0xf0 && c <= 0xf4) {
        codepoint = c & 0x07;
        remaining = 3;
    } else {
        return 0;
    }

    if (offset + remaining >= line.size())
        return 0;

    for (unsigned int i = 1; i <= remaining; i++) {
        const unsigned char continuation = (unsigned char)line[offset + i];
        if ((continuation & 0xc0) != 0x80)
            return 0;
        codepoint = (codepoint << 6) | (continuation & 0x3f);
    }

    if ((remaining == 2 && codepoint < 0x800)
        || (remaining == 3 && codepoint < 0x10000)
        || (codepoint >= 0xd800 && codepoint <= 0xdfff)
        || codepoint > 0x10ffff)
        return 0;

    offset += remaining + 1;
    return 1;
}

static int convertFortuneLineToCp437(const std::string& input, std::string& output,
    const char* path, int lineNumber) {
    output.clear();

    std::string line = input;
    if (!line.empty() && line[line.size() - 1] == '\r')
        line.erase(line.size() - 1);

    for (unsigned int offset = 0; offset < line.size();) {
        unsigned int characterOffset = offset;
        unsigned int codepoint = 0;

        if (!decodeUtf8(line, offset, codepoint)) {
            unsigned char raw = (unsigned char)line[characterOffset];
            if (raw >= 0x80) {
                output += char(raw);
                offset = characterOffset + 1;
                continue;
            }

            CTH_WARN("%s:%d: rejected silence message: illegal byte 0x%02x at column %u.\n",
                path, lineNumber, raw, characterOffset + 1);
            return 0;
        }

        int cp437 = cp437ByteForUnicode(codepoint);
        if (cp437 < 0) {
            CTH_WARN("%s:%d: rejected silence message: non-CP437 character U+%04X at column %u.\n",
                path, lineNumber, codepoint, characterOffset + 1);
            return 0;
        }

        output += char(cp437);
    }

    return !output.empty();
}

static int scanResourceDirectoryForFortune(const char* directory, std::string& path) {
    DIR* dir = opendir(directory);
    if (dir == 0)
        return 0;

    int found = 0;
    for (struct dirent* entry = readdir(dir); entry != 0; entry = readdir(dir)) {
        if (strcmp(entry->d_name, "fortune.txt") == 0) {
            path = std::string(directory);
            if (!path.empty() && path[path.size() - 1] != '/')
                path += '/';
            path += entry->d_name;
            found = 1;
            break;
        }
    }

    closedir(dir);
    return found;
}

static int findResourceFortuneFile(std::string& path) {
    static const char* resourceDirectories[] = {
        "./resources",
        CTH_LIBDIR,
        0
    };

    for (int i = 0; resourceDirectories[i] != 0; i++)
        if (scanResourceDirectoryForFortune(resourceDirectories[i], path))
            return 1;

    return 0;
}

static int loadFortuneFile(const char* path, std::vector<std::string>& loadedMessages) {
    std::ifstream input(path, std::ios::binary);
    if (!input)
        return 0;

    std::string line;
    int lineNumber = 0;
    while (std::getline(input, line)) {
        lineNumber++;

        std::string cp437Line;
        if (convertFortuneLineToCp437(line, cp437Line, path, lineNumber))
            loadedMessages.push_back(cp437Line);
    }

    return 1;
}

SilenceMessage::SilenceMessage()
    : messages()
    , initialized(0) { }

void SilenceMessage::loadDefaultMessages() {
    messages.clear();
    for (unsigned int i = 0; i < sizeof(defaultSilenceMessages) / sizeof(defaultSilenceMessages[0]); i++)
        messages.push_back(defaultSilenceMessages[i]);
}

void SilenceMessage::loadResourceFortuneMessages() {
    std::string fortunePath;
    if (findResourceFortuneFile(fortunePath)) {
        std::vector<std::string> loadedMessages;
        if (loadFortuneFile(fortunePath.c_str(), loadedMessages)) {
            if (!loadedMessages.empty()) {
                messages = loadedMessages;
                CTH_INFO("Loaded %d silence messages from `%s'.\n",
                    int(messages.size()), fortunePath.c_str());
            } else {
                CTH_WARN("fortune file `%s' contained no valid CP437 silence messages.\n",
                    fortunePath.c_str());
            }
        }
    }
}

void SilenceMessage::initialize() {
    if (initialized)
        return;

    initialized = 1;
    loadDefaultMessages();
    loadResourceFortuneMessages();
}

void SilenceMessage::loadFile(const char* fname) {
    FILE* file;
    char line[256];

    if ((fname == 0) || (*fname == '\0'))
        return;

    if ((file = fopen(fname, "r")) == 0) {
        CTH_ERRNO(errno, "Can't open quiet strings file `%s'.", fname);
        return;
    }

    std::vector<std::string> loadedMessages;
    while (fgets(line, sizeof(line), file) != 0)
        loadedMessages.push_back(line);

    fclose(file);

    if (loadedMessages.empty()) {
        CTH_WARN("silence strings file `%s' was empty.\n", fname);
        loadedMessages.push_back("Where is the music?");
    }

    messages = loadedMessages;
    initialized = 1;
}

std::string SilenceMessage::nextMessage() {
    initialize();

    if (messages.empty())
        return "Where is the music?";

    const std::string& message = messages[rand() % messages.size()];
    if (!message.empty() && message[0] == '@') {
        if (message.compare(0, 8, "@fortune") == 0)
            return fortuneMessage();
    }

    return message;
}
