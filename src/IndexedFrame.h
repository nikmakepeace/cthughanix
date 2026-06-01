#ifndef __INDEXED_FRAME_H
#define __INDEXED_FRAME_H

class FramePalette;

class IndexedFrame {
public:
    const unsigned char* pixels;
    int width;
    int height;
    int pitch;
    FramePalette* framePalette;

    IndexedFrame()
        : pixels(0)
        , width(0)
        , height(0)
        , pitch(0)
        , framePalette(0) { }

    IndexedFrame(const unsigned char* pixels_, int width_, int height_,
        int pitch_, FramePalette* framePalette_)
        : pixels(pixels_)
        , width(width_)
        , height(height_)
        , pitch(pitch_)
        , framePalette(framePalette_) { }

    int valid() const {
        return pixels != 0 && width > 0 && height > 0 && pitch >= width;
    }
};

#endif
