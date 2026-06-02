#include "cthugha.h"
#include "VideoFilters.h"
#include "VideoFilterchain.h"
#include "VideoFilterchainFactory.h"

VideoFilterchainFactory::VideoFilterchainFactory() { }

VideoFilterchain* VideoFilterchainFactory::create(const VideoFilterchainSequence& sequence) const {
    VideoFilterchain* filterchain = new VideoFilterchain();
    PaletteFilter* paletteFilter = 0;

    filterchain->setStageSequence(sequence.sequence());

    if (sequence.includes(VideoFilterchainSequence::ImageStage))
        filterchain->add(VideoFilterchainSequence::ImageStage, new ImageFilter(), 1);
    if (sequence.includes(VideoFilterchainSequence::BorderStage))
        filterchain->add(VideoFilterchainSequence::BorderStage, new BorderFilter(), 1);
    if (sequence.includes(VideoFilterchainSequence::FlameStage))
        filterchain->add(VideoFilterchainSequence::FlameStage, new FlameFilter(), 1);
    if (sequence.includes(VideoFilterchainSequence::TranslateStage))
        filterchain->add(VideoFilterchainSequence::TranslateStage, new TranslateFilter(), 1);
    if (sequence.includes(VideoFilterchainSequence::WaveStage))
        filterchain->add(VideoFilterchainSequence::WaveStage, new WaveFilter(), 1);
    if (sequence.includes(VideoFilterchainSequence::TextStage))
        filterchain->add(VideoFilterchainSequence::TextStage, new TextInjectionFilter(), 1);
    if (sequence.includes(VideoFilterchainSequence::FrameCommitStage))
        filterchain->add(VideoFilterchainSequence::FrameCommitStage, new FrameCommitFilter(), 1);
    if (sequence.includes(VideoFilterchainSequence::PaletteStage)) {
        paletteFilter = new PaletteFilter();
        filterchain->add(VideoFilterchainSequence::PaletteStage, paletteFilter, 1);
        filterchain->setFramePalette(&paletteFilter->framePalette());
    }
    if (sequence.includes(VideoFilterchainSequence::FlashlightStage))
        filterchain->add(VideoFilterchainSequence::FlashlightStage, new FlashlightFilter(), 1);
    if (sequence.includes(VideoFilterchainSequence::IndexedFrameStage))
        filterchain->add(VideoFilterchainSequence::IndexedFrameStage, new IndexedFrameFilter(), 1);

    CTH_DEBUG("video filterchain factory: created filterchain=%p stages=%d filters=%d\n",
        filterchain, int(sequence.sequence().size()), filterchain->size());
    return filterchain;
}

void VideoFilterchainFactory::refresh(VideoFilterchain& filterchain,
    const VideoFilterchainSequence& sequence) const {
    CTH_DEBUG("video filterchain factory: refreshing filterchain=%p stages=%d filters=%d\n",
        &filterchain, int(sequence.sequence().size()), filterchain.size());
    filterchain.refresh();
}
