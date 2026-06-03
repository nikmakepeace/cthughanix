#ifndef __FRAME_COMPLETION_H
#define __FRAME_COMPLETION_H

class IndexedDisplayFrame;

class FrameCompletion {
public:
    static void mirrorHorizontally(IndexedDisplayFrame& frame, int filledWidth,
        int filledHeight);
    static void mirrorVertically(IndexedDisplayFrame& frame, int filledHeight);
    static void completeMirrored(IndexedDisplayFrame& frame, int filledWidth,
        int filledHeight);

    static void mirrorHorizontally(unsigned char* pixels, int filledWidth,
        int outputWidth, int height, int pitch);
    static void mirrorVertically(unsigned char* pixels, int rowBytes,
        int outputHeight, int filledHeight, int pitch);
};

#endif
