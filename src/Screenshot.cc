#include "Screenshot.h"
#include "defaults.h"

char display_prt_file[PATH_MAX] = DEFAULT_SCREENSHOT_FILE_PREFIX; /* filename used by PrtScrn */

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
