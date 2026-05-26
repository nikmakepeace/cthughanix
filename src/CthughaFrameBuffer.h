// Internal indexed visual framebuffer scaffold.

#ifndef __CTHUGHA_FRAME_BUFFER_H
#define __CTHUGHA_FRAME_BUFFER_H

class CthughaFrameBuffer {
    unsigned char* activeData;
    unsigned char* passiveData;
    int widthValue;
    int heightValue;
    int pitchValue;
    int ownsData;

public:
    CthughaFrameBuffer();
    ~CthughaFrameBuffer();

    void bind(unsigned char* active, unsigned char* passive, int width, int height, int pitch);
    void allocate(int width, int height, int pitch = 0);
    void clear(unsigned char value = 0);
    void swapBuffers();

    unsigned char* active() { return activeData; }
    unsigned char* passive() { return passiveData; }
    const unsigned char* active() const { return activeData; }
    const unsigned char* passive() const { return passiveData; }

    int width() const { return widthValue; }
    int height() const { return heightValue; }
    int pitch() const { return pitchValue; }
    int isBound() const { return activeData != 0; }
};

#endif
