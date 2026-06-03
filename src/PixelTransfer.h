#ifndef __PIXEL_TRANSFER_H
#define __PIXEL_TRANSFER_H

#include "DisplayGeometry.h"

class PixelTransfer {
public:
    static void indexedToNative(const unsigned char* source,
        PixelSize sourceSize, int sourcePitch,
        unsigned char* destination, PixelSize destinationSize,
        int destinationPitch, int bytesPerPixel,
        const unsigned long* nativePixels);
};

#endif
