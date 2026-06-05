/** @file
 * Runtime Option adapters for the OSS mixer panel.
 */

#include "cthugha.h"
#include "Mixer.h"
#include "Interface.h"

#include <stdio.h>
#include <stdlib.h>

class MixerControls::MixerVolumeOption : public Option {
    MixerSession& sessionValue;
    size_t channelIndex;

    void syncValue() {
        const std::vector<MixerChannel>& channels = sessionValue.channels();
        value = (channelIndex < channels.size())
            ? channels[channelIndex].encodedVolume
            : 0;
    }

public:
    MixerVolumeOption(MixerSession& session, size_t channelIndex_)
        : Option(NULL)
        , sessionValue(session)
        , channelIndex(channelIndex_) {
        syncValue();
    }

    virtual void setValue(int value_) {
        sessionValue.setChannelValue(channelIndex, value_);
        syncValue();
    }

    virtual void change(int by) {
        sessionValue.changeChannelBy(channelIndex, by);
        syncValue();
    }

    virtual void change(const char* to) {
        char* end = NULL;
        long parsed = strtol(to, &end, 0);
        if ((to == end) || ((end != NULL) && (*end != '\0'))) {
            CTH_ERROR("Not a mixer volume value `%s'.\n", to);
            return;
        }
        sessionValue.setChannelValue(channelIndex, int(parsed));
        syncValue();
    }

    virtual const char* text() const {
        static char str[128];
        const std::vector<MixerChannel>& channels = sessionValue.channels();
        if (channelIndex >= channels.size())
            return "";

        const MixerChannel& channel = channels[channelIndex];
        snprintf(str, sizeof(str), "%3d:%-3d%c",
            channel.encodedVolume % 256, channel.encodedVolume / 256,
            channel.active ? '*' : ' ');
        return str;
    }
};

MixerControls::MixerControls(MixerSession& session)
    : sessionValue(session)
    , optionsValue()
    , labelsValue()
    , elementsValue()
    , elementPointersValue() {
    rebuild();
}

MixerControls::~MixerControls() { }

void MixerControls::rebuild() {
    optionsValue.clear();
    labelsValue.clear();
    elementsValue.clear();
    elementPointersValue.clear();

    const std::vector<MixerChannel>& channels = sessionValue.channels();
    optionsValue.reserve(channels.size());

    for (size_t i = 0; i < channels.size(); i++) {
        optionsValue.push_back(
            std::unique_ptr<MixerVolumeOption>(
                new MixerVolumeOption(sessionValue, i)));
    }
}

size_t MixerControls::optionCount() const {
    return optionsValue.size();
}

Option* MixerControls::optionAt(size_t index) {
    if (index >= optionsValue.size())
        return NULL;
    return optionsValue[index].get();
}

int MixerControls::findOption(const Option& option) const {
    for (size_t i = 0; i < optionsValue.size(); i++) {
        if (optionsValue[i].get() == &option)
            return int(i);
    }
    return -1;
}

int MixerControls::changeOptionBy(Option& option, int by) {
    int index = findOption(option);
    if (index < 0)
        return 0;

    optionsValue[size_t(index)]->change(by);
    return 1;
}

int MixerControls::changeOptionTo(Option& option, const char* to) {
    int index = findOption(option);
    if (index < 0)
        return 0;

    optionsValue[size_t(index)]->change(to);
    return 1;
}
