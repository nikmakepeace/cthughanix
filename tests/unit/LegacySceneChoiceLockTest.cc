#include "LegacySceneChoiceLock.h"

#include "Option.h"

#include <cassert>

int cth_log_enabled(int) { return 0; }
int cth_log(int, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }

static void testLockMirrorsLegacyOption() {
    OptionOnOff lock("visual-lock", 0);
    LegacySceneChoiceLock sceneLock(lock);

    assert(sceneLock.enabled() == 0);
    sceneLock.change("on");
    assert(sceneLock.enabled() == 1);
    assert(int(lock) == 1);
    sceneLock.change("off");
    assert(sceneLock.enabled() == 0);
    assert(int(lock) == 0);
}

int main() {
    testLockMirrorsLegacyOption();
    return 0;
}
