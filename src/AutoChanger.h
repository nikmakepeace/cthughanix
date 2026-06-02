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

class AutoChanger {
    SceneCommands& sceneCommands;

    int quietSince;
    int waitTime;
    int lastChange;

public:
    AutoChanger(SceneCommands& sceneCommands_);
    ~AutoChanger();

    void operator()();

    void change();

    const char* status(); // print status information
};
extern AutoChanger* autoChanger;

#endif
