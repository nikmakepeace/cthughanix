#include "cthugha.h"
#include "EffectControl.h"

#include <ctype.h>

EffectChoice::EffectChoice(const char* n, const char* d, int inUse)
    : use("", inUse) {
    name = new char[strlen(n ? n : "none") + 1];
    strcpy(name, n ? n : "none");

    desc = new char[strlen(d ? d : "") + 1];
    strcpy(desc, d ? d : "");
}

// compare up to first SPACE in other
int EffectChoice::sameName(const char* other) {

    const char* n = name;
    while ((*other != '\0') && (*other != ' ') && (*n != '\0')) {
        if (toupper(*other) != toupper(*n))
            return 0;
        other++;
        n++;
    }
    if ((*n == '\0') && ((*other == '\0') || (*other == ' ')))
        return 1;
    return 0;
}

int OnEntry::sameName(const char* other) {

    if (!strncasecmp("yes", other, 3))
        return 1;
    else if (!strncasecmp("on", other, 2))
        return 1;
    else if (!strncasecmp("1", other, 1))
        return 1;
    else
        return 0;
}

int OffEntry::sameName(const char* other) {

    if (!strncasecmp("no", other, 2))
        return 1;
    else if (!strncasecmp("off", other, 3))
        return 1;
    else if (!strncasecmp("0", other, 1))
        return 1;
    else
        return 0;
}
