#include "EffectControl.h"
#include "Wave.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

EffectChoice* read_object(FILE* file, const char* name, const char* dir, const char* total_name);
WObject* waveObjectEntryObject(EffectChoice* entry);
int waveObjectEntryOwnsObject(const EffectChoice* entry);

int cth_log_enabled(int) {
    return 0;
}

int cth_log(int, const char*, ...) {
    return 0;
}

int cth_log_context(int, const char*, const char*, ...) {
    return 0;
}

int cth_log_error(const char*, ...) {
    return 0;
}

int cth_log_errno(int, const char*, ...) {
    return 0;
}

static FILE* objectFixture(const char* text) {
    FILE* file = tmpfile();
    assert(file != 0);
    assert(fputs(text, file) >= 0);
    rewind(file);
    return file;
}

static void assertTerminator(const WObject& line) {
    for (int endpoint = 0; endpoint < 2; endpoint++)
        for (int coord = 0; coord < 3; coord++)
            assert(line[endpoint][coord] == -1);
}

static void testReadObjectNormalizesCoordinatesAndTerminates() {
    FILE* file = objectFixture(
        "# ignored comment\n"
        "10,20,30 - 11,22,33\n"
        "malformed after first valid line\n"
        "13,24,31 - 10,20,30\n");

    EffectChoice* entry = read_object(file, "fixture", "", "");
    fclose(file);

    assert(entry != 0);
    assert(strcmp(entry->Name(), "fixture") == 0);
    assert(waveObjectEntryOwnsObject(entry));

    WObject* object = waveObjectEntryObject(entry);
    assert(object != 0);

    assert(object[0][0][0] == 0);
    assert(object[0][0][1] == 0);
    assert(object[0][0][2] == 0);
    assert(object[0][1][0] == 1);
    assert(object[0][1][1] == 2);
    assert(object[0][1][2] == 3);

    assert(object[1][0][0] == 3);
    assert(object[1][0][1] == 4);
    assert(object[1][0][2] == 1);
    assert(object[1][1][0] == 0);
    assert(object[1][1][1] == 0);
    assert(object[1][1][2] == 0);

    assertTerminator(object[2]);

    delete entry;
}

static void testReadObjectRejectsMalformedFirstLine() {
    FILE* file = objectFixture("not an object line\n");

    EffectChoice* entry = read_object(file, "broken", "", "");
    fclose(file);

    assert(entry == 0);
}

int main() {
    testReadObjectNormalizesCoordinatesAndTerminates();
    testReadObjectRejectsMalformedFirstLine();
    return 0;
}
