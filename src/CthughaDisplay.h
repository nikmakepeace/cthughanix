// -*- c++ -*-
#ifndef __CTHUGHA_DISPLAY_H
#define __CTHUGHA_DISPLAY_H

#include "CoreOption.h"

// must be initialized AFTER displayDevice

extern xy draw_size;
extern OptionInt zoom;
extern OptionInt maxFramesPerSecond;

extern double now;				// position in time for this frame
extern double deltaT;				// duration of last frame

class CthughaDisplay {
protected:
    unsigned char * buffer0;
    
    double displayStart;
    int frames;

    void mirrorHorizontally();
    void mirrorVertically();
    int clearBorder();
    void zoom2Screen(unsigned char *, int);
    virtual void expandPalette(int) {}
    void checkFPS();
    void checkZoom();

public:
    unsigned char * buffer;
    int bufferWidth;

    unsigned char * expandedBuffer;
    int expandedBufferWidth;

    int needsClear;				// border must be cleard

    CthughaDisplay();

    void nextFrame();				// start the next frame
    virtual void operator()() {}

    void resetFPS();

    double fps;

    const char * status();

    friend int save_display();
};


//
// a special CthughaDisplay for X11
//
class CthughaDisplayX11 : public CthughaDisplay {
    unsigned char * expandedBuffer0;
    virtual void expandPalette(int);
    virtual void expandPaletteMirrorHV();
public:
    CthughaDisplayX11();
    virtual void operator()();
};

//
// SVGA is a little bit easier (here we always have 8bit)
//
class CthughaDisplaySVGA : public CthughaDisplay {
    void expandPalette(int);
public:
    CthughaDisplaySVGA();
    virtual void operator()();
};

//
// OpenGL is different
//
class CthughaDisplayGL : public CthughaDisplay {
public:
    virtual void operator()();
};


class ScreenEntry : public  CoreOptionEntry {
public:
    int (*screen)();
    xy size;

    ScreenEntry(int (*f)(), const char * name, const char * desc, xy s, int inUse=1) :
	CoreOptionEntry(name, desc, inUse), screen(f), size(s) {
    }
    
    int operator()() {
	return (*screen)();
    }
};


extern CthughaDisplay * cthughaDisplay;

void newCthughaDisplay();

#endif
