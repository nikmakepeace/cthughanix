#include "cthugha.h"
#include "display.h"
#include "SoundAnalyze.h"
#include "CthughaDisplay.h"
#include "glcthugha.h"
#include "imath.h"

#include <GL/glut.h>



class Fly : public CoreOptionEntry {
protected:
    static GLuint texture;
    
    void doFly(GLfloat x, GLfloat y, GLfloat z, GLfloat r = 0.02) {
	glBegin( GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex3f(x-r, y-r, z);
	glTexCoord2f(1.0, 0.0); glVertex3f(x+r, y-r, z);
	glTexCoord2f(1.0, 1.0); glVertex3f(x+r, y+r, z);
	glTexCoord2f(0.0, 1.0); glVertex3f(x-r, y+r, z);
	glEnd();
    }
    
public:
    Fly(const char * name, const char * desc) :
	CoreOptionEntry(name, desc) {}
    
    virtual int operator()() {
	if(texture == 0) {
	    static char RGBA[32][32][4];

	    for(int i=0; i < 32; i++)
		for(int j=0; j < 32; j++) {
		    int r = min((i-16)*(i-16) + (j-16)*(j-16), 255);
		    RGBA[i][j][0] = 255;
		    RGBA[i][j][1] = (r < 5) ? 255-r : min(128, 64 + r/4);
		    RGBA[i][j][2] = (r < 5) ? 255-r : 0;
		    RGBA[i][j][3] = 255 - r;
		}
	    glGenTextures(1, &texture);
	    glEnable(GL_TEXTURE_2D);
	    
	    glBindTexture(GL_TEXTURE_2D, texture);

	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	    gluBuild2DMipmaps(GL_TEXTURE_2D,
			      GL_RGBA,
			      32, 32,
			      GL_RGBA,
			      GL_UNSIGNED_BYTE,
			      RGBA);
	    glBindTexture(GL_TEXTURE_2D, pcxTextures[0]);
	}
	return this->run();
    }
    virtual int run() {
	return 0;
    }
};
GLuint Fly::texture = 0;


class NoFly : public Fly {
public:
    NoFly() : Fly("none", "nothing") {}
};


class Rings : public Fly {
public:
    Rings() : Fly("rings", "fire fings") {}
    int run() {
	
	ScreenEntryGL * s = (ScreenEntryGL *)screen.current();
	if( s->type != ScreenEntryGL::plane) {
	    return 0;
	}
	
	
	glBindTexture(GL_TEXTURE_2D, texture);
	
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_FALSE);	// flies have no depth
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	const int N = 16;

	GLfloat offset[3] = {0.3 * sin(now), 
			     0, 
			     1.4 + 0.6 * sin(now)};
	for(int i=0; i < N; i++) {
	    double alpha = now + i/double(N)*M_PI2;
	    GLfloat pos[3] = { 0.1 * sin(alpha), 
			       0.6 * sin(now * 0.2), 
			       0.1 * cos(alpha) };
	    
	    doFly( pos[0] + offset[0], 
		   pos[1] + offset[1], 
		   pos[2] + offset[2]);
	}
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_TRUE);
	glDisable(GL_BLEND);
	
	glBindTexture(GL_TEXTURE_2D, pcxTextures[0]);

	return 0;
    }
};

static CoreOptionEntry * _flies[] = {
    new NoFly,
    new Rings
};
static CoreOptionEntryList flyEntries(_flies, sizeof(_flies)/sizeof(CoreOptionEntry*));
CoreOption fly(-1, "fly", flyEntries);





