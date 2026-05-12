#include "cthugha.h"
#include "CthughaDisplay.h"
#include "SoundProcess.h"
#include "imath.h"
#include "DisplayDevice.h"
#include "interface.h"
#include "glcthugha.h"

#include <GL/glut.h>

void newCthughaDisplay() {
    cthughaDisplay = new CthughaDisplayGL();
}


DisplayPhase displayPhase;



void CthughaDisplayGL::operator()() { 

    displayDevice->preDraw();

    displayPhase = beforeScreen;

    background();		// draw background afterwards!

    // 
    // set up the perspective for the drawn objects
    //
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, double(disp_size.x)/double(disp_size.y), 0.1, 40.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    light();			// set up  light
    
    screen();			// draw the buffer

    displayPhase = afterScreen;

    fly();

    background();		// draw background afterwards!
    
    glDisable(GL_LIGHTING);
}
