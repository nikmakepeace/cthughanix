#ifndef __VIDEO_FILTERS_H
#define __VIDEO_FILTERS_H

#include "Flame.h"
#include "FramePalette.h"
#include "Image.h"
#include "PaletteTransition.h"
#include "Translate.h"
#include "VideoFilterchain.h"
#include "VideoFilterchainSequence.h"
#include "Wave.h"

#include <string>

class BitmapFont;
class PaletteEntry;

enum TextInjectionHorizontalAlign {
    TextInjectionAlignLeft,
    TextInjectionAlignCenter,
    TextInjectionAlignRight
};

enum TextInjectionVerticalAlign {
    TextInjectionAlignTop,
    TextInjectionAlignMiddle,
    TextInjectionAlignBottom
};

// Contract: one-shot pixel injector. Writes the selected image into the active
// buffer, and can mirror the same pixels into passive for immediate display.
class ImageFilter : public VideoFilter {
    const IndexedImage* image;
    ImagePlacement placement;
    int overlayPassiveBuffer;

public:
    ImageFilter();

    void setImage(const IndexedImage* image_);
    void setPlacement(const ImagePlacement& placement_);
    void setOverlayPassiveBuffer(int enabled);
    void execute(VideoFrame& frame);
};

// Contract: feedback filter. Runs the selected Flame over the buffer, usually
// reading passive pixels and hidden border rows while writing active pixels.
class FlameFilter : public VideoFilter {
    const Flame* flame;
    int generalFlame;
    FlameLookupTables lookupTables;

public:
    FlameFilter();

    void setFlame(const Flame* flame_);
    void setGeneralFlame(int generalFlame_);
    void execute(VideoFrame& frame);
};

// Contract: coordinate remap filter. The Translate executor owns any
// active/passive swap it needs before remapping passive pixels into active.
class TranslateFilter : public VideoFilter {
    Translate translate;

public:
    TranslateFilter();

    void setTranslate(const TranslationTable& table);
    void execute(VideoFrame& frame);
};

// Contract: sound-reactive drawing filter. Draws into active pixels using the
// current frame context plus wave-local state; it does not commit the frame.
class WaveFilter : public VideoFilter {
    Wave* wave;
    WaveConfig config;
    WaveState state;
    WaveLookupTables lookupTables;
    int configured;
    int needsConfiguration;

public:
    WaveFilter();

    void setWave(Wave* wave_, const WaveConfig& config_);
    void execute(VideoFrame& frame);
};

// Contract: indexed-buffer text injector. Wraps CP437 text at word boundaries,
// draws it into active pixels, and lets later frame commits/feed-back stages
// make it part of the visual material.
class TextInjectionFilter : public VideoFilter {
    const BitmapFont* font;
    std::string message;
    int framesRemaining;
    int inkColor;
    int marginPixels;
    TextInjectionHorizontalAlign horizontalAlign;
    TextInjectionVerticalAlign verticalAlign;

public:
    TextInjectionFilter();

    void setMessage(const char* message_, int frameCount);
    void setInkColor(int color);
    void setPlacement(TextInjectionHorizontalAlign horizontalAlign_,
        TextInjectionVerticalAlign verticalAlign_, int marginPixels_);
    void execute(VideoFrame& frame);
};

// Contract: frame boundary. Emits optional diagnostics, then swaps active and
// passive so the finished indexed image becomes the display source.
class FrameCommitFilter : public VideoFilter {
    const char* flameName;
    const char* waveName;
    const char* waveScaleName;
    const char* tableName;

public:
    FrameCommitFilter();

    void setSceneNames(const char* flameName_, const char* waveName_,
        const char* waveScaleName_, const char* tableName_);
    void execute(VideoFrame& frame);
};

// Contract: palette post-filter. Reads acoustic context and writes temporary
// flashlight output into the frame palette; it ignores indexed pixels.
class FlashlightFilter : public VideoFilter {
public:
    FlashlightFilter();

    void execute(VideoFrame& frame);
};

// Contract: hidden-row writer. Fills the active buffer border rows used by
// flame feedback; visible pixels are left to later stages.
class BorderFilter : public VideoFilter {
    int borderMode;

public:
    BorderFilter();

    void setBorderMode(int borderMode_);
    void execute(VideoFrame& frame);
};

// Contract: palette transition filter. Advances the display-facing
// FramePalette toward the configured target; it ignores indexed pixels.
class PaletteFilter : public VideoFilter {
    PaletteTransition transition;
    FramePalette framePaletteValue;

public:
    PaletteFilter();

    FramePalette& framePalette();
    int needsTarget(PaletteEntry* paletteEntry) const;
    void setTargetPalette(PaletteEntry* paletteEntry, int frameBudget,
        const PaletteTransitionStrategy& strategy);
    void execute(VideoFrame& frame);
};

// Contract: final display export. Publishes the committed indexed pixels and
// frame palette as a driver-facing frame descriptor.
class IndexedFrameFilter : public VideoFilter {
public:
    IndexedFrameFilter();

    void execute(VideoFrame& frame);
};

FramePalette* framePaletteFromFilterchain(VideoFilterchain& filterchain);

#endif
