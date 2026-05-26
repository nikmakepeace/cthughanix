// Visual flashlight option.

#ifndef __FLASHLIGHT_H
#define __FLASHLIGHT_H

#include "CoreOption.h"

class CthughaFrameBuffer;
class VisualFrameContext;

extern CoreOption flashlight;

void init_flashlight();
void apply_flashlight(CthughaFrameBuffer& frameBuffer, const VisualFrameContext& context);

#endif
