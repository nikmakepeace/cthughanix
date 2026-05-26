// -*- c++ -*-

#ifndef __AUDIO_PROCESSOR_H
#define __AUDIO_PROCESSOR_H

#include "cthugha.h"
#include "CoreOption.h"
#include "Option.h"

class AudioProcessingOption : public Option {
    CoreOptionEntryList& entries;
    char initialEntry[256];

    int entryCount() const;
    int optNr(const char* name) const;

public:
    AudioProcessingOption(const char* name, CoreOptionEntryList& entries);

    void setInitialEntry(const char* entry);
    void changeToInitial();

    virtual void change(int by);
    virtual void change(const char* to);
    virtual const char* text() const;

    int process();
};

extern AudioProcessingOption audioProcessing;

#endif
