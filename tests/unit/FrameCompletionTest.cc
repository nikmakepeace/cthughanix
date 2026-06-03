#include "FrameCompletion.h"
#include "IndexedFrameTestFixtures.h"

#include <assert.h>
#include <string.h>

static const unsigned char kFill = 0xa5;

static void assertVisibleRowEquals(const IndexedDisplayFrame& frame, int y,
    const unsigned char* expected) {
    assert(memcmp(frame.line(y), expected, frame.width()) == 0);
}

static void assertPaddingUnchanged(const IndexedDisplayFrame& frame) {
    for (int y = 0; y < frame.height(); ++y) {
        const unsigned char* row = frame.line(y);
        for (int x = frame.width(); x < frame.pitch(); ++x)
            assert(row[x] == kFill);
    }
}

static void testHorizontalMirrorCompletesFilledRowsAndKeepsPitchPadding() {
    IndexedDisplayFrame frame;
    preparePaddedDestination(frame, 4, 2, 6, kFill);
    frame.line(0)[0] = 1;
    frame.line(0)[1] = 2;
    frame.line(1)[0] = 3;
    frame.line(1)[1] = 4;

    FrameCompletion::mirrorHorizontally(frame, 2, 2);

    const unsigned char row0[] = { 1, 2, 2, 1 };
    const unsigned char row1[] = { 3, 4, 4, 3 };
    assertVisibleRowEquals(frame, 0, row0);
    assertVisibleRowEquals(frame, 1, row1);
    assertPaddingUnchanged(frame);
}

static void testVerticalMirrorCompletesFullRowsAndKeepsPitchPadding() {
    IndexedDisplayFrame frame;
    preparePaddedDestination(frame, 4, 4, 6, kFill);
    const unsigned char row0[] = { 1, 2, 3, 4 };
    const unsigned char row1[] = { 5, 6, 7, 8 };
    memcpy(frame.line(0), row0, sizeof(row0));
    memcpy(frame.line(1), row1, sizeof(row1));

    FrameCompletion::mirrorVertically(frame, 2);

    assertVisibleRowEquals(frame, 0, row0);
    assertVisibleRowEquals(frame, 1, row1);
    assertVisibleRowEquals(frame, 2, row1);
    assertVisibleRowEquals(frame, 3, row0);
    assertPaddingUnchanged(frame);
}

static void testCombinedMirrorCompletesQuadrantInPlace() {
    IndexedDisplayFrame frame;
    preparePaddedDestination(frame, 4, 4, 6, kFill);
    frame.line(0)[0] = 1;
    frame.line(0)[1] = 2;
    frame.line(1)[0] = 3;
    frame.line(1)[1] = 4;

    FrameCompletion::completeMirrored(frame, 2, 2);

    const unsigned char row0[] = { 1, 2, 2, 1 };
    const unsigned char row1[] = { 3, 4, 4, 3 };
    assertVisibleRowEquals(frame, 0, row0);
    assertVisibleRowEquals(frame, 1, row1);
    assertVisibleRowEquals(frame, 2, row1);
    assertVisibleRowEquals(frame, 3, row0);
    assertPaddingUnchanged(frame);
}

static void testNoCompletionLeavesVisibleAndPaddingBytesUnchanged() {
    IndexedDisplayFrame frame;
    preparePaddedDestination(frame, 4, 2, 6, kFill);
    const unsigned char row0[] = { 1, 2, 3, 4 };
    const unsigned char row1[] = { 5, 6, 7, 8 };
    memcpy(frame.line(0), row0, sizeof(row0));
    memcpy(frame.line(1), row1, sizeof(row1));

    FrameCompletion::completeMirrored(frame, 4, 2);

    assertVisibleRowEquals(frame, 0, row0);
    assertVisibleRowEquals(frame, 1, row1);
    assertPaddingUnchanged(frame);
}

static void testRawVerticalMirrorSupportsExpandedRows() {
    unsigned char rows[4][10];
    memset(rows, kFill, sizeof(rows));
    const unsigned char row0[] = { 1, 2, 3, 4, 5, 6 };
    const unsigned char row1[] = { 7, 8, 9, 10, 11, 12 };
    memcpy(rows[0], row0, sizeof(row0));
    memcpy(rows[1], row1, sizeof(row1));

    FrameCompletion::mirrorVertically(&rows[0][0], 6, 4, 2, 10);

    assert(memcmp(rows[0], row0, sizeof(row0)) == 0);
    assert(memcmp(rows[1], row1, sizeof(row1)) == 0);
    assert(memcmp(rows[2], row1, sizeof(row1)) == 0);
    assert(memcmp(rows[3], row0, sizeof(row0)) == 0);
    for (int y = 0; y < 4; ++y)
        for (int x = 6; x < 10; ++x)
            assert(rows[y][x] == kFill);
}

int main() {
    testHorizontalMirrorCompletesFilledRowsAndKeepsPitchPadding();
    testVerticalMirrorCompletesFullRowsAndKeepsPitchPadding();
    testCombinedMirrorCompletesQuadrantInPlace();
    testNoCompletionLeavesVisibleAndPaddingBytesUnchanged();
    testRawVerticalMirrorSupportsExpandedRows();
    return 0;
}
