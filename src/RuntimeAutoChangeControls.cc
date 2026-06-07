/** @file
 * Runtime automatic scene-change control adapter.
 */

#include "RuntimeAutoChangeControls.h"

#include "AutoChangeControls.h"
#include "VideoDirector.h"

static int isAutoChangeOption(Option& option) {
    return (&option == &changeMsgTime);
}

DefaultRuntimeAutoChangeControls::DefaultRuntimeAutoChangeControls(
    AutoChangeControls& autoChangeControls_)
    : autoChangeControls(autoChangeControls_) { }

static void markChanged(RuntimeChangeSet& changes) {
    changes.autoChangeChanged = 1;
}

void DefaultRuntimeAutoChangeControls::toggleLock() {
    autoChangeControls.toggleLock();
}

int DefaultRuntimeAutoChangeControls::changeAutoChangeOptionBy(
    Option& option, int by, RuntimeChangeSet& changes) {
    if (autoChangeControls.changeOptionBy(option, by)) {
        markChanged(changes);
        return 1;
    }

    if (!isAutoChangeOption(option))
        return 0;

    option.change(by);
    markChanged(changes);
    return 1;
}

int DefaultRuntimeAutoChangeControls::changeAutoChangeOptionTo(
    Option& option, const char* to, RuntimeChangeSet& changes) {
    if (autoChangeControls.changeOptionTo(option, to)) {
        markChanged(changes);
        return 1;
    }

    if (!isAutoChangeOption(option))
        return 0;

    option.change(to);
    markChanged(changes);
    return 1;
}
