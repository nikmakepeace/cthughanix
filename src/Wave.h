#ifndef __WAVE_H
#define __WAVE_H

class CthughaBuffer;
class VisualFrameContext;

// A 3D object is a list of lines, each line is two 3-space points.
// The list is terminated by the final line having coordinates that are all -1.
typedef int WObject[2][3];

class WaveConfig {
public:
    int waveScale;
    int table;
    WObject* object;
    int bufferWidth;
    int bufferHeight;

    WaveConfig();
    WaveConfig(int waveScale_, int table_, WObject* object_,
        int bufferWidth_, int bufferHeight_);

    int sameAs(const WaveConfig& other) const;
};

class WaveState {
    void* value;
    void (*destroyValue)(void*);

    template <class T>
    static void destroy(void* value_) {
        delete static_cast<T*>(value_);
    }

public:
    WaveState()
        : value(0)
        , destroyValue(0) { }
    ~WaveState() {
        clear();
    }

    void clear() {
        if (destroyValue != 0)
            (*destroyValue)(value);
        value = 0;
        destroyValue = 0;
    }

    template <class T>
    T& get() {
        if (value == 0) {
            value = new T;
            destroyValue = &WaveState::destroy<T>;
        }
        return *static_cast<T*>(value);
    }
};

class WaveRuntime {
    int needsConfigurationValue;
    WaveState& stateValue;

public:
    int waveScale;
    int table;
    WObject* object;

    WaveRuntime(const WaveConfig& config, int needsConfiguration_, WaveState& state_);

    int needsConfiguration() const;

    template <class T>
    T& state() {
        return stateValue.get<T>();
    }
};

class Wave {
public:
    typedef void (*Function)(CthughaBuffer& buffer, WaveRuntime& runtime);
    typedef int (*CanRunFunction)(const WaveConfig& config);

private:
    Function functionValue;
    CanRunFunction canRunFunctionValue;
    const char* nameValue;
    const char* descriptionValue;
    WaveConfig configValue;
    WaveState stateValue;
    int configuredValue;
    int needsConfigurationValue;

public:
    Wave(Function function, const char* name, const char* description,
        CanRunFunction canRunFunction = 0);

    const char* name() const;
    const char* description() const;
    int canRun(const WaveConfig& config) const;
    void configure(const WaveConfig& config);
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context);
};

extern Wave waveCatalog[];
extern const int nWaveCatalogEntries;

Wave* waveByIndex(int index);

#endif
