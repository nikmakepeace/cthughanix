// Current audio settings snapshot.

#ifndef __SETTINGS_H
#define __SETTINGS_H

#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

class Settings {
public:
    int audioInputMode;
    int soundDSPMethod;
    int silent;
    char fileName[PATH_MAX];

    Settings();

    void refreshFromCurrentOptions();
    static Settings fromCurrentOptions();
};

#endif
