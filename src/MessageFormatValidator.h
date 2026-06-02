#ifndef CTHUGHA_MESSAGE_FORMAT_VALIDATOR_H
#define CTHUGHA_MESSAGE_FORMAT_VALIDATOR_H

#include <string>

class MessageFormatValidator {
public:
    enum {
        MaxMessageCharacters = 512
    };

    static int validateLine(const std::string& input, std::string& output,
        const char* sourceName, int lineNumber);
};

#endif
