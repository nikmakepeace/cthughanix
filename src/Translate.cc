// Runtime coordinate remap executor used by TranslateFilter.

#include "Translate.h"
#include "FrameRenderTarget.h"

Translate::Translate()
    : tableValue() { }

Translate::Translate(const TranslationTable& table)
    : tableValue(table) { }

const char* Translate::name() const {
    return tableValue.name();
}

int Translate::ready() const {
    return tableValue.ready();
}

void Translate::execute(FrameRenderTarget& buffer, const FrameGeneratorContext& context) const {
    (void)context;

    if (!ready())
        return;

    const int width = buffer.width();
    const int height = buffer.height();
    const int size = buffer.size();

    if (width != tableValue.width() || height != tableValue.height()
        || size != tableValue.size())
        return;

    const int* trans = tableValue.data();

    buffer.swapBuffers();
    unsigned char* src = buffer.passivePixels();
    unsigned char* dst = buffer.activePixels();

    src[0] = 0;

    if (buffer.visibleRowsArePacked()) {
        for (int i = 0; i < size; i++)
            dst[i] = src[trans[i]];
        return;
    }

    for (int y = 0; y < height; y++) {
        unsigned char* row = buffer.activeRow(y);
        for (int x = 0; x < width; x++)
            row[x] = src[buffer.visibleLinearOffset(*trans++)];
    }
}
