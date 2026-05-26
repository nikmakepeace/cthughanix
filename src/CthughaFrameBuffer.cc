#include "cthugha.h"
#include "CthughaFrameBuffer.h"

CthughaFrameBuffer::CthughaFrameBuffer()
    : activeData(0)
    , passiveData(0)
    , paletteData(0)
    , paletteChangedData(0)
    , widthValue(0)
    , heightValue(0)
    , pitchValue(0)
    , ownsData(0) { }

CthughaFrameBuffer::~CthughaFrameBuffer() {
    if (ownsData) {
        delete[] activeData;
        delete[] passiveData;
    }
    activeData = 0;
    passiveData = 0;
    paletteData = 0;
    paletteChangedData = 0;
}

void CthughaFrameBuffer::bind(unsigned char* active, unsigned char* passive,
    int width, int height, int pitch, Palette* palette, int* paletteChanged) {
    if (ownsData) {
        delete[] activeData;
        delete[] passiveData;
    }

    activeData = active;
    passiveData = passive;
    paletteData = palette;
    paletteChangedData = paletteChanged;
    widthValue = width;
    heightValue = height;
    pitchValue = (pitch > 0) ? pitch : width;
    ownsData = 0;

    CTH_TRACE("bound active=%p passive=%p width=%d height=%d pitch=%d\n", "cthugha frame buffer",
        activeData, passiveData, widthValue, heightValue, pitchValue);
}

void CthughaFrameBuffer::allocate(int width, int height, int pitch) {
    if (ownsData) {
        delete[] activeData;
        delete[] passiveData;
    }

    widthValue = width;
    heightValue = height;
    pitchValue = (pitch > 0) ? pitch : width;
    int bytes = pitchValue * heightValue;
    activeData = new unsigned char[bytes];
    passiveData = new unsigned char[bytes];
    ownsData = 1;
    clear();

    CTH_TRACE("allocated width=%d height=%d pitch=%d bytes=%d\n", "cthugha frame buffer",
        widthValue, heightValue, pitchValue, bytes);
}

void CthughaFrameBuffer::clear(unsigned char value) {
    int bytes = pitchValue * heightValue;
    if (activeData != 0)
        memset(activeData, value, bytes);
    if (passiveData != 0)
        memset(passiveData, value, bytes);
}

void CthughaFrameBuffer::swapBuffers() {
    unsigned char* tmp = activeData;
    activeData = passiveData;
    passiveData = tmp;
}

void CthughaFrameBuffer::setPalette(const Palette palette) {
    if (paletteData == 0)
        return;

    memcpy(*paletteData, palette, sizeof(Palette));
    if (paletteChangedData != 0)
        *paletteChangedData = 1;
}
