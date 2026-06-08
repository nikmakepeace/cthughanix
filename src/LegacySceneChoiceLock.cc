// Legacy OptionOnOff-backed Scene choice lock adapter.

#include "LegacySceneChoiceLock.h"

#include "Option.h"

LegacySceneChoiceLock::LegacySceneChoiceLock(OptionOnOff& lockValue_)
    : lockValue(lockValue_) { }

int LegacySceneChoiceLock::enabled() const {
    return int(lockValue);
}

void LegacySceneChoiceLock::change(const char* to) {
    lockValue.change(to);
}
