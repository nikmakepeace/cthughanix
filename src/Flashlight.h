// Visual flashlight option.

#ifndef __FLASHLIGHT_H
#define __FLASHLIGHT_H

#include "CoreOption.h"

class CthughaBuffer;
class FramePalette;
class VisualFrameContext;

extern CoreOption flashlight;

void init_flashlight();
void apply_flashlight(FramePalette& framePalette, const VisualFrameContext& context);

#endif
