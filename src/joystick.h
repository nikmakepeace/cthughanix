#ifndef __JOYSTICK_H
#define __JOYSTICK_H

#include "cthugha.h"

// test, if joystick header files are available, and if 
// event handling is supported.
#if HAVE_LINUX_JOYSTICK_H
#  include <linux/joystick.h>
#  ifdef JS_EVENT_BUTTON
#    define HAVE_JOYSTICK 1
#  else
#    undef HAVE_JOYSTICK
#  endif
#  include <unistd.h>
#  include <sys/ioctl.h>
#endif

class Joystick {
    static int js;
    static int init;

    static int active;
public:
    static int useJoystick;

    static int axis[4];
    static int stiffAxis[4];

    static void run();
};

#endif
