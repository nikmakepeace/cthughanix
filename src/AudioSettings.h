// Current audio settings snapshot.

#ifndef __AUDIO_SETTINGS_H
#define __AUDIO_SETTINGS_H

#include <limits.h>
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

struct AudioConfig;

class AudioSettings {
public:
    int audioInputMode;
    int soundDSPMethod;
    int silent;
    char fileName[PATH_MAX];

    AudioSettings();

    void refreshFromConfig(const AudioConfig& config);
    static AudioSettings fromConfig(const AudioConfig& config);
};

#endif
