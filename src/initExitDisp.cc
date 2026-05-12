#include "cthugha.h"
#include "information.h"			/* title, credits, ... */
#include "display.h"
#include "sound.h"
#include "translate.h"
#include "options.h"
#include "keys.h"
#include "imath.h"
#include "waves.h"
#include "Option.h"
#include "SoundAnalyze.h"
#include "AutoChanger.h"
#include "CthughaBuffer.h"
#include "SoundServer.h"
#include "CthughaDisplay.h"
#include "CDPlayer.h"
#include "DisplayDevice.h"
#include "interface.h"
#include "keymap.h"

#include <unistd.h>
#include <signal.h>


/*
 * handle ^Z and continue 
 */
void sig_tty_cont(int);
void sig_tty_stop(int) {
    printfv(0, "Stopping...\n");

    signal(SIGCONT, sig_tty_cont);		/* set, how to continue */

    cthugha_pause = 1;				/* in interface we will really stop */
}
void sig_tty_cont(int) {
    printfv(0, "Continuing...\n");

    init_sound();				/* and sound */

    signal(SIGTSTP, sig_tty_stop);		/* set, how to stop again */

    raise(SIGCONT);				/* default action */
}


//
// deleter of global objects
//	
void deleter() {
    delete autoChanger;				// this also saves options 
    delete cthughaDisplay;
    delete soundServer;
    delete soundDevice;
    delete cdPlayer;
}

/*
 *
 */
int main(int argc, char * argv[]) {

    srand(time(0));			/* initialize random generator */
    seteuid(getuid());			// give up root privileges

    if( get_pre_params(argc, argv) )	// handle some special arguments (verbose, ...)
	return 1;

    if( cth_init(&argc, argv) )		/* special initialization */
	return 1;

    if( get_params(argc, argv) )	/* parse cmd-line and read ini-files*/
	return 1;

    title();				/* Display titlemessage */

    init_imath();

    atexit(deleter);

    if( ncurses_use) {
	init_ncurses(); 
	atexit(exit_ncurses);
    }

    printfv(1, "Initializing the sound device...\n");
    SoundDevice::newSD();

    printfv(1, "Initializing the sound server...\n");
    soundServer = new SoundServer;

    printfv(1, "Initializing CD player...\n");
    cdPlayer = new CDPlayer;

    printfv(1, "Initializing Mixer device...\n");
    if(init_mixer())
	exit(0);

    printfv(1, "Initializing cthugha Buffer...\n");
    CthughaBuffer::initAll();

    printfv(1, "Initializing display...\n");
    newDisplayDevice();
    newCthughaDisplay();

    printfv(1, "Setting initial core options...\n");
    CoreOption::changeToInitial();

    printfv(1, "Initializing interface...\n");
    Interface::set("main");

    printfv(1, "Initializing keymaps...\n");
    Keymap::init();

    printfv(1, "Initializing the automatic changing...\n");
    autoChanger = new AutoChanger;

    signal(SIGTSTP, sig_tty_stop);	/* react to ^Z */

    displayDevice->mainLoop();

    printfv(1, "Exiting cthugha...\n");
    
    return 0;
}


//#define PROF
#undef PROF

#ifdef PROF
#define P(a) a
#else
#define P(a)
#endif

void run(int doDisplay) {
#ifdef PROF
    double T[10] = {0,0,0,0,0,0,0,0,0,0};
#endif

    P(T[0] = getTime();)
    cthughaDisplay->nextFrame();

    P(T[1] = getTime();)
    (*soundDevice)();

    P(T[2] = getTime();)
    soundAnalyze();

    P(T[3] = getTime();)
    (*autoChanger)();

    P(T[4] = getTime();)
    (*soundServer)();

    P(T[5] = getTime();)
    CthughaBuffer::run();

    P(T[6] = getTime();)
    if(doDisplay)
	(*cthughaDisplay)();

    P(T[7] = getTime();)
    (*cdPlayer)();

    P(T[8] = getTime();)
    // this is here to be sure not to interrupt a graphics operation. 
    if(cthugha_pause) {
	cthugha_pause = 0;

	exit_sound();				/* and sound */
	
	raise(SIGTSTP);				/* default action */
    }

    P(T[9] = getTime();)

#ifdef PROF
    static double Ts[10] = {0,0,0,0,0,0,0,0,0,0};
    for(int i=0; i < 8; i++)
	Ts[i] = Ts[i] + T[i+1] - T[i];

    static int n = 24;
    n ++;
    if(n == 25) {
	static double To = 0;
	double to = getTime();

	printf("%6.4f: ", to - To);
	To = to;

	for(int i=0; i < 9; i++) {
	    printf("%6.4f ", Ts[i]);
	    Ts[i] = 0;
	}
	printf("\n");
	n = 0;	
    }
#endif

}







