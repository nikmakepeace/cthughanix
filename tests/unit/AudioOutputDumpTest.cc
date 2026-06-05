/** @file
 * Unit coverage for injected audio output dump writing.
 */

#include "Audio.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string>
#include <unistd.h>

int cth_log_enabled(int) { return 0; }
int cth_log(int, const char*, ...) { return 0; }
int cth_log_context(int, const char*, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }
int cth_log_errno(int, const char*, ...) { return 0; }
double getTime() { return 0.0; }
const char* audioSampleFormatText(int) { return "signed-16"; }

static PcmFormat mono16Format() {
    PcmFormat format;
    format.sampleRate = 1000;
    format.channels = 1;
    format.sampleFormat = SF_s16_le;
    return format;
}

static unsigned int readLe32(const unsigned char* p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8)
        | ((unsigned int)p[2] << 16) | ((unsigned int)p[3] << 24);
}

static void testInjectedDumpWritesSubmittedPcmAsWav() {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "/tmp/cthughanix-output-dump-%ld.wav",
        (long)getpid());
    unlink(path);

    AudioOutputDump dump(path);
    AudioOutputConfig config;
    AudioNullOutput output(config, &dump);
    DecodedAudioHistory history(16, mono16Format(), 8);
    AudioOutputStream stream(history);
    char scratch[8];
    const char pcm[4] = { 1, 0, 2, 0 };

    assert(history.appendDecodedPcm(pcm, 2) == 2);
    output.configureTiming(1000, mono16Format().bytesPerSample(), 2);
    assert(output.service(stream, scratch, 2, 0) == 1);
    dump.close();

    FILE* file = fopen(path, "rb");
    assert(file != NULL);
    unsigned char bytes[48];
    assert(fread(bytes, 1, sizeof(bytes), file) == sizeof(bytes));
    fclose(file);
    unlink(path);

    assert(bytes[0] == 'R');
    assert(bytes[1] == 'I');
    assert(bytes[2] == 'F');
    assert(bytes[3] == 'F');
    assert(bytes[8] == 'W');
    assert(bytes[9] == 'A');
    assert(bytes[10] == 'V');
    assert(bytes[11] == 'E');
    assert(readLe32(bytes + 40) == 4);
    assert(bytes[44] == 1);
    assert(bytes[45] == 0);
    assert(bytes[46] == 2);
    assert(bytes[47] == 0);
}

int main() {
    testInjectedDumpWritesSubmittedPcmAsWav();
    return 0;
}
