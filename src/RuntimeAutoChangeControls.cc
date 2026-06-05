/** @file
 * Runtime AutoChanger control adapter.
 */

#include "RuntimeAutoChangeControls.h"

#include "AutoChanger.h"
#include "VideoDirector.h"

static int isAutoChangeOption(Option& option) {
    return (&option == &changeQuiet)
        || (&option == &changeWaitMin)
        || (&option == &changeWaitRandom)
        || (&option == &changeMsgTime)
        || (&option == &changeCumulativeFireLevel)
        || (&option == &lock)
        || (&option == &change_little);
}

void DefaultRuntimeAutoChangeControls::toggleLock() {
    lock.change(+1);
}

int DefaultRuntimeAutoChangeControls::changeAutoChangeOptionBy(
    Option& option, int by, RuntimeChangeSet& changes) {
    if (!isAutoChangeOption(option))
        return 0;

    option.change(by);
    changes.autoChangeChanged = 1;
    return 1;
}

int DefaultRuntimeAutoChangeControls::changeAutoChangeOptionTo(
    Option& option, const char* to, RuntimeChangeSet& changes) {
    if (!isAutoChangeOption(option))
        return 0;

    option.change(to);
    changes.autoChangeChanged = 1;
    return 1;
}
