#include "Wave.h"

void wave_dotHor(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_dotVert(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_lineHor(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_lineVert(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_spike(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_spikeH(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_buff9(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_buff10(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_buff11(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_buff14(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_buff15(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_buff16(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_pete0(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_pete1(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_pete2(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_fract1(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_fract2(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_test(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_aaron(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_wire1(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_wire1dot5(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_wire1dot55(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_wire1dot6(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_wire2(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_wire2dot1(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_lineHLdiff(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_spiral(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_pyro(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_warp(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_laser(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_corner(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_jump(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_sticks(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_grid(CthughaBuffer& buffer, WaveRuntime& runtime);
void wave_none(CthughaBuffer& buffer, WaveRuntime& runtime);

static int waveCanAlwaysRun(const WaveConfig& config) {
    (void)config;

    return 1;
}

static int waveCanRunWithObject(const WaveConfig& config) {
    return config.object != 0;
}

static int waveCanRunAaron(const WaveConfig& config) {
    return config.bufferHeight > 128 && config.bufferWidth >= 256;
}

WaveConfig::WaveConfig()
    : waveScale(0)
    , table(0)
    , object(0)
    , bufferWidth(0)
    , bufferHeight(0) { }

WaveConfig::WaveConfig(int waveScale_, int table_, WObject* object_,
    int bufferWidth_, int bufferHeight_)
    : waveScale(waveScale_)
    , table(table_)
    , object(object_)
    , bufferWidth(bufferWidth_)
    , bufferHeight(bufferHeight_) { }

int WaveConfig::sameAs(const WaveConfig& other) const {
    return waveScale == other.waveScale
        && table == other.table
        && object == other.object
        && bufferWidth == other.bufferWidth
        && bufferHeight == other.bufferHeight;
}

WaveRuntime::WaveRuntime(const WaveConfig& config, int needsConfiguration_, WaveState& state_)
    : needsConfigurationValue(needsConfiguration_)
    , stateValue(state_)
    , waveScale(config.waveScale)
    , table(config.table)
    , object(config.object) { }

int WaveRuntime::needsConfiguration() const {
    return needsConfigurationValue;
}

Wave::Wave(Function function, const char* name, const char* description,
    CanRunFunction canRunFunction)
    : functionValue(function)
    , canRunFunctionValue(canRunFunction != 0 ? canRunFunction : waveCanAlwaysRun)
    , nameValue(name)
    , descriptionValue(description)
    , configuredValue(0)
    , needsConfigurationValue(1) { }

const char* Wave::name() const {
    return nameValue;
}

const char* Wave::description() const {
    return descriptionValue;
}

int Wave::canRun(const WaveConfig& config) const {
    return (*canRunFunctionValue)(config);
}

void Wave::configure(const WaveConfig& config) {
    needsConfigurationValue = !configuredValue || !configValue.sameAs(config);
    configValue = config;
    configuredValue = 1;
}

void Wave::execute(CthughaBuffer& buffer, const VisualFrameContext& context) {
    (void)context;

    if (functionValue != 0) {
        WaveRuntime runtime(configValue, needsConfigurationValue, stateValue);
        needsConfigurationValue = 0;
        (*functionValue)(buffer, runtime);
    }
}

Wave waveCatalog[] = {
    Wave(wave_dotHor, "DotHor", "Dots Horizontal"),
    Wave(wave_dotVert, "DotVert", "Dots Vertical"),
    Wave(wave_lineHor, "LineHor", "Lines Horizontal"),
    Wave(wave_lineVert, "LineVert", "Lines Vertical"),
    Wave(wave_spike, "Spike", "Spikes"),
    Wave(wave_spikeH, "SpikeH", "Spikes Hollow"),
    Wave(wave_buff9, "Walking", "Walking"),
    Wave(wave_buff10, "Falling", "Falling"),
    Wave(wave_buff11, "Lissa", "Lissa"),
    Wave(wave_buff14, "LineX", "Line X"),
    Wave(wave_buff15, "Light1", "Lightning 1"),
    Wave(wave_buff16, "Light2", "Lightning 2"),
    Wave(wave_pete0, "Pete0", "FireFlies"),
    Wave(wave_pete1, "Pete1", "Pete"),
    Wave(wave_pete2, "Pete2", "Dot VS sine"),
    Wave(wave_fract1, "Fract1", "Zippy 1"),
    Wave(wave_fract2, "Fract2", "Zippy 2"),
    Wave(wave_test, "Test", "Test"),
    Wave(wave_aaron, "Aaron", "Rings of Fire", waveCanRunAaron),
    Wave(wave_wire1, "Wire1", "Wire frame 1", waveCanRunWithObject),
    Wave(wave_wire1dot5, "Wire1dot5", "Wire frame 1.5", waveCanRunWithObject),
    Wave(wave_wire1dot55, "Wire1dot55", "Wire frame 1.55", waveCanRunWithObject),
    Wave(wave_wire1dot6, "Wire1dot6", "Wire frame 1.6", waveCanRunWithObject),
    Wave(wave_wire2, "Wire2", "Wire frame 2", waveCanRunWithObject),
    Wave(wave_wire2dot1, "Wire2dot1", "Wire frame 2.1", waveCanRunWithObject),
    Wave(wave_lineHLdiff, "LineHLDiff", "Difference Hor."),
    Wave(wave_spiral, "Spiral", "Spirograph"),
    Wave(wave_pyro, "Pyro", "Fire works"),
    Wave(wave_warp, "Warp", "Space warp"),
    Wave(wave_laser, "Laser", "Laser"),
    Wave(wave_corner, "Corner", "Corner"),
    Wave(wave_jump, "Jump", "Jumping points"),
    Wave(wave_sticks, "Sticks", "Random sticks"),
    Wave(wave_grid, "Grid", "Diagnostic grid"),
    Wave(wave_none, "None", "No wave drawing"),
};

const int nWaveCatalogEntries = sizeof(waveCatalog) / sizeof(Wave);

Wave* waveByIndex(int index) {
    if (index < 0 || index >= nWaveCatalogEntries)
        return 0;

    return waveCatalog + index;
}
