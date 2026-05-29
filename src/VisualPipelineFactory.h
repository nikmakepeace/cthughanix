#ifndef __VISUAL_PIPELINE_FACTORY_H
#define __VISUAL_PIPELINE_FACTORY_H

#include "VisualPipelineSequence.h"

class VisualPipeline;

class VisualPipelineFactory {
public:
    VisualPipelineFactory();

    VisualPipeline* create(const VisualPipelineSequence& sequence) const;
    void refresh(VisualPipeline& pipeline, const VisualPipelineSequence& sequence) const;
};

#endif
