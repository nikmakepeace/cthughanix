/** @file
 * Unit coverage for WaveRuntime injected randomness.
 */

#include "ProcessServices.h"
#include "Wave.h"
#include "VideoFilterchain.h"

#include <assert.h>
#include <stdarg.h>
#include <vector>

int cth_log_enabled(int) { return 0; }
int cth_log(int, const char*, ...) { return 0; }
int cth_log_context(int, const char*, const char*, ...) { return 0; }
int cth_log_error(const char*, ...) { return 0; }
int cth_log_errno(int, const char*, ...) { return 0; }

void wave_dotHor(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_dotVert(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_lineHor(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_lineVert(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_spike(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_spikeH(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_buff9(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_buff10(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_buff11(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_buff14(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_buff15(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_buff16(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_pete0(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_pete1(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_pete2(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_fract1(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_fract2(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_test(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_aaron(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_wire1(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_wire1dot5(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_wire1dot55(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_wire1dot6(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_wire2(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_wire2dot1(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_lineHLdiff(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_spiral(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_pyro(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_warp(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_laser(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_corner(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_jump(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_sticks(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_grid(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }
void wave_none(CthughaBuffer&, const VideoFrameContext&, WaveRuntime&) { }

class SequenceRandomSource : public RandomSource {
    std::vector<int> values;
    std::vector<int> ranges;
    unsigned int index;

public:
    explicit SequenceRandomSource(const std::vector<int>& values_)
        : values(values_)
        , ranges()
        , index(0) { }

    virtual int uniformInt(int exclusiveMax) {
        ranges.push_back(exclusiveMax);
        assert(index < values.size());
        if (exclusiveMax <= 1)
            return 0;
        return values[index++] % exclusiveMax;
    }

    int requestedRange(unsigned int i) const {
        assert(i < ranges.size());
        return ranges[i];
    }
};

static void testWaveRuntimeRandomUsesInjectedSource() {
    std::vector<int> values;
    values.push_back(7);
    values.push_back(3);
    values.push_back(0x800000);
    SequenceRandomSource randomSource(values);
    WaveConfig config;
    WaveState state;
    WaveLookupTables lookupTables;
    WaveRuntime runtime(config, 0, state, lookupTables, randomSource, 0);

    assert(runtime.randomInt(10) == 7);
    assert(runtime.randomCenteredInt(4) == -1);
    double unit = runtime.randomUnit();
    assert(unit > 0.49);
    assert(unit < 0.51);

    assert(randomSource.requestedRange(0) == 10);
    assert(randomSource.requestedRange(1) == 9);
    assert(randomSource.requestedRange(2) == 0x1000000);
}

int main() {
    testWaveRuntimeRandomUsesInjectedSource();
    return 0;
}
