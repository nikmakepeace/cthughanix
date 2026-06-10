/** @file
 * Automatic scene-change policy driven by audio metrics and acoustic context.
 */

#ifndef CTHUGHA_SCENE_CHANGE_SCHEDULER_H
#define CTHUGHA_SCENE_CHANGE_SCHEDULER_H

#include "SceneChangeStatusProvider.h"

class AcousticContext;
class AutoChangeSettings;
class LogSink;
class MillisecondClock;
class RandomSource;
struct AudioMetrics;

/**
 * Mutation target for automatic changes.
 *
 * Automatic changes are broader than Scene-owned visual selections: they also
 * include presentation-screen and audio-processing choices, which live behind
 * display/audio runtime controls.
 */
class AutoChangeTarget {
public:
    /** Destroys the automatic-change target interface. */
    virtual ~AutoChangeTarget() { }

    /** Changes every eligible automatic-change subsystem. */
    virtual void changeAll() = 0;

    /**
     * Changes one eligible automatic-change subsystem.
     *
     * @param randomSource Random source used to choose the subsystem.
     */
    virtual void changeOne(RandomSource& randomSource) = 0;
};

/**
 * Observer for quiet-audio notifications emitted by automatic scene changes.
 */
class AutoChangeQuietObserver {
public:
    /** Destroys the quiet observer interface. */
    virtual ~AutoChangeQuietObserver() { }

    /**
     * Reports an ongoing quiet period.
     *
     * @param quietLength Milliseconds since the last noisy audio frame.
     * @return Nonzero when the quiet period was consumed and should restart.
     */
    virtual int observeQuiet(int quietLength) = 0;
};

class SceneChangeScheduler : public SceneChangeStatusProvider {
    AutoChangeTarget& changeTarget;
    const AutoChangeSettings& settings;
    AcousticContext& acousticContextValue;
    MillisecondClock& clock;
    RandomSource& randomSource;
    AutoChangeQuietObserver& quietObserver;
    LogSink& log;

    int quietSince;
    int waitTime;
    int lastChange;
    mutable char statusTextValue[512];

    int nextWaitTime();

public:
    /**
     * Creates the automatic scene changer.
     *
     * @param changeTarget_ Target used for automatic runtime mutations. The
     *        referenced object must outlive this scheduler.
     * @param settings_ Automatic scene-change settings. The referenced object
     *        must outlive this scheduler.
     * @param acousticContext_ Rolling acoustic state used for fire-triggered
     *        scene changes. The referenced object must outlive this scheduler.
     * @param clock_ Clock used for wait/quiet timing. The referenced object
     *        must outlive this scheduler.
     * @param randomSource_ Random source used for wait jitter. The referenced
     *        object must outlive this scheduler.
     * @param quietObserver_ Observer notified about quiet audio periods. The
     *        referenced object must outlive this scheduler.
     * @param log_ Diagnostic sink. The referenced object must outlive this
     *        scheduler.
     */
    SceneChangeScheduler(AutoChangeTarget& changeTarget_,
        const AutoChangeSettings& settings_,
        AcousticContext& acousticContext_, MillisecondClock& clock_,
        RandomSource& randomSource_, AutoChangeQuietObserver& quietObserver_,
        LogSink& log_);

    /** Releases automatic scene-change policy state. */
    ~SceneChangeScheduler();

    /**
     * Runs one automatic-change policy step using supplied audio metrics.
     *
     * @param metrics Metrics for the current visual audio frame.
     */
    void operator()(const AudioMetrics& metrics);

    /**
     * Applies the selected automatic change action.
     *
     * Uses the little option to choose between changing one eligible scene option
     * and changing the whole unlocked scene option set.
     */
    void change();

    /**
     * @return Pointer to instance-owned status text for the interface. The
     *         text is overwritten on the next status() call.
     */
    const char* status() const;

    /**
     * Returns current automatic scene-change status text.
     *
     * @return Pointer to instance-owned text valid until the next status call.
     */
    virtual const char* sceneChangeStatus() const;
};

#endif
