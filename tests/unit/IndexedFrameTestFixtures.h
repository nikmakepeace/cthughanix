#ifndef CTH_TEST_INDEXED_FRAME_TEST_FIXTURES_H
#define CTH_TEST_INDEXED_FRAME_TEST_FIXTURES_H

#include "IndexedDisplayFrame.h"
#include "IndexedFrame.h"

#include <assert.h>
#include <string.h>
#include <vector>

static const unsigned char CTH_TEST_PADDING_BYTE = 0xee;

inline unsigned char indexedFixtureValue(int x, int y) {
    return static_cast<unsigned char>(((y + 1) * 31 + (x + 1) * 7) & 0xff);
}

class IndexedFrameFixture {
    std::vector<unsigned char> storageValue;
    IndexedFrame frameValue;
    int widthValue;
    int heightValue;
    int pitchValue;

public:
    IndexedFrameFixture(int width, int height, int pitch)
        : storageValue(pitch * height, CTH_TEST_PADDING_BYTE)
        , frameValue(storageValue.data(), width, height, pitch, 0)
        , widthValue(width)
        , heightValue(height)
        , pitchValue(pitch) {
        assert(width > 0);
        assert(height > 0);
        assert(pitch >= width);

        for (int y = 0; y < heightValue; ++y) {
            unsigned char* row = storageValue.data() + y * pitchValue;
            for (int x = 0; x < widthValue; ++x)
                row[x] = indexedFixtureValue(x, y);
        }
    }

    const IndexedFrame& frame() const { return frameValue; }

    const unsigned char* line(int y) const {
        return storageValue.data() + y * pitchValue;
    }

    int width() const { return widthValue; }
    int height() const { return heightValue; }
    int pitch() const { return pitchValue; }
};

inline void preparePaddedDestination(IndexedDisplayFrame& frame, int width, int height,
    int pitch, unsigned char fill) {
    frame.resize(width, height, pitch);
    memset(frame.pixels(), fill, frame.byteCount());
}

inline int indexedRowsEqual(const unsigned char* left, const unsigned char* right,
    int width) {
    return memcmp(left, right, width) == 0;
}

inline int firstIndexedRowMismatch(const unsigned char* left, const unsigned char* right,
    int width) {
    for (int x = 0; x < width; ++x) {
        if (left[x] != right[x])
            return x;
    }
    return -1;
}

#endif
