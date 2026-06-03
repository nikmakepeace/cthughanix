#include "PixelTransfer.h"

static void writeNativePixel(unsigned char* destination, int bytesPerPixel,
    unsigned char index, const unsigned long* nativePixels) {
    unsigned long value = nativePixels != 0 ? nativePixels[index] : index;
    for (int i = 0; i < bytesPerPixel; ++i)
        destination[i] = (unsigned char)((value >> (8 * i)) & 0xff);
}

void PixelTransfer::indexedToNative(const unsigned char* source,
    PixelSize sourceSize, int sourcePitch, unsigned char* destination,
    PixelSize destinationSize, int destinationPitch, int bytesPerPixel,
    const unsigned long* nativePixels) {
    if (source == 0 || destination == 0)
        return;
    if (!sourceSize.valid() || !destinationSize.valid())
        return;
    if (sourcePitch < sourceSize.width)
        return;
    if (bytesPerPixel < 1 || bytesPerPixel > 4)
        return;
    if (destinationPitch < destinationSize.width * bytesPerPixel)
        return;

    for (int y = 0; y < destinationSize.height; ++y) {
        int sourceY = y * sourceSize.height / destinationSize.height;
        const unsigned char* sourceRow = source + sourceY * sourcePitch;
        unsigned char* destinationRow = destination + y * destinationPitch;

        for (int x = 0; x < destinationSize.width; ++x) {
            int sourceX = x * sourceSize.width / destinationSize.width;
            writeNativePixel(destinationRow + x * bytesPerPixel,
                bytesPerPixel, sourceRow[sourceX], nativePixels);
        }
    }
}
