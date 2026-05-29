// -*- c++ -*-

#ifndef __CTHUGHA_BUFFER_H
#define __CTHUGHA_BUFFER_H

#include "cthugha.h"
#include "CoreOption.h"

#include "display.h"
#include "pcx.h"
#include "translate.h"

class CthughaBuffer {
public:
    int palChanged;

    CoreOption palette;
    OptionPCX pcx;
    TranslateOption translate;

    Palette currentPalette;

    CthughaBuffer();

    void setPalette(const Palette pal);
    void swapBuffers();
    unsigned char* activePixels();
    unsigned char* passivePixels();
    const unsigned char* activePixels() const;
    const unsigned char* passivePixels() const;

private:
    unsigned char* activeBuffer; /* buffer next on screen */
    unsigned char* passiveBuffer; /* buffer current on screen */

public:
    static CthughaBuffer buffer;
    static CthughaBuffer* current;
    static int nInit;

    void init();
    static void initAll();

    static const Palette& getPalette(int i) {
        return ((PaletteEntry*)(buffer.palette[i]))->pal;
    }
};

#endif
