#include "cthugha.h"
#include "Option.h"
#include "CoreOption.h"

static CoreOptionEntryList dummyEntries;

CoreOption light(-1, "light", dummyEntries);
CoreOption background(-1, "background", dummyEntries);
CoreOption fly(-1, "fly", dummyEntries);

Option & MeshSize = optionDummy;
Option & TextureQuality = optionDummy;
Option & Hints = optionDummy;
Option & Dither = optionDummy;
Option & Shade = optionDummy;


