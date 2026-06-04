// Runtime ini persistence and effect-control ini adapters.

#ifndef CTHUGHA_INI_FILES_H
#define CTHUGHA_INI_FILES_H

#include "Configuration.h"

struct ContinuationIniConfig {
    SceneConfig scene;
    int showFpsEnabled;

    ContinuationIniConfig();
};

int remove_continuation_ini(const PathConfig& paths);
void configure_ini_persistence(const Config& config);
int write_ini(const Config& config);
int write_ini();
int write_continuation_ini(const ContinuationIniConfig& config);

#endif
