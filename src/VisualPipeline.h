// Internal visual pipeline scaffold.

#ifndef __VISUAL_PIPELINE_H
#define __VISUAL_PIPELINE_H

#include "AudioFrame.h"
#include "AudioAnalyzer.h"

#include <vector>

class CthughaFrameBuffer;

class VisualFrameContext {
public:
    const AudioFrame* audioFrame;
    const AudioAnalysis* audioAnalysis;
    const AcousticContext* acousticContext;
    double now;
    double deltaT;

    VisualFrameContext();
};

class VisualModule {
public:
    virtual ~VisualModule();

    virtual void refresh() { }
    virtual void execute(CthughaFrameBuffer& frameBuffer,
        const VisualFrameContext& context) = 0;
};

class VisualPipeline {
    struct Entry {
        VisualModule* module;
        int owned;

        Entry(VisualModule* module_, int owned_)
            : module(module_)
            , owned(owned_) { }
    };

    std::vector<Entry> modules;

public:
    VisualPipeline();
    ~VisualPipeline();

    void clear();
    void add(VisualModule* module, int takeOwnership = 0);
    void refresh();
    void run(CthughaFrameBuffer& frameBuffer, const VisualFrameContext& context);
    int size() const;
};

#endif
