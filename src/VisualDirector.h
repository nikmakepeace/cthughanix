// Visual pipeline policy.

#ifndef __VISUAL_DIRECTOR_H
#define __VISUAL_DIRECTOR_H

#include "VisualPipelineSequence.h"

class CthughaBuffer;
class VisualPipeline;

class VisualDirector {
    int lastPcxSelection;

    int pcxSelectionChanged();
    void syncCurrentBuffer();
    void updatePipelineStages(VisualPipeline& pipeline, CthughaBuffer& buffer);

public:
    VisualDirector();

    VisualPipelineSequence defaultPipelineSequence() const;
    CthughaBuffer* configurePipeline(VisualPipeline& pipeline);
};

extern double paletteSmoothingChance;

#endif
