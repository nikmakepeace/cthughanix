#include "DisplayRuntime.h"
#include "FramePalette.h"

#include <assert.h>

class FakeBackend : public DisplayBackend {
public:
    int processEventCalls;
    mutable int outputSizeCalls;
    int presentCalls;
    DisplayEventStats nextStats;
    PixelSize outputSizeValue;
    const IndexedDisplayFrame* presentedFrame;
    FramePalette* presentedPalette;
    DisplayViewport presentedViewport;
    int presentedNeedsFullCopy;
    int presentedNeedsBorderClear;

    FakeBackend()
        : processEventCalls(0)
        , outputSizeCalls(0)
        , presentCalls(0)
        , nextStats()
        , outputSizeValue(12, 10)
        , presentedFrame(0)
        , presentedPalette(0)
        , presentedViewport()
        , presentedNeedsFullCopy(0)
        , presentedNeedsBorderClear(0) {
    }

    virtual DisplayEventStats processEvents() {
        processEventCalls++;
        return nextStats;
    }

    virtual PixelSize outputSize() const {
        outputSizeCalls++;
        return outputSizeValue;
    }

    virtual void present(const DisplayPresentation& presentation) {
        presentCalls++;
        presentedFrame = &presentation.frame;
        presentedPalette = presentation.framePalette;
        presentedViewport = presentation.viewport;
        presentedNeedsFullCopy = presentation.needsFullCopy;
        presentedNeedsBorderClear = presentation.needsBorderClear;
    }
};

static DisplayViewport fixedViewport() {
    DisplayViewport viewport;
    viewport.frameSize = PixelSize(4, 3);
    viewport.windowSize = PixelSize(12, 10);
    viewport.drawSize = PixelSize(4, 3);
    viewport.destination = PixelRect(4, 3, 4, 3);
    viewport.scaleMode = SCALE_MODE_FIXED_ZOOM;
    viewport.requestedZoom = 1;
    viewport.effectiveZoom = 1;
    return viewport;
}

static void testProcessEventsDelegatesToBackend() {
    FakeBackend backend;
    backend.nextStats.eventCount = 5;
    backend.nextStats.resizeEvents = 2;
    backend.nextStats.exposeEvents = 1;
    DisplayRuntime runtime(backend);

    DisplayEventStats stats = runtime.processEvents();

    assert(backend.processEventCalls == 1);
    assert(stats.eventCount == 5);
    assert(stats.resizeEvents == 2);
    assert(stats.exposeEvents == 1);
}

static void testOutputSizeDelegatesToBackend() {
    FakeBackend backend;
    backend.outputSizeValue = PixelSize(20, 13);
    DisplayRuntime runtime(backend);

    PixelSize outputSize = runtime.outputSize();

    assert(backend.outputSizeCalls == 1);
    assert(outputSize == PixelSize(20, 13));
}

static void testPresentationRequestCarriesFrameViewportAndFlags() {
    FakeBackend backend;
    DisplayRuntime runtime(backend);
    IndexedDisplayFrame frame;
    FramePalette palette;
    DisplayViewport viewport = fixedViewport();
    frame.resize(4, 3, 6);
    frame.setFramePalette(&palette);

    runtime.present(frame, viewport, 1, 0);

    assert(backend.presentCalls == 1);
    assert(backend.presentedFrame == &frame);
    assert(backend.presentedPalette == &palette);
    assert(backend.presentedViewport.frameSize == viewport.frameSize);
    assert(backend.presentedViewport.windowSize == viewport.windowSize);
    assert(backend.presentedViewport.drawSize == viewport.drawSize);
    assert(backend.presentedViewport.destination == viewport.destination);
    assert(backend.presentedViewport.scaleMode == viewport.scaleMode);
    assert(backend.presentedViewport.requestedZoom == viewport.requestedZoom);
    assert(backend.presentedViewport.effectiveZoom == viewport.effectiveZoom);
    assert(backend.presentedNeedsFullCopy == 1);
    assert(backend.presentedNeedsBorderClear == 0);
}

int main() {
    testProcessEventsDelegatesToBackend();
    testOutputSizeDelegatesToBackend();
    testPresentationRequestCarriesFrameViewportAndFlags();
    return 0;
}
