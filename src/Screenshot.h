#ifndef __SCREENSHOT_H
#define __SCREENSHOT_H

#include "cthugha.h"

extern char display_prt_file[];
struct DisplayConfig;
void configureScreenshot(const DisplayConfig& config);
char* prtFileName(const char* ext);

#endif
