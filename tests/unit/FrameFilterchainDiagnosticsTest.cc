/** @file
 * Unit coverage for FrameFilterchain injected diagnostics.
 */

#include "FrameFilterchain.h"
#include "FrameRenderTarget.h"
#include "ProcessServices.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <vector>

class RecordingLogSink : public LogSink {
public:
    int debugWrites;
    int traceWrites;
    std::string lastDebug;
    std::vector<std::string> traceMessages;

    RecordingLogSink()
        : debugWrites(0)
        , traceWrites(0)
        , lastDebug()
        , traceMessages() { }

    virtual int enabled(int level) const {
        return level <= CTH_LOG_TRACE;
    }

protected:
    virtual void write(int level, const char*, int, const char* fmt, va_list ap) {
        if (level == CTH_LOG_DEBUG) {
            char message[512];
            vsnprintf(message, sizeof(message), fmt, ap);
            lastDebug = message;
            debugWrites++;
        }
        if (level == CTH_LOG_TRACE) {
            char message[512];
            vsnprintf(message, sizeof(message), fmt, ap);
            traceMessages.push_back(message);
            traceWrites++;
        }
    }
};

class NoOpFrameFilter : public FrameFilter {
public:
    int executions;

    NoOpFrameFilter()
        : executions(0) { }

    virtual const char* name() const { return "test-filter"; }

    virtual void execute(FrameFilterFrame&) { executions++; }
};

static void testFrameFilterchainUsesInjectedLogSink() {
    RecordingLogSink log;
    FrameFilterchain filterchain(log);
    NoOpFrameFilter filter;

    filterchain.add(7, &filter);
    assert(log.debugWrites == 1);
    assert(log.lastDebug.find("filter=test-filter") != std::string::npos);

    std::vector<unsigned int> stages;
    stages.push_back(7);
    filterchain.setStageSequence(stages);
    assert(log.debugWrites == 2);

    assert(filterchain.setStageMode(7, FrameFilterEnabled) == 1);
    assert(log.traceWrites == 1);

    FrameRenderTarget target;
    FrameGeneratorContext context;
    assert(filterchain.stageEnabled(7) == 1);
    assert(filterchain.setStageEnabled(7, 0) == 1);
    assert(filterchain.stageEnabled(7) == 0);
    filterchain.run(target, context);
    assert(filter.executions == 0);
    assert(filterchain.setStageEnabled(7, 1) == 1);
    assert(filterchain.stageEnabled(7) == 1);
    filterchain.run(target, context);
    assert(filter.executions == 1);

    int sawFilterTiming = 0;
    int sawRunTiming = 0;
    for (std::vector<std::string>::const_iterator it = log.traceMessages.begin();
         it != log.traceMessages.end(); ++it) {
        if (it->find("filter-ms=") != std::string::npos
            && it->find("filter=test-filter") != std::string::npos)
            sawFilterTiming = 1;
        if (it->find("run-ms=") != std::string::npos)
            sawRunTiming = 1;
    }

    assert(sawFilterTiming);
    assert(sawRunTiming);
}

int main() {
    testFrameFilterchainUsesInjectedLogSink();
    return 0;
}
