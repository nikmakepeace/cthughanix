#ifndef __GLCTHUGHA_H
#define __GLCTHUGHA_H

#include "Option.h"
#include "CoreOption.h"

#include "CthughaBuffer.h"

extern int fullScreen;

class ScreenEntryGL : public CoreOptionEntry {
public:
    int (*screen)();
    int nBuffers;
    enum Type {
	object,
	plane
    } type;

    ScreenEntryGL(int (*f)(), 
		  const char * name, 
		  const char * desc, 
		  Type t,
		  int nB=1) :
	CoreOptionEntry(name, desc), screen(f), nBuffers(nB), type(t) {
    }

    int operator()() {
	CthughaBuffer::nBuffers = nBuffers;
	return (*screen)();
    }
};



#include <GL/glut.h>

extern GLuint * pcxTextures;
void setPcxTexture(int P);

//
// is the screen funciton alread done, or not
//
// this is used by the background methods. some need to be
// drawn before the objects ("solid background"), some
// after the background ("blended images")
// added an extra argument to the background method would be more work.
//
enum DisplayPhase {
    beforeScreen = 0,
    afterScreen = 1
};

extern DisplayPhase displayPhase;

#endif
