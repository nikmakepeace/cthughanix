#include "cthugha.h"
#include "FrameRenderTarget.h"
#include "imath.h"

static const int hiddenBorderRowsPerSide = 3; // Unit: rows; used above and below visible pixels by BorderFilter and flame feedback.

FrameRenderTarget::FrameRenderTarget()
    : activeAllocation(0)
    , passiveAllocation(0)
    , activeBuffer(0)
    , passiveBuffer(0)
    , widthValue(160)
    , heightValue(100) { }

FrameRenderTarget::~FrameRenderTarget() {
    delete[] activeAllocation;
    delete[] passiveAllocation;
}

int FrameRenderTarget::width() const {
    return widthValue;
}

int FrameRenderTarget::height() const {
    return heightValue;
}

int FrameRenderTarget::size() const {
    return widthValue * heightValue;
}

int FrameRenderTarget::bottom() const {
    return heightValue - 1;
}

int FrameRenderTarget::maxDimension() const {
    return max(widthValue, heightValue);
}

int FrameRenderTarget::hiddenBorderRows() const {
    return hiddenBorderRowsPerSide;
}

int FrameRenderTarget::hiddenBorderByteCount() const {
    return hiddenBorderRows() * width();
}

void FrameRenderTarget::setDimensions(int width_, int height_) {
    widthValue = width_;
    heightValue = height_;
}

int FrameRenderTarget::allocationByteCount() const {
    return size() + 2 * hiddenBorderByteCount();
}

unsigned char* FrameRenderTarget::visiblePixels(unsigned char* allocation) const {
    return allocation == 0 ? 0 : allocation + hiddenBorderByteCount();
}

void FrameRenderTarget::allocatePixels() {

    delete[] activeAllocation;
    delete[] passiveAllocation;

    activeAllocation = new unsigned char[allocationByteCount()];
    passiveAllocation = new unsigned char[allocationByteCount()];
    activeBuffer = visiblePixels(activeAllocation);
    passiveBuffer = visiblePixels(passiveAllocation);

    memset(activeAllocation, 0, allocationByteCount());
    memset(passiveAllocation, 0, allocationByteCount());
}

void FrameRenderTarget::swapBuffers() {
    unsigned char* allocation = activeAllocation;
    activeAllocation = passiveAllocation;
    passiveAllocation = allocation;

    unsigned char* t = activeBuffer;
    activeBuffer = passiveBuffer;
    passiveBuffer = t;
}

void FrameRenderTarget::clear() {
    if (activeAllocation != 0)
        memset(activeAllocation, 0, allocationByteCount());
    if (passiveAllocation != 0)
        memset(passiveAllocation, 0, allocationByteCount());
}

unsigned char* FrameRenderTarget::activePixels() {
    return activeBuffer;
}

unsigned char* FrameRenderTarget::passivePixels() {
    return passiveBuffer;
}

const unsigned char* FrameRenderTarget::activePixels() const {
    return activeBuffer;
}

const unsigned char* FrameRenderTarget::passivePixels() const {
    return passiveBuffer;
}

unsigned char* FrameRenderTarget::activeTopHiddenRows() {
    return activeBuffer == 0 ? 0 : activeBuffer - hiddenBorderByteCount();
}

unsigned char* FrameRenderTarget::activeBottomHiddenRows() {
    return activeBuffer == 0 ? 0 : activeBuffer + size();
}
