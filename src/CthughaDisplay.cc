#include "cthugha.h"
#include "CthughaDisplay.h"
#include "CthughaBuffer.h"
#include "display.h"
#include "DisplayDevice.h"
#include "FrameClock.h"
#include "cth_buffer.h"
#include "disp-sys.h"
#include "imath.h"
#include "Interface.h"
#include "IndexedFrame.h"
#include "Screen.h"
#include "ViewportPolicy.h"
#include "ViewportPresentation.h"

#include <stdint.h>

// The active display coordinator.  The selected frontend supplies
// newCthughaDisplay(), so this points at the X11 display subclass.
CthughaDisplay* cthughaDisplay = NULL;

CthughaDisplay::~CthughaDisplay() { }

OptionInt maxFramesPerSecond("maxFPS", DEFAULT_MAX_FRAMES_PER_SECOND);
OptionOnOff showFPS("show-fps", DEFAULT_SHOW_FPS_ENABLED);

OptionInt zoom("zoom", DEFAULT_ZOOM_MODE, ZOOM_MODE_MAX_EXCLUSIVE);
xy draw_size(0, 0); /* size of the drawn image (including zoom) */

double displayStart;

// Frame clock shared by animation, sound processing, and display effects.
// nextFrame() updates these before the rest of the frame runs.
double now = 0;
double deltaT = 0;

static SystemFrameTimeSource systemFrameTimeSource;
static FrameClock frameClock(systemFrameTimeSource);

class VisualFrameView {
public:
    int width() const {
        return cthughaDisplay->sourceWidth();
    }

    int height() const {
        return cthughaDisplay->sourceHeight();
    }

    int size() const {
        return width() * height();
    }

    int displayWidth() const {
        return cthughaDisplay->displayFrameWidth();
    }

    int displayHeight() const {
        return cthughaDisplay->displayFrameHeight();
    }
};

static VisualFrameView visualBuffer() {
    return VisualFrameView();
}

class GlobalPresentationScreenSelection : public PresentationScreenSelection {
public:
    virtual ScreenEntry* current() {
        return (ScreenEntry*)screen.current();
    }

    virtual void change(int by, int doSave) {
        screen.change(by, doSave);
    }
};

CthughaDisplay::CthughaDisplay()
    : sourceFrame(0)
    , indexedDisplayFrameValue()
    , presentationComposer()
    , displayViewportValue()
    , buffer0(0)
    , displayStart(0)
    , frames(0)
    , visualLatencyEstimate(0)
    , buffer(0)
    , bufferWidth(0)
    , needsClear(1)
    , fps(0) {

    displayStart = frameClock.sample();
}

void CthughaDisplay::present(const IndexedFrame& frame) {
    if (!frame.valid()) {
        sourceFrame = NULL;
        return;
    }

    sourceFrame = &frame;
    if (displayDevice != NULL && frame.framePalette != NULL)
        displayDevice->setFramePalette(frame.framePalette);
    (*this)();
}

const unsigned char* CthughaDisplay::sourcePixels() const {
    if (sourceFrame != NULL && sourceFrame->valid())
        return sourceFrame->pixels;

    // Legacy fallback for code paths that still call operator() without first
    // publishing an IndexedFrame through present().
    return CthughaBuffer::current->passivePixels();
}

int CthughaDisplay::sourceWidth() const {
    if (sourceFrame != NULL && sourceFrame->valid())
        return sourceFrame->width;

    return CthughaBuffer::current->width();
}

int CthughaDisplay::sourceHeight() const {
    if (sourceFrame != NULL && sourceFrame->valid())
        return sourceFrame->height;

    return CthughaBuffer::current->height();
}

int CthughaDisplay::sourcePitch() const {
    if (sourceFrame != NULL && sourceFrame->valid())
        return sourceFrame->pitch;

    return CthughaBuffer::current->width();
}

int CthughaDisplay::sourceSize() const {
    return sourceWidth() * sourceHeight();
}

const IndexedDisplayFrame& CthughaDisplay::indexedDisplayFrame() const {
    return indexedDisplayFrameValue;
}

const DisplayViewport& CthughaDisplay::displayViewport() const {
    return displayViewportValue;
}

int CthughaDisplay::displayFrameWidth() const {
    if (indexedDisplayFrameValue.valid())
        return indexedDisplayFrameValue.width();

    return 2 * sourceWidth();
}

int CthughaDisplay::displayFrameHeight() const {
    if (indexedDisplayFrameValue.valid())
        return indexedDisplayFrameValue.height();

    return 2 * sourceHeight();
}

void CthughaDisplay::indexedPixelsWillMove(unsigned char*) {
}

void CthughaDisplay::indexedFrameGeometryChanged() {
    needsClear = 1;
}

