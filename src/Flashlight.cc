#include "cthugha.h"
#include "CoreOption.h"
#include "SoundAnalyze.h"
#include "display.h"
#include "CthughaBuffer.h"
#include "imath.h"

class FlashlightEntry : public OnEntry {
public:
    int operator()() {
        int i, j, l;
        static Palette Pal;

        // Brighten the palette currently being displayed. This preserves
        // transient palette changes such as PCX image palettes.
        memcpy(Pal, CthughaBuffer::current->currentPalette, sizeof(Palette));

        for (l = soundAnalyze.fire << 3, i = 0; (i < 256) && (l > 0); i++, l -= 8)
            for (j = 0; j < 3; j++)
                Pal[i][j] = min(Pal[i][j] + l, 255);

        CthughaBuffer::current->setPalette(Pal);

        return 0;
    }
};

CoreOptionEntry* flashlight_entries[] = { new OffEntry(), new FlashlightEntry() };
