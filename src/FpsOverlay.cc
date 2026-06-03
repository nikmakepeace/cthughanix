#include "FpsOverlay.h"
#include "DisplayDevice.h"

#include <stdio.h>

void FpsOverlay::append(OverlayCommands& commands, double fps, int enabled) {
    if (!enabled)
        return;

    char text[32];
    snprintf(text, sizeof(text), "fps: %5.2f", fps);
    commands.addText(text, 0.0, 'r', TEXT_COLOR_NORMAL, 1);
}
