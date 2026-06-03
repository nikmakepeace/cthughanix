#ifndef __FPS_OVERLAY_H
#define __FPS_OVERLAY_H

#include "OverlaySource.h"

class FpsOverlay {
public:
    static void append(OverlayCommands& commands, double fps, int enabled);
};

#endif
