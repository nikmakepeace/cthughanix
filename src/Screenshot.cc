#include "Screenshot.h"
#include "Configuration.h"

char display_prt_file[PATH_MAX] = ""; /* filename used by PrtScrn */

void configureScreenshot(const DisplayConfig& config) {
    strncpy(display_prt_file, config.screenshotFilePrefix.c_str(), PATH_MAX);
    display_prt_file[PATH_MAX - 1] = '\0';
}

char* prtFileName(const char* ext) {
    static char name[PATH_MAX];
    static int count = 0;

    if (count == 0) {
        snprintf(name, sizeof(name), "%s.%s", display_prt_file, ext);
    } else {
        snprintf(name, sizeof(name), "%s.%d.%s", display_prt_file, count, ext);
    }
    count++;

    return name;
}
