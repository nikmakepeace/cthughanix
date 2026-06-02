#include "cthugha.h"
#include "MessageFormatValidator.h"

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

static int appendCp437ForUnicode(unsigned int codepoint, std::string& output) {
    if (codepoint == 0x2026) {
        output += "...";
        return 1;
    }
    if (codepoint == 0x2014 || codepoint == 0x2015) {
        output += "--";
        return 1;
    }
    if (codepoint == 0x2018 || codepoint == 0x2019
        || codepoint == 0x201A || codepoint == 0x201B) {
        output += "'";
        return 1;
    }
    if (codepoint == 0x201C || codepoint == 0x201D
        || codepoint == 0x201E || codepoint == 0x201F) {
        output += "\"";
        return 1;
    }

    int cp437 = cp437ByteForUnicode(codepoint);
    if (cp437 < 0)
        return 0;

    output += char(cp437);
    return 1;
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

static void warnRejectedByte(const char* sourceName, int lineNumber,
    unsigned int raw, unsigned int column) {
    CTH_WARN("%s:%d: rejected silence message: illegal byte 0x%02x at column %u.\n",
        sourceName, lineNumber, raw, column);
}

static void warnRejectedCodepoint(const char* sourceName, int lineNumber,
    unsigned int codepoint, unsigned int column) {
    CTH_WARN("%s:%d: rejected silence message: non-CP437 character U+%04X at column %u.\n",
        sourceName, lineNumber, codepoint, column);
}

static void warnRejectedLength(const char* sourceName, int lineNumber) {
    CTH_WARN("%s:%d: rejected silence message: longer than %d CP437 characters.\n",
        sourceName, lineNumber, int(MessageFormatValidator::MaxMessageCharacters));
}

int MessageFormatValidator::validateLine(const std::string& input, std::string& output,
    const char* sourceName, int lineNumber) {
    output.clear();

    const char* source = sourceName;
    if (source == 0 || source[0] == '\0')
        source = "silence message";

    std::string line = input;
    while (!line.empty() && (line[line.size() - 1] == '\r' || line[line.size() - 1] == '\n'))
        line.erase(line.size() - 1);

    for (unsigned int offset = 0; offset < line.size();) {
        unsigned int characterOffset = offset;
        unsigned int codepoint = 0;

        if (!decodeUtf8(line, offset, codepoint)) {
            unsigned char raw = (unsigned char)line[characterOffset];
            if (raw >= 0x80) {
                output += char(raw);
                if (output.size() > MessageFormatValidator::MaxMessageCharacters) {
                    warnRejectedLength(source, lineNumber);
                    output.clear();
                    return 0;
                }
                offset = characterOffset + 1;
                continue;
            }

            warnRejectedByte(source, lineNumber, raw, characterOffset + 1);
            return 0;
        }

        if (!appendCp437ForUnicode(codepoint, output)) {
            warnRejectedCodepoint(source, lineNumber, codepoint, characterOffset + 1);
            return 0;
        }

        if (output.size() > MessageFormatValidator::MaxMessageCharacters) {
            warnRejectedLength(source, lineNumber);
            output.clear();
            return 0;
        }
    }

    return !output.empty();
}
