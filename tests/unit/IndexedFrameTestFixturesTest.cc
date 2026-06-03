#include "IndexedFrameTestFixtures.h"

#include <assert.h>

static void testSourceFixtureHasPaddedPitch() {
    IndexedFrameFixture fixture(5, 3, 8);
    const IndexedFrame& frame = fixture.frame();

    assert(frame.valid());
    assert(frame.width == 5);
    assert(frame.height == 3);
    assert(frame.pitch == 8);
    assert(frame.pixels == fixture.line(0));
    assert(fixture.line(2) == fixture.line(0) + 16);
}

static void testSourceFixtureUsesDistinctVisibleValues() {
    IndexedFrameFixture fixture(5, 3, 8);

    for (int y = 0; y < fixture.height(); ++y) {
        const unsigned char* row = fixture.line(y);
        for (int x = 0; x < fixture.width(); ++x)
            assert(row[x] == indexedFixtureValue(x, y));
    }

    assert(fixture.line(0)[0] != fixture.line(0)[1]);
    assert(fixture.line(0)[0] != fixture.line(1)[0]);
}

static void testSourceFixturePaddingIsSentinel() {
    IndexedFrameFixture fixture(5, 3, 8);

    for (int y = 0; y < fixture.height(); ++y) {
        const unsigned char* row = fixture.line(y);
        for (int x = fixture.width(); x < fixture.pitch(); ++x)
            assert(row[x] == CTH_TEST_PADDING_BYTE);
    }
}

static void testPaddedDestinationHelper() {
    IndexedDisplayFrame frame;
    preparePaddedDestination(frame, 4, 2, 7, 0x5a);

    assert(frame.valid());
    assert(frame.width() == 4);
    assert(frame.height() == 2);
    assert(frame.pitch() == 7);
    assert(frame.line(1) == frame.line(0) + 7);

    for (int y = 0; y < frame.height(); ++y) {
        const unsigned char* row = frame.line(y);
        for (int x = 0; x < frame.pitch(); ++x)
            assert(row[x] == 0x5a);
    }
}

static void testRowCompareHelpers() {
    const unsigned char left[] = { 1, 2, 3, 4 };
    const unsigned char same[] = { 1, 2, 3, 4 };
    const unsigned char different[] = { 1, 9, 3, 4 };

    assert(indexedRowsEqual(left, same, 4));
    assert(!indexedRowsEqual(left, different, 4));
    assert(firstIndexedRowMismatch(left, same, 4) == -1);
    assert(firstIndexedRowMismatch(left, different, 4) == 1);
}

int main() {
    testSourceFixtureHasPaddedPitch();
    testSourceFixtureUsesDistinctVisibleValues();
    testSourceFixturePaddingIsSentinel();
    testPaddedDestinationHelper();
    testRowCompareHelpers();
    return 0;
}
