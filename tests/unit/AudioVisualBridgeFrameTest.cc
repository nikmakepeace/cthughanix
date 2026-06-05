/** @file
 * Unit coverage for explicit AudioVisualBridge frame input.
 */

#include "cthugha.h"
#include "AudioVisualBridge.h"
#include "AudioAnalyzer.h"
#include "AudioFrame.h"
#include "AudioProcessing.h"
#include "AudioProcessor.h"

#include <assert.h>
#include <stdarg.h>

static int debugLoggingEnabled = 0;

int cth_log_enabled(int level) {
    return debugLoggingEnabled && (level == CTH_LOG_DEBUG);
}
int cth_log(int, const char*, ...) { return 0; }
int cth_log_context(int, const char*, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }
int cth_log_errno(int, const char*, ...) { return 0; }
double getTime() { return 0.0; }

static void fillConstant(AudioFrame& frame, int left, int right) {
    frame.samples = 1024;
    for (int i = 0; i < 1024; i++) {
        frame.raw[i][0] = char(left);
        frame.raw[i][1] = char(right);
    }
}

static void testBridgeProcessesSuppliedFrame() {
    AudioFrame frame;
    AcousticContext acousticContext;
    AudioProcessor processor;
    AudioProcessingState processingState;
    AudioProcessingSelector processingSelector(processingState, processor);
    AudioVisualBridge bridge(acousticContext, processingSelector, processor, 4);

    processingSelector.changeTo("none");
    fillConstant(frame, 3, 4);

    bridge.runFrame(frame);

    assert(frame.metrics.amplitudeLeft == 3);
    assert(frame.metrics.amplitudeRight == 4);
    assert(frame.metrics.amplitude == 3);
    assert(frame.metrics.noisy == 1);
    assert(frame.processedWaveData[0][0] == 3);
    assert(frame.processedWaveData[0][1] == 4);
    assert(acousticContext.intensity() > 0.0);
}

static void testBridgeDebugReportsAreInstanceLocal() {
    AudioFrame firstFrame;
    AudioFrame secondFrame;
    AcousticContext firstAcousticContext;
    AcousticContext secondAcousticContext;
    AudioProcessor firstProcessor;
    AudioProcessor secondProcessor;
    AudioProcessingState firstProcessingState;
    AudioProcessingState secondProcessingState;
    AudioProcessingSelector firstProcessingSelector(firstProcessingState,
        firstProcessor);
    AudioProcessingSelector secondProcessingSelector(secondProcessingState,
        secondProcessor);
    AudioVisualBridge first(firstAcousticContext, firstProcessingSelector,
        firstProcessor, 4);
    AudioVisualBridge second(secondAcousticContext, secondProcessingSelector,
        secondProcessor, 4);

    fillConstant(firstFrame, 2, 3);
    fillConstant(secondFrame, 4, 5);
    debugLoggingEnabled = 1;

    first.runFrame(firstFrame);
    assert(first.debugReportCount() == 1);
    assert(second.debugReportCount() == 0);

    second.runFrame(secondFrame);
    assert(first.debugReportCount() == 1);
    assert(second.debugReportCount() == 1);

    for (int i = 0; i < 24; i++)
        first.runFrame(firstFrame);
    assert(first.debugReportCount() == 16);
    assert(second.debugReportCount() == 1);
    debugLoggingEnabled = 0;
}

int main() {
    testBridgeProcessesSuppliedFrame();
    testBridgeDebugReportsAreInstanceLocal();
    return 0;
}
