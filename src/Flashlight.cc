#include "cthugha.h"
#include "Flashlight.h"
#include "CthughaFrameBuffer.h"
#include "VisualPipeline.h"
#include "display.h"
#include "imath.h"

static CoreOptionEntry* flashlight_entries[] = { new OffEntry(), new OnEntry() };
static CoreOptionEntryList flashlightEntries;

CoreOption flashlight(0, "flashlight", flashlightEntries);

void init_flashlight() {
    flashlight.add(flashlight_entries, 2);
}

void apply_flashlight(CthughaFrameBuffer& frameBuffer, const VisualFrameContext& context) {
    if (!int(flashlight) || context.acousticContext == 0 || frameBuffer.palette() == 0)
        return;

    int i, j, l;
    static Palette pal;

    // Brighten the palette currently represented by this frame buffer. This
    // preserves transient palette changes such as PCX image palettes.
    memcpy(pal, *frameBuffer.palette(), sizeof(Palette));

    for (l = context.acousticContext->fire() << 3, i = 0; (i < 256) && (l > 0); i++, l -= 8)
        for (j = 0; j < 3; j++)
            pal[i][j] = min(pal[i][j] + l, 255);

    frameBuffer.setPalette(pal);
}
