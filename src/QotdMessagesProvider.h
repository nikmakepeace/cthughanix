#ifndef CTHUGHA_QOTD_MESSAGES_PROVIDER_H
#define CTHUGHA_QOTD_MESSAGES_PROVIDER_H

#include <memory>
#include <string>

class QotdMessagesProviderState;
struct MessagesConfig;

class QotdMessagesProvider {
    std::shared_ptr<QotdMessagesProviderState> state;

public:
    QotdMessagesProvider();
    ~QotdMessagesProvider();

    static const char* defaultServer();

    void configure(const MessagesConfig& config);
    void setServer(const char* server);
    void request();
    int takeMessage(std::string& message);
};

#endif
