#include "cthugha.h"
#include "CoreOption.h"
#include "display.h"
#include "pcx.h"
#include "CthughaBuffer.h"
#include "disp-sys.h"
#include "cth_buffer.h"
#include "interface.h"
#include "CthughaDisplay.h"
#include "SoundAnalyze.h"
#include "imath.h"
#include "glcthugha.h"
#include "DisplayDevice.h"

#include <GL/glut.h>


//
// pcx image as background
//
class ImageBackground : public CoreOptionEntry {
public:
    ImageBackground() : CoreOptionEntry("image", "show image", 1) {}

    int operator()() {

	if(displayPhase == afterScreen)
	    return 0;

	// image 0 is empty
	if(CthughaBuffer::buffers[0].pcx.currentN() == 0) 
	    return 0;

	// only with object type displays
	ScreenEntryGL * s = (ScreenEntryGL *)screen.current();
	if( s->type != ScreenEntryGL::object) {
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	    return 0;
	}

	glDepthMask(GL_FALSE);	// ignore the background for the depth buffer
	glDisable(GL_DEPTH_TEST);

	//
	// set up the coordinate system
	//
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, disp_size.x,
		0.0, disp_size.y, 
		-1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	setPcxTexture(0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDisable(GL_LIGHTING);
	
	glBegin( GL_POLYGON);
	glTexCoord2f(0.0, 1.0); glVertex2i(0, 0);
	glTexCoord2f(0.0, 0.0); glVertex2i(0, disp_size.y);
	glTexCoord2f(1.0, 0.0); glVertex2i(disp_size.x, disp_size.y);
	glTexCoord2f(1.0, 1.0); glVertex2i(disp_size.x, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, pcxTextures[0]);

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);	

	return 0;
    }
};


class ImageBlendBackground : public CoreOptionEntry {
public:
    ImageBlendBackground() : CoreOptionEntry("image-blend", "show image blended", 1) {}

    int operator()() {

	if(displayPhase == beforeScreen)
	    return 0;

	// image 0 is empty
	if(CthughaBuffer::buffers[0].pcx.currentN() == 0) 
	    return 0;

	// only with object type displays
	ScreenEntryGL * s = (ScreenEntryGL *)screen.current();
	if( s->type != ScreenEntryGL::object) {
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	    return 0;
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, disp_size.x,
		0.0, disp_size.y, 
		-1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glDisable(GL_DEPTH_TEST);

// NOTE: currently the image textures are stored without alpha value
//	glBlendFunc(GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA);
//	glBlendFunc(GL_SRC_ALPHA,  GL_ONE);
	glBlendFunc(GL_ONE,  GL_ONE);
	glEnable(GL_BLEND);
	
	setPcxTexture(0);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDisable(GL_LIGHTING);
	
	glBegin( GL_POLYGON);
	glTexCoord2f(0.0, 1.0); glVertex2i(0, 0);
	glTexCoord2f(0.0, 0.0); glVertex2i(0, disp_size.y);
	glTexCoord2f(1.0, 0.0); glVertex2i(disp_size.x, disp_size.y);
	glTexCoord2f(1.0, 1.0); glVertex2i(disp_size.x, 0);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, pcxTextures[0]);
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);

	return 0;
    }
};


class BlackBackground : public CoreOptionEntry {
public:
    BlackBackground() : CoreOptionEntry("off", "do not show image", 1) {}
};


static CoreOptionEntry * _entries[] = {
    new ImageBackground,
    new ImageBlendBackground,
    new BlackBackground,
};
static CoreOptionEntryList backgroundEntries(_entries, 
					     sizeof(_entries)/sizeof(CoreOptionEntry*));


CoreOption background(-1, "background", backgroundEntries);
