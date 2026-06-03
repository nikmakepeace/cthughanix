#include "PixelTransfer.h"

#include <assert.h>
#include <string.h>

static void fillNativePixels(unsigned long* nativePixels) {
    for (int i = 0; i < 256; ++i)
        nativePixels[i] = (unsigned long)i;
}

static void testIndexed8CopyRespectsSourceAndDestinationPitch() {
    unsigned char source[] = {
        1, 2, 3, 99, 99,
        4, 5, 6, 88, 88
    };
    unsigned char destination[] = {
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
        0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa
    };

    PixelTransfer::indexedToNative(source, PixelSize(3, 2), 5,
        destination, PixelSize(3, 2), 6, 1, 0);

    assert(destination[0] == 1);
    assert(destination[1] == 2);
    assert(destination[2] == 3);
    assert(destination[3] == 0xaa);
    assert(destination[4] == 0xaa);
    assert(destination[5] == 0xaa);
    assert(destination[6] == 4);
    assert(destination[7] == 5);
    assert(destination[8] == 6);
    assert(destination[9] == 0xaa);
    assert(destination[10] == 0xaa);
    assert(destination[11] == 0xaa);
}

static void testNative16UsesLookupLowByteFirst() {
    unsigned long nativePixels[256];
    fillNativePixels(nativePixels);
    nativePixels[1] = 0x1201;
    nativePixels[2] = 0x3402;
    nativePixels[3] = 0x5603;
    nativePixels[4] = 0x7804;
    unsigned char source[] = {
        1, 2, 99,
        3, 4, 88
    };
    unsigned char destination[12];
    memset(destination, 0xaa, sizeof(destination));

    PixelTransfer::indexedToNative(source, PixelSize(2, 2), 3,
        destination, PixelSize(2, 2), 6, 2, nativePixels);

    assert(destination[0] == 0x01);
    assert(destination[1] == 0x12);
    assert(destination[2] == 0x02);
    assert(destination[3] == 0x34);
    assert(destination[4] == 0xaa);
    assert(destination[5] == 0xaa);
    assert(destination[6] == 0x03);
    assert(destination[7] == 0x56);
    assert(destination[8] == 0x04);
    assert(destination[9] == 0x78);
    assert(destination[10] == 0xaa);
    assert(destination[11] == 0xaa);
}

static void testNative24SupportsPreparedByteOrderValues() {
    unsigned long nativePixels[256];
    fillNativePixels(nativePixels);
    nativePixels[1] = 0x112233;
    nativePixels[2] = 0x332211;
    unsigned char source[] = { 1, 2 };
    unsigned char destination[6];
    memset(destination, 0xaa, sizeof(destination));

    PixelTransfer::indexedToNative(source, PixelSize(2, 1), 2,
        destination, PixelSize(2, 1), 6, 3, nativePixels);

    assert(destination[0] == 0x33);
    assert(destination[1] == 0x22);
    assert(destination[2] == 0x11);
    assert(destination[3] == 0x11);
    assert(destination[4] == 0x22);
    assert(destination[5] == 0x33);
}

static void testNative32WritesFourBytesFromLookup() {
    unsigned long nativePixels[256];
    fillNativePixels(nativePixels);
    nativePixels[7] = 0x01020304;
    unsigned char source[] = { 7 };
    unsigned char destination[4];
    memset(destination, 0xaa, sizeof(destination));

    PixelTransfer::indexedToNative(source, PixelSize(1, 1), 1,
        destination, PixelSize(1, 1), 4, 4, nativePixels);

    assert(destination[0] == 0x04);
    assert(destination[1] == 0x03);
    assert(destination[2] == 0x02);
    assert(destination[3] == 0x01);
}

static void testNearestNeighbor2xScaling() {
    unsigned char source[] = {
        10, 20,
        30, 40
    };
    unsigned char destination[4 * 4];
    memset(destination, 0xaa, sizeof(destination));

    PixelTransfer::indexedToNative(source, PixelSize(2, 2), 2,
        destination, PixelSize(4, 4), 4, 1, 0);

    assert(destination[0] == 10);
    assert(destination[1] == 10);
    assert(destination[2] == 20);
    assert(destination[3] == 20);
    assert(destination[4] == 10);
    assert(destination[5] == 10);
    assert(destination[6] == 20);
    assert(destination[7] == 20);
    assert(destination[8] == 30);
    assert(destination[9] == 30);
    assert(destination[10] == 40);
    assert(destination[11] == 40);
    assert(destination[12] == 30);
    assert(destination[13] == 30);
    assert(destination[14] == 40);
    assert(destination[15] == 40);
}

int main() {
    testIndexed8CopyRespectsSourceAndDestinationPitch();
    testNative16UsesLookupLowByteFirst();
    testNative24SupportsPreparedByteOrderValues();
    testNative32WritesFourBytesFromLookup();
    testNearestNeighbor2xScaling();
    return 0;
}
