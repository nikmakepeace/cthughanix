// Preset slots for whole-effect snapshots.

#ifndef __EFFECT_PRESET_CATALOG_H
#define __EFFECT_PRESET_CATALOG_H

#include <vector>

class EffectControl;

class EffectPresetSlot {
    struct Value {
        EffectControl* option;
        int selection;

        Value(EffectControl* option_, int selection_)
            : option(option_)
            , selection(selection_) { }
    };

    std::vector<Value> values;

    int indexOf(const EffectControl& option) const;

public:
    void setValue(EffectControl& option, int selection);
    int value(const EffectControl& option) const;
    void saveCurrentValues();
    void restoreValues();
};

class EffectPresetCatalog {
    static const int PresetSlotCount = 10;

    EffectPresetSlot slots[PresetSlotCount];

    int validSlot(int slot) const;

public:
    int slotCount() const;

    void save(int slot);
    void restore(int slot);
    void setValue(int slot, EffectControl& option, int selection);
    int value(int slot, const EffectControl& option) const;
};

extern EffectPresetCatalog effectPresetCatalog;

#endif
