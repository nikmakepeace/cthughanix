#include "cthugha.h"
#include "Configuration.h"
#include "QotdMessagesProvider.h"
#include "MessageFormatValidator.h"

#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifndef _WIN32
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#endif

enum QotdFetchResult {
    QotdFetchFailed,
    QotdFetchTimedOut,
    QotdFetchSucceeded
};

class QotdMessagesProviderState {
public:
    std::mutex mutex;
    int inFlight;
    int hasMessage;
    unsigned int generation;
    std::string defaultServer;
    std::string defaultPort;
    std::string server;
    std::string message;
    int prefetchTimeoutMs;

    QotdMessagesProviderState()
        : mutex()
        , inFlight(0)
        , hasMessage(0)
        , generation(0)
        , defaultServer()
        , defaultPort("17")
        , server()
        , message()
        , prefetchTimeoutMs(0) { }
};

static std::string qotdServerOrDefault(const char* server,
    const std::string& defaultServer) {
    if (server == 0)
        return defaultServer;

    std::string value(server);
    while (!value.empty() && (unsigned char)value[0] <= ' ')
        value.erase(0, 1);
    while (!value.empty() && (unsigned char)value[value.size() - 1] <= ' ')
        value.erase(value.size() - 1);

    return value.empty() ? defaultServer : value;
}

static int splitQotdServer(const std::string& server,
    const std::string& defaultPort, std::string& host, std::string& port) {
    host = server;
    port = defaultPort;

    if (server.empty())
        return 0;

    if (server[0] == '[') {
        std::string::size_type closeBracket = server.find(']');
        if (closeBracket == std::string::npos || closeBracket == 1)
            return 0;

        host = server.substr(1, closeBracket - 1);
        if (closeBracket + 1 < server.size()) {
            if (server[closeBracket + 1] != ':' || closeBracket + 2 == server.size())
                return 0;
            port = server.substr(closeBracket + 2);
        }
        return !host.empty() && !port.empty();
    }

    std::string::size_type firstColon = server.find(':');
    if (firstColon != std::string::npos && server.find(':', firstColon + 1) == std::string::npos) {
        host = server.substr(0, firstColon);
        port = server.substr(firstColon + 1);
    }

    return !host.empty() && !port.empty();
}

static int qotdPayloadWhitespace(unsigned char c) {
    return c <= ' ';
}

static int validateQotdPayload(const std::string& payload, const std::string& server,
    std::string& message) {
    std::string normalized;
    int previousWasSpace = 1;

    for (unsigned int i = 0; i < payload.size(); i++) {
        unsigned char c = (unsigned char)payload[i];
        if (qotdPayloadWhitespace(c)) {
            if (!previousWasSpace) {
                normalized += ' ';
                previousWasSpace = 1;
            }
            continue;
        }

        normalized += char(c);
        previousWasSpace = 0;
    }

    if (!normalized.empty() && normalized[normalized.size() - 1] == ' ')
        normalized.erase(normalized.size() - 1);

    std::string sourceName = std::string("qotd://") + server;
    return MessageFormatValidator::validateLine(normalized, message, sourceName.c_str(), 1);
}

#ifndef _WIN32

static int millisecondsUntil(std::chrono::steady_clock::time_point deadline) {
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (now >= deadline)
        return 0;

    return int(std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count());
}

static QotdFetchResult waitForSocket(int socketFd, int writeReady,
    std::chrono::steady_clock::time_point deadline) {
    int timeoutMs = millisecondsUntil(deadline);
    if (timeoutMs <= 0)
        return QotdFetchTimedOut;

    fd_set readSet;
    fd_set writeSet;
    FD_ZERO(&readSet);
    FD_ZERO(&writeSet);

    if (writeReady)
        FD_SET(socketFd, &writeSet);
    else
        FD_SET(socketFd, &readSet);

    struct timeval timeout;
    timeout.tv_sec = timeoutMs / 1000;
    timeout.tv_usec = (timeoutMs % 1000) * 1000;

    int rc = select(socketFd + 1,
        writeReady ? 0 : &readSet,
        writeReady ? &writeSet : 0,
        0,
        &timeout);

    if (rc > 0)
        return QotdFetchSucceeded;
    if (rc == 0)
        return QotdFetchTimedOut;
    return QotdFetchFailed;
}

static QotdFetchResult connectWithinBudget(int socketFd, const struct addrinfo* address,
    std::chrono::steady_clock::time_point deadline) {
    int flags = fcntl(socketFd, F_GETFL, 0);
    if (flags < 0)
        return QotdFetchFailed;
    if (fcntl(socketFd, F_SETFL, flags | O_NONBLOCK) < 0)
        return QotdFetchFailed;

    int rc = connect(socketFd, address->ai_addr, address->ai_addrlen);
    if (rc == 0)
        return QotdFetchSucceeded;

    if (errno != EINPROGRESS)
        return QotdFetchFailed;

    QotdFetchResult waitResult = waitForSocket(socketFd, 1, deadline);
    if (waitResult != QotdFetchSucceeded)
        return waitResult;

    int socketError = 0;
    socklen_t socketErrorLength = sizeof(socketError);
    if (getsockopt(socketFd, SOL_SOCKET, SO_ERROR, &socketError, &socketErrorLength) < 0)
        return QotdFetchFailed;

    return socketError == 0 ? QotdFetchSucceeded : QotdFetchFailed;
}

