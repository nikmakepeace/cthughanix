#include "cthugha.h"
#include "display.h"
#include "SoundAnalyze.h"
#include "CthughaDisplay.h"
#include "glcthugha.h"
#include "imath.h"

#include <GL/glut.h>



//
// Light
//
class LightEntry : public CoreOptionEntry {
protected:
    static GLfloat red[3];
    static GLfloat green[3];
    static GLfloat blue[3];
    static GLfloat white[3];

    // add a small spotlight
    void spot(int i, GLfloat * pos, GLfloat * col, GLfloat * dir = NULL) {
	static GLfloat defDir[4] = {0, 0, -1, 1};
	if(dir == NULL)
	    dir = defDir;

	glLightfv(GL_LIGHT0 + i, GL_POSITION, pos);
	glLightfv(GL_LIGHT0 + i, GL_SPOT_DIRECTION, dir);
	glLightf (GL_LIGHT0 + i, GL_SPOT_CUTOFF, 50.0);
	glLighti (GL_LIGHT0 + i, GL_SPOT_EXPONENT, 128);

	glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, col);

	static GLfloat specular[] = {1,1,1,1};
	glLightfv(GL_LIGHT0 + i, GL_SPECULAR, specular);

	static GLfloat ambient[] = {1,1,1,1};
	glLightfv(GL_LIGHT0 + i, GL_AMBIENT, ambient);

	glEnable (GL_LIGHT0 + i);
    }

    // set the default material values
    void material() {
	static GLfloat mat_shininess[] = {40.0};
	static GLfloat mat_diffuse[] = {1,1,1,1};
	static GLfloat mat_ambient[] = {0.2,0.2,0.2,1};
	GLfloat mat_emmission[] = {0,0,0, 1.0};
	GLfloat mat_specular[] = {0.8, 0.8, 0.8, 1.0};

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, mat_ambient);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, mat_emmission);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, 100.0);

    }

public:
    LightEntry(const char * n, const char * d) : CoreOptionEntry(n,d) {}
};
GLfloat LightEntry::red[3]   = {1.0, 0.2, 0.2};
GLfloat LightEntry::green[3] = {0.2, 1.0, 0.2};
GLfloat LightEntry::blue[3]  = {0.2, 0.2, 1.0};
GLfloat LightEntry::white[3] = {1.0, 1.0, 1.0};

class NoLight : public LightEntry {
public:
    NoLight() : LightEntry("none", "no Lighting") {}
    int operator()() {
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDisable(GL_LIGHTING);
	return 0;
    }

};

class Spots1 : public LightEntry {
public:
    Spots1() : LightEntry("spots1", "RGB Spots rotating") {}
    int operator()() {

	GLfloat pos[] = {0,0,1.5,1};
	
	ScreenEntryGL * s = (ScreenEntryGL *)screen.current();
	if( s->type != ScreenEntryGL::object) {
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	    return 0;
	}
	
	glEnable(GL_LIGHTING);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

	
	pos[0] = 0.5 * sin(now);
	pos[1] = 0.5 * cos(now);
	spot(0, pos, red);
	
	pos[0]= 0.5 * sin(now + M_PI2/3.0);
	pos[1]= 0.5 * cos(now + M_PI2/3.0);
	spot(1, pos, green);
	
	pos[0] = 0.5 * sin(now + 2.0*M_PI2/3.0);
	pos[1] = 0.5 * cos(now + 2.0*M_PI2/3.0);
	spot(2, pos, blue);
	
	material();
	
	return 0;
    }
};

class Spots2 : public LightEntry {
public:
    Spots2() : LightEntry("spots2", "RGB Spots pendulum") {}
    int operator()() {
	ScreenEntryGL * s = (ScreenEntryGL *)screen.current();
	if( s->type != ScreenEntryGL::object) {
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	    return 0;
	}
	
	glEnable(GL_LIGHTING);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	
	float r = sin(now);
	
	GLfloat pos[] = {0,0,1.5,1};
	
	pos[0] = 0;
	pos[1] = r;
	spot(0, pos, red);
	
	pos[0]= r * sin(M_PI2/3.0);
	pos[1]= r * cos(M_PI2/3.0);
	spot(1, pos, green);
	
	pos[0] = r * sin(2.0*M_PI2/3.0);
	pos[1] = r * cos(2.0*M_PI2/3.0);
	spot(2, pos, blue);
	
	material();
	
	return 0;
    }

};

class Spots3 : public LightEntry {
public:
    Spots3() : LightEntry("with-spot", "A single white spots") {}
    int operator()() {
	ScreenEntryGL * s = (ScreenEntryGL *)screen.current();
	if( s->type != ScreenEntryGL::object) {
	    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	    return 0;
	}
	
	glEnable(GL_LIGHTING);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	
	GLfloat pos[] = {0, 0, 1.2, 1.0};
	GLfloat dir[] = {0, 0, -1};
	spot(0, pos, white, dir);

	glDisable(GL_LIGHT1);
	glDisable(GL_LIGHT2);

	material();
	
	return 0;
    }

};


static CoreOptionEntry * _lights[] = {
    new NoLight,
    new Spots1,
    new Spots2,
    new Spots3
};
static CoreOptionEntryList lightEntries(_lights, sizeof(_lights)/sizeof(CoreOptionEntry*));

CoreOption light(-1, "light", lightEntries);






