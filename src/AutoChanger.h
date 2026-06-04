// -*- c++ -*-

#ifndef __AUTO_CHANGER_H
#define __AUTO_CHANGER_H

#include "Option.h"

//
// automatically change the display, based on timeouts and noise
//

extern OptionTime changeQuiet; /* change after quiet-pause (1.5 sec) */
extern OptionTime changeWaitMin; /* min time between change (3 sec) */
extern OptionTime changeWaitRandom; /* extra random wait-time (11 sec) */
extern OptionInt changeCumulativeFireLevel;
extern OptionOnOff lock; /* change automatically */
extern OptionOnOff change_little; /* only change one options */

class SceneCommands;
struct AutoChangeConfig;

class AutoChanger {
    SceneCommands& sceneCommands;

    int quietSince;
    int waitTime;
    int lastChange;

public:
    /**
     * Creates the automatic scene changer.
     *
     * @param sceneCommands_ Scene command facade used for all visual mutations.
     *        The referenced object must outlive this AutoChanger.
     */
    AutoChanger(SceneCommands& sceneCommands_);
    ~AutoChanger();

    /**
     * Runs one automatic-change policy step for the current audio frame.
     *
     * Reads global AudioAnalyzer/AcousticContext metrics, tracks quiet duration
     * in milliseconds from gettime(), may emit quiet-message cues through
     * VideoDirector, and may mutate scene options through SceneCommands.
     */
    void operator()();

    /**
     * Applies the selected automatic change action.
     *
     * Uses the little option to choose between changing one eligible EffectControl
     * and changing the whole unlocked scene option set.
     */
    void change();

    /**
     * @return Pointer to static status text for the interface. The text is
     *         overwritten on the next status() call.
     */
    const char* status();
};
extern AutoChanger* autoChanger;

void configureAutoChanger(const AutoChangeConfig& config);

#endif
