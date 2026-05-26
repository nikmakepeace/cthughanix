// Internal indexed visual framebuffer scaffold.

#ifndef __CTHUGHA_FRAME_BUFFER_H
#define __CTHUGHA_FRAME_BUFFER_H

#include "display.h"

class CthughaFrameBuffer {
    unsigned char* activeData;
    unsigned char* passiveData;
    Palette* paletteData;
    int* paletteChangedData;
    CoreOption* paletteOptionData;
    int* lastPaletteData;
    int widthValue;
    int heightValue;
    int pitchValue;
    int ownsData;

public:
    CthughaFrameBuffer();
    ~CthughaFrameBuffer();

    void bind(unsigned char* active, unsigned char* passive, int width, int height, int pitch,
        Palette* palette = 0, int* paletteChanged = 0,
        CoreOption* paletteOption = 0, int* lastPalette = 0);
    void allocate(int width, int height, int pitch = 0);
    void clear(unsigned char value = 0);
    void swapBuffers();
    void setPalette(const Palette palette);

    unsigned char* active() { return activeData; }
    unsigned char* passive() { return passiveData; }
    const unsigned char* active() const { return activeData; }
    const unsigned char* passive() const { return passiveData; }
    Palette* palette() { return paletteData; }
    const Palette* palette() const { return paletteData; }
    CoreOption* paletteOption() { return paletteOptionData; }
    const CoreOption* paletteOption() const { return paletteOptionData; }
    int* lastPalette() { return lastPaletteData; }
    const int* lastPalette() const { return lastPaletteData; }
    int paletteChanged() const { return (paletteChangedData != 0) ? *paletteChangedData : 0; }
    void setPaletteChanged(int changed) {
        if (paletteChangedData != 0)
            *paletteChangedData = changed;
    }

    int width() const { return widthValue; }
    int height() const { return heightValue; }
    int pitch() const { return pitchValue; }
    int isBound() const { return activeData != 0; }
};

#endif
