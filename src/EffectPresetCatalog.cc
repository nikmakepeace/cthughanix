// Preset slots for whole-effect snapshots.

#include "cthugha.h"
#include "EffectPresetCatalog.h"
#include "EffectControl.h"

EffectPresetCatalog effectPresetCatalog;

int EffectPresetSlot::indexOf(const EffectControl& option) const {
    for (unsigned int i = 0; i < values.size(); i++)
        if (values[i].option == &option)
            return int(i);

    return -1;
}

void EffectPresetSlot::setValue(EffectControl& option, int selection) {
    int index = indexOf(option);
    if (index >= 0) {
        values[index].selection = selection;
        return;
    }

    values.push_back(Value(&option, selection));
}

int EffectPresetSlot::value(const EffectControl& option) const {
    int index = indexOf(option);
    return (index >= 0) ? values[index].selection : 0;
}

void EffectPresetSlot::saveCurrentValues() {
    for (EffectControl* option = EffectControl::firstRegistered(); option != NULL;
         option = option->nextRegistered()) {
        setValue(*option, option->currentN());
    }
}

void EffectPresetSlot::restoreValues() {
    for (EffectControl* option = EffectControl::firstRegistered(); option != NULL;
         option = option->nextRegistered()) {
        option->setValue(value(*option));
        option->change(0, 0);
    }
}

int EffectPresetCatalog::validSlot(int slot) const {
    return (slot >= 0) && (slot < PresetSlotCount);
}

int EffectPresetCatalog::slotCount() const {
    return PresetSlotCount;
}

void EffectPresetCatalog::save(int slot) {
    if (!validSlot(slot))
        return;

    slots[slot].saveCurrentValues();
}

void EffectPresetCatalog::restore(int slot) {
    if (!validSlot(slot))
        return;

    EffectControl::save();
    slots[slot].restoreValues();
}

void EffectPresetCatalog::setValue(int slot, EffectControl& option, int selection) {
    if (!validSlot(slot))
        return;

    slots[slot].setValue(option, selection);
}

int EffectPresetCatalog::value(int slot, const EffectControl& option) const {
    if (!validSlot(slot))
        return 0;

    return slots[slot].value(option);
}
