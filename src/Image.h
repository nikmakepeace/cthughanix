#ifndef __IMAGE_H
#define __IMAGE_H

#include "ColorPalette.h"
#include "CoreOption.h"

class CthughaBuffer;

/**
 * Target buffer dimensions for image loaders.
 */
struct ImageLoadTarget {
    /** Target display/buffer width in pixels. */
    int width;

    /** Target display/buffer height in pixels. */
    int height;

    /**
     * @param width_ Target width in pixels.
     * @param height_ Target height in pixels.
     */
    ImageLoadTarget(int width_, int height_)
        : width(width_)
        , height(height_) { }
};

/**
 * Owned 8-bit indexed image.
 */
class IndexedImage {
    char* nameValue;
    unsigned char* pixelsValue;
    int widthValue;
    int heightValue;

    IndexedImage(const IndexedImage&);
    IndexedImage& operator=(const IndexedImage&);

public:
    /**
     * Allocates an indexed image.
     *
     * @param name Image display/option name.
     * @param width Width in pixels.
     * @param height Height in pixels.
     */
    IndexedImage(const char* name, int width, int height);
    ~IndexedImage();

    /** @return Image display/option name. */
    const char* name() const;

    /** @return Width in pixels. */
    int width() const;

    /** @return Height in pixels. */
    int height() const;

    /** @return Number of indexed pixels, width * height. */
    int size() const;

    /** @return Read-only 8-bit palette-index pixels. */
    const unsigned char* pixels() const;

    /** @return Mutable 8-bit palette-index pixels for loaders. */
    unsigned char* mutablePixels();
};

/**
 * Clipped source/destination rectangle for drawing an image into a buffer.
 *
 * left/top preserve the requested placement. The source/destination/size fields
 * describe the visible copy rectangle after clipping against the target buffer.
 */
class ImagePlacement {
public:
    /** Requested image left edge relative to the target buffer, in pixels. */
    int left;

    /** Requested image top edge relative to the target buffer, in pixels. */
    int top;

    /** Source x coordinate inside the image, in pixels. */
    int sourceX;

    /** Source y coordinate inside the image, in pixels. */
    int sourceY;

    /** Destination x coordinate inside the buffer, in pixels. */
    int destinationX;

    /** Destination y coordinate inside the buffer, in pixels. */
    int destinationY;

    /** Visible copy width in pixels. */
    int width;

    /** Visible copy height in pixels. */
    int height;

    ImagePlacement();

    /**
     * Computes the clipped visible rectangle.
     *
     * @param left_ Requested image left edge relative to the buffer, in pixels.
     * @param top_ Requested image top edge relative to the buffer, in pixels.
     * @param imageWidth Source image width in pixels.
     * @param imageHeight Source image height in pixels.
     * @param bufferWidth Target buffer width in pixels.
     * @param bufferHeight Target buffer height in pixels.
     */
    ImagePlacement(int left_, int top_, int imageWidth, int imageHeight,
        int bufferWidth, int bufferHeight);

    /** @return Nonzero when width and height describe visible pixels. */
    int visible() const;
};

/**
 * Chooses where an image should be drawn in a target buffer.
 */
class ImagePlacementStrategy {
public:
    virtual ~ImagePlacementStrategy();

    /**
     * @param image Image to place.
     * @param bufferWidth Target buffer width in pixels.
     * @param bufferHeight Target buffer height in pixels.
     * @return Clipped image placement.
     */
    virtual ImagePlacement choose(const IndexedImage& image, int bufferWidth,
        int bufferHeight) const = 0;
};

/**
 * Random placement strategy that keeps at least part of the image visible.
 */
class RandomLegalImagePlacementStrategy : public ImagePlacementStrategy {
public:
    /**
     * @param image Image to place.
     * @param bufferWidth Target buffer width in pixels.
     * @param bufferHeight Target buffer height in pixels.
     * @return Random legal placement, clipped to the target buffer.
     */
    ImagePlacement choose(const IndexedImage& image, int bufferWidth,
        int bufferHeight) const;
};

/**
 * Image option entry owning optional image pixels and source palette.
 */
class ImageEntry : public CoreOptionEntry {
    IndexedImage* imageValue;
    ColorPalette* paletteValue;

    ImageEntry(const ImageEntry&);
    ImageEntry& operator=(const ImageEntry&);

public:
    /**
     * Creates an image option entry.
     *
     * @param name Option/display name.
     * @param desc Human-readable description.
     * @param image Owned indexed image, or NULL.
     * @param palette Owned source palette, or NULL.
     */
    ImageEntry(const char* name, const char* desc, IndexedImage* image = 0,
        ColorPalette* palette = 0);
    ~ImageEntry();

    /** @return Owned image pointer, or NULL. */
    const IndexedImage* image() const;

    /** @return Owned source palette pointer, or NULL. */
    const ColorPalette* palette() const;
};

/**
 * CoreOption that loads/selects indexed image entries.
 */
class ImageOption : public CoreOption {
public:
    ImageOption(int buffer, const char* name);

    /** @return Currently selected image entry, or NULL. */
    ImageEntry* currentImageEntry();

    /** @return Currently selected indexed image, or NULL. */
    const IndexedImage* currentImage();

    /**
     * Loads supported image formats from configured image paths.
     *
     * @param targetWidth Display/buffer width in pixels.
     * @param targetHeight Display/buffer height in pixels.
     * @return Combined loader result from the configured image catalog loaders.
     */
    int loadImages(int targetWidth, int targetHeight);
};

#endif
