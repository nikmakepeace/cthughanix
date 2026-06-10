/** @file
 * Optional local control service owned by Application.
 */

#include "ControlService.h"

#include "ControlCommandMapper.h"
#include "ControlSnapshot.h"
#include "ControlTransport.h"
#include "ProcessServices.h"
#include "RuntimeCommandSink.h"

#include <stdio.h>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <sys/types.h>
#include <unistd.h>
#endif

namespace {

static int controlMessageId(const ControlJsonValue& message) {
    const ControlJsonValue* id = message.member("id");
    return id != 0 ? int(id->asNumber(0)) : 0;
}

static std::string processInstanceId() {
    char text[64];
#ifdef _WIN32
    snprintf(text, sizeof(text), "%lu", unsigned long(GetCurrentProcessId()));
#else
    snprintf(text, sizeof(text), "%ld", long(getpid()));
#endif
    return text;
}

static bool writeMessage(ControlStream& stream, const ControlJsonValue& message,
    std::string* error) {
    std::string line;
    if (!serializeControlJsonLine(message, &line, error))
        return false;
    return stream.writeAll(line.data(), line.size(), 25, error)
        == ControlIoReady;
}

}

bool SystemControlPanelProcessLauncher::launchPanel(
    const std::string& endpoint, std::string* error) {
#ifdef _WIN32
    std::string command = "cthugha-panel.exe --control-endpoint \"" + endpoint + "\"";
    STARTUPINFOA startupInfo;
    PROCESS_INFORMATION processInfo;
    ZeroMemory(&startupInfo, sizeof(startupInfo));
    ZeroMemory(&processInfo, sizeof(processInfo));
    startupInfo.cb = sizeof(startupInfo);
    std::vector<char> mutableCommand(command.begin(), command.end());
    mutableCommand.push_back('\0');
    if (!CreateProcessA(NULL, mutableCommand.data(), NULL, NULL, FALSE, 0,
            NULL, NULL, &startupInfo, &processInfo)) {
        if (error != 0)
            *error = "CreateProcess failed for cthugha-panel";
        return false;
    }
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    return true;
#else
    pid_t pid = fork();
    if (pid < 0) {
        if (error != 0)
            *error = "fork failed for cthugha-panel";
        return false;
    }
    if (pid == 0) {
        execlp("cthugha-panel", "cthugha-panel", "--control-endpoint",
            endpoint.c_str(), static_cast<char*>(0));
        _exit(127);
    }
    return true;
#endif
}

ControlService::InboundItem::InboundItem(InboundType type_)
    : type(type_)
    , message() { }

ControlService::InboundItem::InboundItem(const ControlJsonValue& message_)
    : type(InboundMessage)
    , message(message_) { }

ControlService::ControlService(RuntimeCommandSink& runtimeCommands_,
    RuntimeConfigRegistry& runtimeConfigRegistry_,
    SceneVisualSelections& sceneVisualSelections_, LogSink& log_)
    : runtimeCommands(runtimeCommands_)
    , runtimeConfigRegistry(runtimeConfigRegistry_)
    , sceneVisualSelections(sceneVisualSelections_)
    , log(log_)
    , ownedProcessLauncher(new SystemControlPanelProcessLauncher())
    , processLauncher(*ownedProcessLauncher)
    , listenerValue()
    , workerThread()
    , mutex()
    , inbound()
    , outbound()
    , stopRequested(0)
    , clientConnectedValue(0)
    , launchPending(0)
    , dirtyValue(0)
    , revisionValue(0)
    , maxOutboundMessages(32) { }

ControlService::ControlService(RuntimeCommandSink& runtimeCommands_,
    RuntimeConfigRegistry& runtimeConfigRegistry_,
    SceneVisualSelections& sceneVisualSelections_, LogSink& log_,
    ControlPanelProcessLauncher& processLauncher_)
    : runtimeCommands(runtimeCommands_)
    , runtimeConfigRegistry(runtimeConfigRegistry_)
    , sceneVisualSelections(sceneVisualSelections_)
    , log(log_)
    , ownedProcessLauncher()
    , processLauncher(processLauncher_)
    , listenerValue()
    , workerThread()
    , mutex()
    , inbound()
    , outbound()
    , stopRequested(0)
    , clientConnectedValue(0)
    , launchPending(0)
    , dirtyValue(0)
    , revisionValue(0)
    , maxOutboundMessages(32) { }

ControlService::~ControlService() {
    stop();
}

bool ControlService::start(const std::string& instanceId, std::string* error) {
    if (listenerValue.get() != 0)
        return true;

    listenerValue = createControlListener(
        instanceId.empty() ? processInstanceId() : instanceId, error);
    if (listenerValue.get() == 0)
        return false;

    stopRequested = 0;
    workerThread = std::thread(&ControlService::workerMain, this);
    log.info("Control endpoint: %s\n", listenerValue->endpoint().text.c_str());
    return true;
}

void ControlService::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        stopRequested = 1;
    }
    if (listenerValue.get() != 0)
        listenerValue->close();
    if (workerThread.joinable())
        workerThread.join();
    listenerValue.reset();
}

std::string ControlService::endpoint() const {
    if (listenerValue.get() == 0)
        return "";
    return listenerValue->endpoint().text;
}

