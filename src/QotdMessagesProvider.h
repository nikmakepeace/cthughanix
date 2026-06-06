/** @file
 * Quote-of-the-Day quiet-message provider.
 */

#ifndef CTHUGHA_QOTD_MESSAGES_PROVIDER_H
#define CTHUGHA_QOTD_MESSAGES_PROVIDER_H

#include <memory>
#include <string>

class CountdownTimerFactory;
class QotdMessagesProviderState;
struct MessagesConfig;

/**
 * Asynchronous Quote-of-the-Day message fetcher.
 */
class QotdMessagesProvider {
    std::shared_ptr<QotdMessagesProviderState> state;

public:
    /** Creates a provider with default startup state. */
    QotdMessagesProvider();

    /** Releases provider state. In-flight fetches keep their shared state alive. */
    ~QotdMessagesProvider();

    /** @return Default QOTD server text from message configuration defaults. */
    static const char* defaultServer();

    /**
     * Applies startup message configuration.
     *
     * @param config Final message configuration from startup config.
     */
    void configure(const MessagesConfig& config);

    /**
     * Installs the timer factory used to bound asynchronous socket fetches.
     *
     * @param timerFactory Application-owned process timer factory.
     */
    void setTimerFactory(CountdownTimerFactory& timerFactory);

    /**
     * Sets the current QOTD server, or the configured default when blank.
     *
     * @param server Requested server text; may be NULL or blank.
     */
    void setServer(const char* server);

    /** Starts a bounded asynchronous prefetch when one is not already pending. */
    void request();

    /**
     * Consumes the prefetched message if one is available.
     *
     * @param message Output message text.
     * @return Nonzero when a message was available.
     */
    int takeMessage(std::string& message);
};

#endif
