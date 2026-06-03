#include "FrameCompletion.h"

#include "IndexedDisplayFrame.h"

#include <string.h>

void FrameCompletion::mirrorHorizontally(IndexedDisplayFrame& frame,
    int filledWidth, int filledHeight) {
    mirrorHorizontally(frame.pixels(), filledWidth, frame.width(), filledHeight,
        frame.pitch());
}

void FrameCompletion::mirrorVertically(IndexedDisplayFrame& frame,
    int filledHeight) {
    mirrorVertically(frame.pixels(), frame.width(), frame.height(), filledHeight,
        frame.pitch());
}

void FrameCompletion::completeMirrored(IndexedDisplayFrame& frame, int filledWidth,
    int filledHeight) {
    mirrorHorizontally(frame, filledWidth, filledHeight);
    mirrorVertically(frame, filledHeight);
}

void FrameCompletion::mirrorHorizontally(unsigned char* pixels, int filledWidth,
    int outputWidth, int height, int pitch) {
    if (pixels == 0 || filledWidth <= 0 || outputWidth <= filledWidth
        || height <= 0 || pitch < outputWidth)
        return;

    unsigned char* row = pixels;
    for (int y = 0; y < height; ++y) {
        unsigned char* src = row;
        unsigned char* dst = row + outputWidth - 1;
        for (int x = 0; x < filledWidth; ++x)
            *dst-- = *src++;
        row += pitch;
    }
}

void FrameCompletion::mirrorVertically(unsigned char* pixels, int rowBytes,
    int outputHeight, int filledHeight, int pitch) {
    if (pixels == 0 || rowBytes <= 0 || outputHeight <= filledHeight
        || filledHeight <= 0 || pitch < rowBytes)
        return;

    unsigned char* src = pixels;
    unsigned char* dst = pixels + (outputHeight - 1) * pitch;
    for (int y = 0; y < filledHeight; ++y) {
        memcpy(dst, src, rowBytes);
        src += pitch;
        dst -= pitch;
    }
}