bool ControlService::clientConnected() const {
    std::lock_guard<std::mutex> lock(mutex);
    return clientConnectedValue != 0;
}

void ControlService::runtimeStateChanged() {
    std::lock_guard<std::mutex> lock(mutex);
    dirtyValue = 1;
}

void ControlService::launchControlPanel() {
    std::string endpointText = endpoint();
    if (endpointText.empty())
        return;

    {
        std::lock_guard<std::mutex> lock(mutex);
        if (clientConnectedValue || launchPending)
            return;
        launchPending = 1;
    }

    std::string error;
    if (!processLauncher.launchPanel(endpointText, &error)) {
        log.warn("Could not launch control panel: %s\n", error.c_str());
        std::lock_guard<std::mutex> lock(mutex);
        launchPending = 0;
    }
}

int ControlService::workerStopRequested() const {
    std::lock_guard<std::mutex> lock(mutex);
    return stopRequested;
}

void ControlService::workerSetClientConnected(int connected) {
    std::lock_guard<std::mutex> lock(mutex);
    clientConnectedValue = connected;
    if (connected)
        launchPending = 0;
}

void ControlService::enqueueInbound(const InboundItem& item) {
    std::lock_guard<std::mutex> lock(mutex);
    inbound.push_back(item);
}

void ControlService::enqueueOutbound(const ControlJsonValue& message) {
    std::lock_guard<std::mutex> lock(mutex);
    while (outbound.size() >= maxOutboundMessages)
        outbound.pop_front();
    outbound.push_back(message);
}

void ControlService::publishCatalogs() {
    enqueueOutbound(buildControlCatalogSnapshot(sceneVisualSelections));
}

void ControlService::publishState() {
    revisionValue++;
    enqueueOutbound(buildControlStateSnapshot(
        runtimeConfigRegistry, revisionValue));
}

void ControlService::processMessage(const ControlJsonValue& message) {
    int id = controlMessageId(message);
    ControlMappedCommand mapped;
    std::string errorCode;
    std::string errorMessage;
    if (!controlCommandFromJson(message, &mapped, &errorCode, &errorMessage)) {
        enqueueOutbound(controlErrorMessage(id, errorCode, errorMessage));
        return;
    }

    RuntimeChangeSet changes = runtimeCommands.apply(mapped.command);
    if (changes.any())
        runtimeStateChanged();
    enqueueOutbound(controlAckMessage(id));
}

void ControlService::serviceFrame(int maxCommands) {
    std::deque<InboundItem> localInbound;
    int publishDirty = 0;
    {
        std::lock_guard<std::mutex> lock(mutex);
        localInbound.swap(inbound);
        publishDirty = dirtyValue;
        dirtyValue = 0;
    }

    int commandsApplied = 0;
    for (std::deque<InboundItem>::const_iterator it = localInbound.begin();
         it != localInbound.end(); ++it) {
        if (it->type == InboundConnected) {
            publishCatalogs();
            publishState();
            continue;
        }
        if (it->type == InboundDisconnected) {
            std::lock_guard<std::mutex> lock(mutex);
            launchPending = 0;
            continue;
        }
        if (maxCommands >= 0 && commandsApplied >= maxCommands) {
            enqueueInbound(*it);
            continue;
        }
        processMessage(it->message);
        commandsApplied++;
        publishDirty = 1;
    }

    if (publishDirty && clientConnected())
        publishState();
}

void ControlService::workerMain() {
    std::unique_ptr<ControlStream> client;
    ControlJsonLineReader reader;
    while (!workerStopRequested()) {
        std::string error;
        if (client.get() == 0) {
            if (listenerValue.get() == 0)
                break;
            client = listenerValue->accept(50, &error);
            if (client.get() != 0) {
                reader.clear();
                workerSetClientConnected(1);
                enqueueInbound(InboundItem(InboundConnected));
            }
            continue;
        }

        char buffer[4096];
        ControlReadResult read = client->readSome(
            buffer, sizeof(buffer), 10, &error);
        if (read.status == ControlIoReady) {
            std::vector<ControlJsonValue> messages;
            if (!reader.feed(buffer, read.bytes, &messages, &error)) {
                client->close();
                client.reset();
                workerSetClientConnected(0);
                enqueueInbound(InboundItem(InboundDisconnected));
                continue;
            }
            for (std::vector<ControlJsonValue>::const_iterator it
                     = messages.begin();
                 it != messages.end(); ++it)
                enqueueInbound(InboundItem(*it));
        } else if (read.status == ControlIoClosed
            || read.status == ControlIoError) {
            client->close();
            client.reset();
            workerSetClientConnected(0);
            enqueueInbound(InboundItem(InboundDisconnected));
            continue;
        }

        std::deque<ControlJsonValue> localOutbound;
        {
            std::lock_guard<std::mutex> lock(mutex);
            localOutbound.swap(outbound);
        }
        for (std::deque<ControlJsonValue>::const_iterator it
                 = localOutbound.begin();
             it != localOutbound.end(); ++it) {
            if (!writeMessage(*client, *it, &error)) {
                client->close();
                client.reset();
                workerSetClientConnected(0);
                enqueueInbound(InboundItem(InboundDisconnected));
                break;
            }
        }
    }

    if (client.get() != 0)
        client->close();
    workerSetClientConnected(0);
}
