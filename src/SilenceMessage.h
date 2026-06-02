#ifndef __SILENCE_MESSAGE_H
#define __SILENCE_MESSAGE_H

#include <string>
#include <vector>

class SilenceMessage {
    std::vector<std::string> messages;
    int initialized;

    void loadDefaultMessages();
    void loadResourceFortuneMessages();

public:
    SilenceMessage();

    void initialize();
    void loadFile(const char* fname);
    std::string nextMessage();
};

#endif
