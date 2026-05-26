// Visual border option and frame-buffer mutation.

#ifndef __BORDER_H
#define __BORDER_H

#include "CoreOption.h"

class CthughaFrameBuffer;
class VisualFrameContext;

extern CoreOption border;

void init_border();
void apply_border(CthughaFrameBuffer& frameBuffer, const VisualFrameContext& context);

#endif