const IndexedDisplayFrame& CthughaDisplay::composePresentationFrame(
    PresentationScreenSelection& screenSelection) {
    IndexedFrame screenSource(sourcePixels(), sourceWidth(), sourceHeight(), sourcePitch(),
        sourceFrame != NULL ? sourceFrame->framePalette : NULL);
    const IndexedDisplayFrame& frame = presentationComposer.compose(screenSource,
        indexedDisplayFrameValue, screenSelection, now, deltaT, fps, this);
    buffer0 = indexedDisplayFrameValue.pixels();
    return frame;
}

const IndexedDisplayFrame& CthughaDisplay::composePresentationFrame() {
    GlobalPresentationScreenSelection screenSelection;
    return composePresentationFrame(screenSelection);
}

/*
 * clear the border around the image on the screen
 */
int CthughaDisplay::clearBorder() {

    if (displayDevice->textOnScreen || needsClear) {
        PixelRect window = ViewportPresentation::fullCopyRect(displayViewportValue);
        PixelRect draw = ViewportPresentation::drawCopyRect(displayViewportValue);
        int bwidth = draw.x; /* width of left border */
        int bheight = draw.y; /* height of upper border */
        int rightWidth = window.width - draw.right();
        int lowerHeight = window.height - draw.bottom();
        if (rightWidth < 0)
            rightWidth = 0;
        if (lowerHeight < 0)
            lowerHeight = 0;

        /* upper border */
        displayDevice->clearBox(0, 0, window.width, bheight);
        /* left border */
        displayDevice->clearBox(0, bheight, bwidth, draw.height);
        /* right border */
        displayDevice->clearBox(draw.right(), bheight, rightWidth, draw.height);
        /* lower border */
        displayDevice->clearBox(0, draw.bottom(), window.width, lowerHeight);

        /* if text is on screen, we have to clean in the next iteration,
           because the text may be removed then */
        needsClear = (displayDevice->textOnScreen) ? 1 : 0;

        // Border clearing touches pixels outside the normal image rectangle,
        // so partial-copy display devices must refresh the full frame.
        displayDevice->needsFullCopy = 1;
    }
    return 0;
}

void CthughaDisplay::checkZoom() {
    ViewportPolicy policy;
    DisplayViewport previous = displayViewportValue;
    displayViewportValue = policy.viewportFor(
        PixelSize(visualBuffer().displayWidth(), visualBuffer().displayHeight()),
        PixelSize::fromXy(disp_size), int(zoom));

    for (int i = 0; i < displayViewportValue.reductionCount; ++i)
        CTH_ERROR("Zoom factor is set too high for current display size. reducing.\n");

    if (displayViewportValue.effectiveZoom != int(zoom))
        zoom.setValue(displayViewportValue.effectiveZoom);

    draw_size = displayViewportValue.drawSize.toXy();
    if (displayViewportValue.requiresBorderClearFrom(previous))
        needsClear = 1;
}

void CthughaDisplay::updateFPS() {
    /*
     * compute frames/second, of at least 3 frames or 0.1 second
     */
    double i = now - displayStart;
    if ((i > 0.1) && (frames > 2))
        fps = double(frames) / i;
    frames++;

    CTH_TRACE("updateFPS deltaT-ms=%.3f fps=%.3f frames=%d\n",
        "frame pacing", deltaT * 1000.0, fps, frames);
}

void CthughaDisplay::resetFPS() {
    displayStart = frameClock.sample(); // restart the averaging window from this instant
    frames = 0;
}

void CthughaDisplay::observeVisualLatency(double seconds) {
    double previous = visualLatencyEstimate;
    double alpha = 0.1;

    if (seconds < 0)
        seconds = 0;

    if (visualLatencyEstimate <= 0)
        visualLatencyEstimate = seconds;
    else
        visualLatencyEstimate = visualLatencyEstimate * (1.0 - alpha) + seconds * alpha;

    CTH_TRACE("visual-latency observed-ms=%.3f previous-ms=%.3f alpha=%.3f estimate-ms=%.3f\n",
        "display timing", seconds * 1000.0, previous * 1000.0, alpha,
        visualLatencyEstimate * 1000.0);
}

double CthughaDisplay::visualLatencySeconds() const {
    return visualLatencyEstimate;
}

// Start a new frame by publishing a stable timestamp for all modules that run
// during this frame, then update FPS accounting.
void CthughaDisplay::nextFrame() {

    double previousNow = now;
    frameClock.beginFrame();
    frameClock.publish(now, deltaT);
    CTH_TRACE("nextFrame previous-now=%.6f sampled-now=%.6f raw-delta-ms=%.3f\n",
        "frame pacing", previousNow, now, deltaT * 1000.0);

    updateFPS();
}

const char* CthughaDisplay::status() {
    static char txt[512];

    snprintf(txt, sizeof(txt), "fps: %5.2f ", fps);
    return txt;
}
