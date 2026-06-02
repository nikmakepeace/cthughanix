#ifndef __BITMAP_FONT_H
#define __BITMAP_FONT_H

#include <stdint.h>

class BitmapFont {
public:
    const char* name;
    int glyphWidth;
    int glyphHeight;
    const uint16_t* glyphRows;

    uint16_t row(unsigned char character, int y) const {
        if (glyphRows == 0 || y < 0 || y >= glyphHeight)
            return 0;

        return glyphRows[int(character) * glyphHeight + y];
    }
};

const BitmapFont& dosVga9x14Font();

#endif
