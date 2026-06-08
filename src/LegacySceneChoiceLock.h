// Legacy OptionOnOff-backed Scene choice lock adapter.

#ifndef CTHUGHA_LEGACY_SCENE_CHOICE_LOCK_H
#define CTHUGHA_LEGACY_SCENE_CHOICE_LOCK_H

#include "SceneChoiceSelection.h"

class OptionOnOff;

/**
 * SceneChoiceLock view over a legacy OptionOnOff lock.
 */
class LegacySceneChoiceLock : public SceneChoiceLock {
    OptionOnOff& lockValue;

public:
    /** Creates a Scene-facing lock over a borrowed legacy option. */
    explicit LegacySceneChoiceLock(OptionOnOff& lockValue_);

    /** @return Nonzero when the lock is enabled. */
    virtual int enabled() const;

    /** Changes the lock from text such as "on" or "off". */
    virtual void change(const char* to);
};

#endif