static QotdFetchResult readWithinBudget(int socketFd, std::string& payload,
    std::chrono::steady_clock::time_point deadline) {
    static const unsigned int maxPayloadBytes =
        MessageFormatValidator::MaxMessageCharacters * 4 + 8;
    char buffer[512];

    while (payload.size() < maxPayloadBytes) {
        QotdFetchResult waitResult = waitForSocket(socketFd, 0, deadline);
        if (waitResult != QotdFetchSucceeded)
            return waitResult;

        unsigned int remainingBytes = maxPayloadBytes - payload.size();
        unsigned int requestedBytes = remainingBytes < sizeof(buffer)
            ? remainingBytes
            : sizeof(buffer);
        ssize_t received = recv(socketFd, buffer, requestedBytes, 0);
        if (received > 0) {
            payload.append(buffer, buffer + received);
            continue;
        }

        if (received == 0)
            return payload.empty() ? QotdFetchFailed : QotdFetchSucceeded;

        if (errno == EAGAIN || errno == EWOULDBLOCK)
            continue;

        return QotdFetchFailed;
    }

    CTH_WARN("QOTD prefetch rejected: response exceeded bounded read size.\n");
    return QotdFetchFailed;
}

static QotdFetchResult fetchQotdPayload(const std::string& server,
    const std::string& defaultPort, int prefetchTimeoutMs, std::string& payload) {
    std::string qotdHost;
    std::string qotdPort;
    if (!splitQotdServer(server, defaultPort, qotdHost, qotdPort)) {
        CTH_WARN("QOTD prefetch failed: invalid qotd-server `%s'.\n", server.c_str());
        return QotdFetchFailed;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* addresses = 0;
    int rc = getaddrinfo(qotdHost.c_str(), qotdPort.c_str(), &hints, &addresses);
    if (rc != 0)
        return QotdFetchFailed;

    const std::chrono::steady_clock::time_point deadline =
        std::chrono::steady_clock::now() + std::chrono::milliseconds(prefetchTimeoutMs);

    QotdFetchResult fetchResult = QotdFetchFailed;
    for (const struct addrinfo* address = addresses; address != 0; address = address->ai_next) {
        if (millisecondsUntil(deadline) <= 0) {
            fetchResult = QotdFetchTimedOut;
            break;
        }

        int socketFd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (socketFd < 0)
            continue;

        QotdFetchResult connectResult = connectWithinBudget(socketFd, address, deadline);
        if (connectResult == QotdFetchSucceeded)
            fetchResult = readWithinBudget(socketFd, payload, deadline);
        else
            fetchResult = connectResult;

        close(socketFd);
        if (fetchResult == QotdFetchSucceeded || fetchResult == QotdFetchTimedOut)
            break;
    }

    freeaddrinfo(addresses);
    return fetchResult;
}

#else

static QotdFetchResult fetchQotdPayload(const std::string& server,
    const std::string& defaultPort, int prefetchTimeoutMs, std::string& payload) {
    (void)server;
    (void)defaultPort;
    (void)prefetchTimeoutMs;
    (void)payload;
    return QotdFetchFailed;
}

#endif

static void fetchQotdMessage(std::shared_ptr<QotdMessagesProviderState> state,
    std::string server, std::string defaultPort, int prefetchTimeoutMs,
    unsigned int generation) {
    std::string payload;
    std::string message;
    QotdFetchResult fetchResult = fetchQotdPayload(server, defaultPort,
        prefetchTimeoutMs, payload);
    if (fetchResult == QotdFetchTimedOut)
        CTH_WARN("QOTD prefetch timed out after %d ms.\n", prefetchTimeoutMs);

    int success = fetchResult == QotdFetchSucceeded
        && validateQotdPayload(payload, server, message);

    std::lock_guard<std::mutex> lock(state->mutex);
    if (success && state->generation == generation) {
        state->message = message;
        state->hasMessage = 1;
    }
    state->inFlight = 0;
}

QotdMessagesProvider::QotdMessagesProvider()
    : state(std::make_shared<QotdMessagesProviderState>()) { }

QotdMessagesProvider::~QotdMessagesProvider() { }

const char* QotdMessagesProvider::defaultServer() {
    static const MessagesConfig defaultConfig;
    return defaultConfig.qotdServer.c_str();
}

void QotdMessagesProvider::configure(const MessagesConfig& config) {
    std::lock_guard<std::mutex> lock(state->mutex);
    state->defaultServer = config.qotdServer;
    state->defaultPort = config.qotdPort;
    state->prefetchTimeoutMs = config.qotdPrefetchTimeoutMs;
    if (state->server.empty())
        state->server = state->defaultServer;
}

void QotdMessagesProvider::setServer(const char* server) {
    std::lock_guard<std::mutex> lock(state->mutex);
    std::string nextServer = qotdServerOrDefault(server, state->defaultServer);
    if (state->server == nextServer)
        return;

    state->server = nextServer;
    state->message.clear();
    state->hasMessage = 0;
    state->generation++;
}

void QotdMessagesProvider::request() {
    std::shared_ptr<QotdMessagesProviderState> currentState = state;
    std::string server;
    std::string defaultPort;
    int prefetchTimeoutMs = 0;
    unsigned int generation = 0;

    {
        std::lock_guard<std::mutex> lock(currentState->mutex);
        if (currentState->inFlight || currentState->hasMessage)
            return;
        currentState->inFlight = 1;
        server = currentState->server;
        defaultPort = currentState->defaultPort;
        prefetchTimeoutMs = currentState->prefetchTimeoutMs;
        generation = currentState->generation;
    }

    std::thread(fetchQotdMessage, currentState, server, defaultPort,
        prefetchTimeoutMs, generation).detach();
}

int QotdMessagesProvider::takeMessage(std::string& message) {
    std::lock_guard<std::mutex> lock(state->mutex);
    if (!state->hasMessage)
        return 0;

    message = state->message;
    state->message.clear();
    state->hasMessage = 0;
    return 1;
}
