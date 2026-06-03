#include "WaveObject.h"

#include "cthugha.h"

namespace {

class ObjectEntry : public EffectChoice {
public:
    WObject* obj;
    int ownsObject;

    ObjectEntry(WObject* o, const char* name, const char* desc)
        : EffectChoice(name, desc)
        , obj(o)
        , ownsObject(0) { }

    ObjectEntry(const char* name, const char* desc)
        : EffectChoice(name, desc)
        , obj(NULL)
        , ownsObject(0) { }

    ~ObjectEntry() {
        if (ownsObject)
            delete[] obj;
        obj = NULL;
        ownsObject = 0;
    }

    void setOwnedObject(WObject* object) {
        if (ownsObject)
            delete[] obj;
        obj = object;
        ownsObject = 1;
    }
};

ObjectEntry* asObjectEntry(EffectChoice* entry) {
    return static_cast<ObjectEntry*>(entry);
}

const ObjectEntry* asObjectEntry(const EffectChoice* entry) {
    return static_cast<const ObjectEntry*>(entry);
}

/* An H, for Harald, presumably */
WObject letterH[] = {
    { { 0, 0, 0 }, { 1, 0, 0 } },
    { { 1, 0, 0 }, { 1, 1, 0 } },
    { { 1, 1, 0 }, { 2, 1, 0 } },
    { { 2, 1, 0 }, { 2, 0, 0 } },
    { { 2, 0, 0 }, { 3, 0, 0 } },
    { { 3, 0, 0 }, { 3, 3, 0 } },
    { { 3, 3, 0 }, { 2, 3, 0 } },
    { { 2, 3, 0 }, { 2, 2, 0 } },
    { { 2, 2, 0 }, { 1, 2, 0 } },
    { { 1, 2, 0 }, { 1, 3, 0 } },
    { { 1, 3, 0 }, { 0, 3, 0 } },
    { { 0, 3, 0 }, { 0, 0, 0 } },
    { { 0, 0, 1 }, { 1, 0, 1 } },
    { { 1, 0, 1 }, { 1, 1, 1 } },
    { { 1, 1, 1 }, { 2, 1, 1 } },
    { { 2, 1, 1 }, { 2, 0, 1 } },
    { { 2, 0, 1 }, { 3, 0, 1 } },
    { { 3, 0, 1 }, { 3, 3, 1 } },
    { { 3, 3, 1 }, { 2, 3, 1 } },
    { { 2, 3, 1 }, { 2, 2, 1 } },
    { { 2, 2, 1 }, { 1, 2, 1 } },
    { { 1, 2, 1 }, { 1, 3, 1 } },
    { { 1, 3, 1 }, { 0, 3, 1 } },
    { { 0, 3, 1 }, { 0, 0, 1 } },
    { { 0, 0, 1 }, { 0, 0, 0 } },
    { { 1, 0, 1 }, { 1, 0, 0 } },
    { { 1, 1, 1 }, { 1, 1, 0 } },
    { { 2, 1, 1 }, { 2, 1, 0 } },
    { { 2, 0, 1 }, { 2, 0, 0 } },
    { { 3, 0, 1 }, { 3, 0, 0 } },
    { { 3, 3, 1 }, { 3, 3, 0 } },
    { { 2, 3, 1 }, { 2, 3, 0 } },
    { { 2, 2, 1 }, { 2, 2, 0 } },
    { { 1, 2, 1 }, { 1, 2, 0 } },
    { { 1, 3, 1 }, { 1, 3, 0 } },
    { { 0, 3, 1 }, { 0, 3, 0 } },
    { { -1, -1, -1 }, { -1, -1, -1 } },
};


}

EffectChoice* _objects[] = { new ObjectEntry(letterH, "bigH", "Big H") };
int _nObjects = sizeof(_objects) / sizeof(EffectChoice*);

WObject* waveObjectEntryObject(EffectChoice* entry) {
    return (entry != NULL) ? asObjectEntry(entry)->obj : NULL;
}

int waveObjectEntryOwnsObject(const EffectChoice* entry) {
    return (entry != NULL) ? asObjectEntry(entry)->ownsObject : 0;
}

EffectChoice* read_object(
    FILE* file, const char* name, const char* /* dir */, const char* /*total_name*/) {
    char dummy[256];
    int i, j, nlines, x1, y1, z1, x2, y2, z2, mx, my, mz;

    ObjectEntry* new_obj = new ObjectEntry(name, "");

    /* count relevant lines, discarding comment lines and empty lines */
    nlines = 0;
    while (fgets(dummy, sizeof(dummy), file) != NULL) {
        if (dummy[0] != 0 && dummy[0] != '#') /* if this is not a comment line */
            nlines++; /* or an empty line, then count it */
    }

    new_obj->setOwnedObject(new WObject[nlines + 1]);

    rewind(file);
    i = 1;
    j = 0;
    mx = my = mz = 0x7fffffff;

    /* now read in the data */
    while (fgets(dummy, sizeof(dummy), file) != NULL) {

        if (dummy[0] != 0 && dummy[0] != '#') { /* if this looks like a legit line */

            if (sscanf(dummy, "%d,%d,%d - %d,%d,%d", &x1, &y1, &z1, &x2, &y2, &z2) < 6) {
                CTH_WARN("\n    Can't read at line: %d (%s)", i, name);
                if (i == 1) { /*  nothing read  */
                    CTH_WARN(" ... skipping file");
                    delete new_obj;
                    return NULL;
                }
            } else {
                if (j >= nlines) {
                    CTH_ERROR("Error reading object file %s", name);
                    delete new_obj;
                    return NULL;
                }

                if (x1 < mx)
                    mx = x1;
                if (x2 < mx)
                    mx = x2;
                if (y1 < my)
                    my = y1;
                if (y2 < my)
                    my = y2;
                if (z1 < mz)
                    mz = z1;
                if (z2 < mz)
                    mz = z2;

                new_obj->obj[j][0][0] = x1;
                new_obj->obj[j][0][1] = y1;
                new_obj->obj[j][0][2] = z1;

                new_obj->obj[j][1][0] = x2;
                new_obj->obj[j][1][1] = y2;
                new_obj->obj[j][1][2] = z2;
                j++;
            }
        }

        i++;
    }

    /* align the object up against the axes */
    for (i = 0; i < j; i++) {
        new_obj->obj[i][0][0] -= mx;
        new_obj->obj[i][0][1] -= my;
        new_obj->obj[i][0][2] -= mz;
        new_obj->obj[i][1][0] -= mx;
        new_obj->obj[i][1][1] -= my;
        new_obj->obj[i][1][2] -= mz;
    }

    /* terminate the line list with -1 coordinates */
    new_obj->obj[j][0][0] = new_obj->obj[j][0][1] = new_obj->obj[j][0][2] = new_obj->obj[j][1][0]
        = new_obj->obj[j][1][1] = new_obj->obj[j][1][2] = -1;

    return new_obj;
}
