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
#include "SoundProcess.h"
#include "AutoChanger.h"
#include "CthughaBuffer.h"
#include "SoundServer.h"
#include "CthughaDisplay.h"
#include "CDPlayer.h"


#include <errno.h>

#ifdef HAVE_VPRINTF
# include <stdarg.h>
#endif


//
// usefull to find memory errors
//
#if 0
void * operator new(size_t s) {
    void * r = malloc(s);
    printf("new %d: %x\n", s, r);
    return r;
}
void operator delete(void *d) {
    printf("delete  %x\n", d);
    if(d != NULL)
	free(d);
}
#endif



OptionInt cthugha_verbose("verbose", 1, 0, -1);		// verbosity level (no max, min=-1)


//
// print a verbose message
//
int printfv(int lvl, const char * fmt, ...) {
    if(lvl <= int(cthugha_verbose)) {
	// I had problems with missing carrige returns on Linux console
	// so i translate it to \n\r 
	char fmt_r[2*strlen(fmt)+1];
	int i;
	for(i=0; *fmt != '\0'; fmt++) {
	    fmt_r[i++] = *fmt;
	    if(*fmt == '\n')
		fmt_r[i++] = '\r';
	}
	fmt_r[i] = '\0';

//	printf("%5d:", getpid());

#ifdef HAVE_VPRINTF
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt_r, ap);
	va_end(ap);
#else
	printf(fmt_r);
#endif
	fflush(stdout);
    }
    return 0;
}

//
// print an error message and the error number
//
int printfee(const char * fmt, ...) {

   printf("\n\r"); 
   fflush(stdout);

   // I had problems with missing carrige returns on Linux console
   // so i translate it to \n\r 
   char fmt_r[2*strlen(fmt)+1];
   int i;
   for(i=0; *fmt != '\0'; fmt++) {
       fmt_r[i++] = *fmt;
       if(*fmt == '\n')
	   fmt_r[i++] = '\r';
   }
   fmt_r[i] = '\0';
   
#ifdef HAVE_VPRINTF
   va_list ap;
   va_start(ap, fmt);
   vfprintf(stderr, fmt_r, ap);
   va_end(ap);
#else
   fprintf(stderr, fmt_r);
#endif

#ifdef HAVE_STRERROR
   fprintf(stderr, " (%d - %s)\n\r", errno, strerror(errno));
#else
   fprintf(stderr, " (%d)\n\r", errno);
#endif

   fflush(stderr);

   return 0;
}

//
// print an error message
//
int printfe(const char * fmt, ...) {

   printf("\n\r"); 
   fflush(stdout);

   // I had problems with missing carrige returns on Linux console
   // so i translate it to \n\r 
   char fmt_r[2*strlen(fmt)+1];
   int i;
   for(i=0; *fmt != '\0'; fmt++) {
       fmt_r[i++] = *fmt;
       if(*fmt == '\n')
	   fmt_r[i++] = '\r';
   }
   fmt_r[i] = '\0';
   
#ifdef HAVE_VPRINTF
   va_list ap;
   va_start(ap, fmt);
   vfprintf(stderr, fmt_r, ap);
   va_end(ap);
#else
   fprintf(stderr, fmt);
#endif

   fflush(stderr);

   return 0;
}



//
// combine sprintf and system
//
int systemf(const char * fmt, ...) {

#ifdef HAVE_VPRINTF
    char cmd[6 * PATH_MAX];
    
    va_list ap;
    va_start(ap, fmt);
    vsprintf(cmd, fmt, ap);
    va_end(ap);
    
    return system(cmd);
#else
#error 'vprintf' is not available. Some parts of Cthugha will not work!
    return -1;
#endif
}



extern int cthugha_close = 0;				// closing right now
int cthugha_pause = 0;				// going to pause (^Z)





/*
 * get the 1/100 sec since program start
 */
int gettime() {
    struct timeval tv;
    static int starttime = 0;
    
    gettimeofday(&tv, NULL);

    if(starttime == 0) {
	starttime = tv.tv_sec;
    }
    tv.tv_sec -= starttime;

    return tv.tv_sec * 100L + tv.tv_usec / 10000L;
}
double getTime() {
    struct timeval tv;
    
    gettimeofday(&tv, NULL);

    return double(tv.tv_sec) + 1e-6 * double(tv.tv_usec);
}










