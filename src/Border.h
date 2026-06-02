// Visual border option and indexed-buffer mutation.

#ifndef __BORDER_H
#define __BORDER_H

#include "EffectControl.h"

class CthughaBuffer;
class VideoFrameContext;

extern EffectControl border;

void init_border();
void apply_border(CthughaBuffer& buffer, const VideoFrameContext& context, int borderMode);

#endif
