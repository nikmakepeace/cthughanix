/** @file
 * Runtime frame-generator control port and default implementation.
 */

#ifndef CTHUGHA_RUNTIME_FRAME_GENERATOR_CONTROLS_H
#define CTHUGHA_RUNTIME_FRAME_GENERATOR_CONTROLS_H

class FrameGeneratorRuntime;

/** Controls runtime frame-generation policy options. */
class RuntimeFrameGeneratorControls {
public:
    /** Destroys the frame-generator controls interface. */
    virtual ~RuntimeFrameGeneratorControls() { }

    /**
     * Sets the probability that palette changes smooth.
     *
     * @param chance Probability in the range 0..1.
     */
    virtual void changePaletteSmoothingChanceTo(double chance) = 0;
};

/** RuntimeFrameGeneratorControls backed by the owned frame generator. */
class DefaultRuntimeFrameGeneratorControls
    : public RuntimeFrameGeneratorControls {
    FrameGeneratorRuntime& frameGenerator;

public:
    /**
     * Creates controls over an application-owned frame generator.
     *
     * @param frameGenerator_ Frame generator to mutate.
     */
    explicit DefaultRuntimeFrameGeneratorControls(
        FrameGeneratorRuntime& frameGenerator_);

    /** Sets the probability that palette changes smooth. */
    virtual void changePaletteSmoothingChanceTo(double chance);
};

#endif
