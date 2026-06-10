/** @file
 * Unit coverage for flame kernels on padded FrameStore layouts.
 */

#include "Flame.h"
#include "FrameStore.h"
#include "FrameGeneratorContext.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>

int cth_log_enabled(int) { return 0; }
int cth_log(int, const char*, ...) { return 0; }
int cth_log_context(int, const char*, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }
int cth_log_errno(int, const char*, ...) { return 0; }

void flame_clear(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_down(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_upslow(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_upsubtle(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_upfast(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_leftslow(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_leftsubtle(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_leftfast(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_rightslow(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_rightsubtle(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_rightfast(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_water(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_watersubtle(FrameRenderTarget& buffer,
    const FrameGeneratorContext& context, FlameRuntime& runtime);
void flame_skyline(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_weird(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_zzz(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_fade(FrameRenderTarget& buffer, const FrameGeneratorContext& context,
    FlameRuntime& runtime);
void flame_general_subtle(FrameRenderTarget& buffer,
    const FrameGeneratorContext& context, FlameRuntime& runtime);
void flame_general_slow(FrameRenderTarget& buffer,
    const FrameGeneratorContext& context, FlameRuntime& runtime);

static unsigned char patternValue(int linearOffset, int salt) {
    return (unsigned char)((linearOffset * 37 + salt * 19 + 251) & 0xff);
}

static void fillVisibleStream(FrameRenderTarget& target) {
    int hidden = target.hiddenBorderRows();
    int first = -hidden * target.width();
    int last = target.size() + hidden * target.width();

    for (int offset = first; offset < last; offset++) {
        int storage = target.visibleLinearOffset(offset);
        target.activePixels()[storage] = patternValue(offset, 3);
        target.passivePixels()[storage] = patternValue(offset, 9);
    }
}

static void poisonActivePadding(FrameRenderTarget& target) {
    for (int y = 0; y < target.height(); y++) {
        unsigned char* row = target.activeRow(y);
        for (int x = target.width(); x < target.pitch(); x++)
            row[x] = 0xee;
    }
}

static void poisonPassivePadding(FrameRenderTarget& target) {
    for (int y = 0; y < target.height(); y++) {
        unsigned char* row = target.passiveRow(y);
        for (int x = target.width(); x < target.pitch(); x++)
            row[x] = 0xee;
    }
}

static void assertVisibleMatches(const FrameRenderTarget& expected,
    const FrameRenderTarget& actual) {
    assert(expected.width() == actual.width());
    assert(expected.height() == actual.height());

    for (int y = 0; y < expected.height(); y++) {
        const unsigned char* expectedRow = expected.activeRow(y);
        const unsigned char* actualRow = actual.activeRow(y);
        for (int x = 0; x < expected.width(); x++)
            assert(expectedRow[x] == actualRow[x]);
    }
}

static void assertActivePaddingUntouched(const FrameRenderTarget& target) {
    for (int y = 0; y < target.height(); y++) {
        const unsigned char* row = target.activeRow(y);
        for (int x = target.width(); x < target.pitch(); x++)
            assert(row[x] == 0xee);
    }
}

typedef void (*FlameKernel)(FrameRenderTarget& buffer,
    const FrameGeneratorContext& context, FlameRuntime& runtime);

static void runKernelOnPackedAndPaddedStores(FlameKernel kernel) {
    FrameStore packedStore;
    FrameStore paddedStore;
    packedStore.resize(FrameStorageLayout(PixelSize(7, 5), 7, 3));
    paddedStore.resize(FrameStorageLayout(PixelSize(7, 5), 11, 3));

    FrameRenderTarget& packed = packedStore.renderTarget();
    FrameRenderTarget& padded = paddedStore.renderTarget();
    fillVisibleStream(packed);
    fillVisibleStream(padded);
    poisonActivePadding(padded);
    poisonPassivePadding(padded);

    FrameGeneratorContext context;
    FlameLookupTables tables;
    FlameRuntime runtime(0, tables);

    kernel(packed, context, runtime);
    kernel(padded, context, runtime);

    assertVisibleMatches(packed, padded);
    assertActivePaddingUntouched(padded);
}

static void testFlameClearSkipsPadding() {
    FrameStore store;
    store.resize(FrameStorageLayout(PixelSize(7, 5), 11, 3));
    FrameRenderTarget& target = store.renderTarget();
    fillVisibleStream(target);
    poisonActivePadding(target);

    FrameGeneratorContext context;
    FlameLookupTables tables;
    FlameRuntime runtime(0, tables);
    flame_clear(target, context, runtime);

    for (int y = 0; y < target.height(); y++) {
        const unsigned char* row = target.activeRow(y);
        for (int x = 0; x < target.width(); x++)
            assert(row[x] == 0);
    }
    assertActivePaddingUntouched(target);
}

static void testFlameKernelsUseVisibleStreamNotPadding() {
    static FlameKernel kernels[] = {
        flame_down,
        flame_upslow,
        flame_upsubtle,
        flame_upfast,
        flame_leftslow,
        flame_leftsubtle,
        flame_leftfast,
        flame_rightslow,
        flame_rightsubtle,
        flame_rightfast,
        flame_water,
        flame_watersubtle,
        flame_skyline,
        flame_weird,
        flame_zzz,
        flame_fade,
        flame_general_subtle,
        flame_general_slow,
    };

    for (unsigned int i = 0; i < sizeof(kernels) / sizeof(kernels[0]); i++)
        runKernelOnPackedAndPaddedStores(kernels[i]);
}

int main() {
    testFlameClearSkipsPadding();
    testFlameKernelsUseVisibleStreamNotPadding();
    return 0;
}
