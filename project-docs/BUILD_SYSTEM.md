# CthughaNix - Build System & Extension Guide

## Build System Architecture

### **GNU Autoconf/Automake Pipeline**

```
source files (.cc, .h)
        ↓
Makefile.am (hand-written template)
        ↓
autoreconf (runs autoconf/automake)
        ↓
configure script + Makefile.in
        ↓
./configure (user runs with options)
        ↓
Makefile (platform-specific, feature-specific)
        ↓
make (compile & link)
        ↓
./cthugha, ./xcthugha, ./glcthugha, etc.
```

### **Key Files**

- `configure.in` - Autoconf template (processed → configure)
- `Makefile.am` - Automake template (processed → Makefile.in)
- `config.h.in` - Header template with feature flags
- `config.status` - Records configuration choices
- `Makefile.pre` - Bootstrap makefile

---

## Configuration Options

### **Display Backend Selection**

```bash
./configure --enable-xwin --enable-svga --enable-gl
./configure --disable-gl                    # Skip OpenGL build
./configure --enable-xwin --disable-svga    # X11 only
```

**Result:** Affects which binaries are compiled
- `xcthugha` - X11 backend (enable-xwin=yes)
- `cthugha` - SVGA backend (enable-svga=yes)
- `glcthugha` - OpenGL backend (enable-gl=yes)
- `cthugha-server` - Network server (enable-serv=yes)

### **Audio Device Selection**

```bash
./configure --with-dsp=/dev/dsp          # Default
./configure --without-dsp                 # Disable OSS
./configure --with-dsp=/dev/audio         # ALSA device
./configure --with-cdrom=/dev/cdrom       # CD-ROM device
./configure --without-cdrom                # Disable CDROM
```

**Affects:** Which `SoundDevice` subclasses are available

### **System Paths**

```bash
./configure --prefix=/usr/local
./configure --bindir=/usr/bin
./configure --libdir=/usr/lib/cthugha      # Resource directory
./configure --datadir=/usr/share/cthugha
```

### **Inspection Commands**

```bash
./configure --help                         # Show all options
grep "TARGETS" config.status               # Which binaries built
grep "WITH_" config.h                      # Feature flags
```

---

## Feature Compilation Flags

Set in `config.h` (generated from `config.h.in`):

### **Display System**
```c
#define HAVE_X11 1              // X11 available
#define HAVE_SVGALIB 1          // SVGAlib available
#define HAVE_GL 1               // OpenGL/GLUT available
#define HAVE_NCURSES 1          // NCurses available
```

### **Audio System**
```c
#define WITH_DSP 1              // /dev/dsp support
#define WITH_MIXER 1            // Mixer device support
#define WITH_CDROM 1            // CD-ROM support
#define WITH_NETWORK 1          // Sound server (UDP)
```

### **Header Availability**
```c
#define HAVE_LINUX_CDROM_H      // Linux CDROM headers
#define HAVE_LINUX_SOUNDCARD_H  // Linux sound headers
#define HAVE_SYS_SOUNDCARD_H    // System sound headers
#define STDC_HEADERS            // Standard C library
#define HAVE_SYS_TIME_H         // System time headers
```

### **Data Type Properties**
```c
#define WORDS_BIGENDIAN 0       // Endianness (0=little, 1=big)
#define SIZEOF_INT 4            // Integer size in bytes
```

---

## Adding a New Feature

### **Scenario 1: Adding a New Flame Effect**

**Step 1: Implement the effect**

File: `src/flames.cc`

```cpp
// Add function at module level
void flame_myeffect() {
    // Manipulate active_buffer
    // active_buffer points to 160×100 pixel array
    // Each pixel is a palette index (0-255)
    
    for(int i=0; i < BUFF_SIZE; i++) {
        if(active_buffer[i] > 0)
            active_buffer[i]--;  // Fade out
    }
}
```

**Step 2: Register the effect**

```cpp
// In flames.cc, add to _flames[] array
CoreOptionEntry * _flames[] = {
    // ... existing effects ...
    new FlameEntry(flame_myeffect, "MyFx", "My new effect"),
};
int _nFlames = sizeof(_flames)/sizeof(CoreOptionEntry*);  // Update count
```

**Step 3: Rebuild**

```bash
make clean
make
./xcthugha  # Test it
```

**Step 4: Use in application**

- Accessible via menu: `flame` → `MyFx`
- Selected by auto-changer
- Persists in config file

---

### **Scenario 2: Adding a New Display Backend**

**Step 1: Create backend header**

File: `src/DisplayDeviceNEW.h`

```cpp
#include "DisplayDevice.h"

class DisplayDeviceNEW : public DisplayDevice {
public:
    DisplayDeviceNEW();
    virtual ~DisplayDeviceNEW();
    
    virtual void mainLoop();
    virtual int setGlobalPalette();
    virtual unsigned char * preDraw();
    virtual void copyBox(int,int,int,int,int,int);
    virtual void postDraw();
};

class CthughaDisplayNEW : public CthughaDisplay {
private:
    virtual void expandPalette(int);
public:
    CthughaDisplayNEW();
    virtual void operator()();
};

extern void newCthughaDisplay();  // Factory
extern void newDisplayDevice();    // Factory
```

**Step 2: Implement backend**

File: `src/DisplayDeviceNEW.cc`

```cpp
#include "DisplayDeviceNEW.h"
#include "display.h"

DisplayDeviceNEW::DisplayDeviceNEW() : DisplayDevice() {
    // Initialize graphics hardware
}

void DisplayDeviceNEW::mainLoop() {
    // Main event loop
    do {
        run(1);              // from initExitDisp.cc
        // Handle events specific to your backend
    } while(!cthugha_close);
}

void newDisplayDevice() {
    displayDevice = new DisplayDeviceNEW();
}

void newCthughaDisplay() {
    cthughaDisplay = new CthughaDisplayNEW();
}
```

**Step 3: Add compilation unit to Makefile.am**

```makefile
# New section in src/Makefile.am
ifdef NEW_DISPLAY
DISPSRC += DisplayDeviceNEW.cc CthughaDisplayNEW.cc
endif

# Or add conditionally:
if BUILD_NEW
  bin_PROGRAMS += glcthugha-new
  glcthugha_new_SOURCES = $(DISPSRC) DisplayDeviceNEW.cc
  glcthugha_new_LDADD = $(LIBS)
endif
```

**Step 4: Update configure.in**

```m4
AC_ARG_ENABLE(new,
  [--enable-new  Build NEW display backend],
  [case "${enableval}" in
    yes) new=1 ;;
    no) new=0 ;;
    *) AC_MSG_ERROR(bad value) ;;
   esac],
  [new=0])

AM_CONDITIONAL(BUILD_NEW, test x$new = x1)
```

**Step 5: Rebuild configure**

```bash
autoreconf -i
./configure --enable-new
make
```

---

### **Scenario 3: Adding a New Sound Source**

**Step 1: Create device class**

File: `src/SoundDeviceNEW.h`

```cpp
#ifndef __SOUND_DEVICE_NEW_H
#define __SOUND_DEVICE_NEW_H

#include "SoundDevice.h"

class SoundDeviceNEW : public SoundDevice {
private:
    int deviceHandle;
    // Device-specific members
    
public:
    SoundDeviceNEW();
    virtual ~SoundDeviceNEW();
    
    virtual int read();      // Read next 1024 samples
    virtual void update();   // Called when sample rate changes
};

#endif
```

**Step 2: Implement device**

File: `src/SoundDeviceNEW.cc`

```cpp
#include "SoundDeviceNEW.h"

SoundDeviceNEW::SoundDeviceNEW() : SoundDevice() {
    // Open your audio source
}

int SoundDeviceNEW::read() {
    // Read samples into tmpData
    // Convert from native format using convert()
    // Return number of samples read
    
    int nRead = // read from device
    convert(data, tmpData, nRead);
    return nRead;
}

void SoundDeviceNEW::update() {
    // Called when sample rate option changes
    // Reconfigure device if necessary
}
```

**Step 3: Register in factory**

File: `src/SoundDevice.cc`

```cpp
void SoundDevice::newSD() {
    switch(soundDeviceNr) {
        case SDN_DSPIn:
            soundDevice = new SoundDeviceDSP();
            break;
        // ... existing ...
        case SDN_MyDevice:
            soundDevice = new SoundDeviceNEW();
            break;
    }
}
```

Update `SoundDevice.h`:
```cpp
enum SoundDeviceNr {
    SDN_DSPIn, 
    SDN_Net, 
    SDN_Random, 
    SDN_File, 
    SDN_MyDevice,     // Add this
    SDN_Max
};
```

**Step 4: Add options**

In your options code or via command-line:
```bash
./xcthugha --snd-method 5  # or whatever your device index is
```

---

### **Scenario 4: Adding a New Configuration Option**

**Option 1: Integer Option**

File: `src/myfeature.cc`

```cpp
#include "Option.h"

// Declare at module level
OptionInt myFeatureLevel("feature-level", 128, 256, 0);

// Register with interface
extern CoreOptionEntry * myFeatureEntries[] = {
    new CoreOptionEntry("level-low", "Low", 1),
    new CoreOptionEntry("level-med", "Medium", 1),
    new CoreOptionEntry("level-high", "High", 1),
};
```

**Option 2: On/Off Option**

```cpp
OptionOnOff enableMyFeature("enable-feature", 1);  // 1=on, 0=off
```

**Option 3: Time Option (in 1/100 second)**

```cpp
OptionTime myTimeout("timeout", 500);  // 5.00 seconds displayed as "5.00 sec"
```

**Option 4: Multi-choice**

```cpp
class MyOption : public CoreOption {
public:
    MyOption(int buffer) : CoreOption(buffer, "my-option", myEntries) {}
};
```

**Usage:**
- Automatically appears in menu
- Saved to config file
- Accessible via `--my-option value` CLI
- Persists across sessions

---

## Debugging & Development

### **Compile-Time Debugging**

```bash
CXXFLAGS="-g -O0 -DDEBUG" ./configure
make clean
make
```

### **Profiling Code**

File `src/initExitDisp.cc` has built-in profiling:

```cpp
#define PROF  // Uncomment to enable

// Measures timing of each subsystem:
// - Display frame setup
// - Sound reading
// - Sound analysis
// - Auto-changer
// - Sound server
// - Buffer rendering
// - Display update
// - CD player
```

Enable and rebuild:
```bash
cd src
vi initExitDisp.cc  # Uncomment #define PROF
cd ..
make clean
make
./xcthugha  # Prints timing every 25 frames
```

### **Verbose Logging**

```bash
./xcthugha -v 2            # Verbose level 2
./xcthugha --verbose 3     # Very verbose
./xcthugha -v 5 --snd-method 1 2>&1 | tee debug.log
```

Prints initialization messages and errors to stderr.

### **Instrumentation Points**

Add debug output with:
```cpp
#include "cthugha.h"

printfv(1, "Debug message\n");        // Verbose level
printfe("Error message\n");            // Error (with errno)
printfee("Error with errno\n");        // Error (detailed)
```

---

## Performance Optimization

### **Profiling Candidates**

From built-in profiling (enable PROF):

1. **Sound reading** - I/O bound
2. **Palette expansion** - CPU bound
3. **Flame effect** - CPU bound
4. **Display blitting** - Graphics bound

### **Optimization Techniques**

**1. Palette Expansion Optimization**

Current approach processes pixels in 4-byte chunks. Already well-optimized for 1999.

Modern improvements:
- SIMD instructions (SSE/AVX)
- Multi-threading
- Pre-computed lookup tables

**2. Flame Effect Optimization**

Currently single-threaded pixel manipulation. Could benefit from:
- Memory-mapped operations
- DMA transfers
- GPU shader implementation

**3. Display Backend Optimization**

- X11: Already uses XShm (shared memory)
- SVGA: Already uses direct framebuffer
- OpenGL: Already GPU-accelerated
- NCurses: Inherently limited by terminal I/O

### **Memory Access Patterns**

The 160×100 internal buffer with 3-pixel border:
```
Memory layout: [3-row border] [100 rows] [3-row border]
Address range: +0 to +16000 (BUFF_SIZE)
Linear access favors sequential writes in flame effects
```

Optimize by:
- Processing entire rows at once
- Using pointer arithmetic instead of 2D indexing
- Pre-calculating row offsets

---

## Cross-Platform Porting

### **Adding macOS Support**

1. **Detection in configure.in:**
```m4
case "$host" in
  *-apple-darwin*)
    CXXFLAGS="$CXXFLAGS -fPIC"
    AC_DEFINE(MACOS, 1)
    ;;
esac
```

2. **Sound on macOS:**
   - Use Core Audio instead of OSS
   - Alternative: libsndfile + ALSA libraries
   - Implement `SoundDeviceCoreAudio`

3. **Display on macOS:**
   - Quartz rendering (X11 emulation)
   - Modern: Use Cocoa with OpenGL
   - Keep OpenGL backend (most portable)

### **Adding Windows Support**

1. **Sound:**
   - Use DirectSound or WASAPI
   - Implement `SoundDeviceWAV` variant
   - Or: MinGW + ALSA emulation

2. **Display:**
   - Use Win32 API for window
   - Keep OpenGL backend
   - Or: Use SDL abstraction layer

### **Adding BSD Support**

1. **Sound:**
   - Similar to Linux OSS
   - Check `/dev/dsp` compatibility
   - May need platform-specific headers

2. **Display:**
   - X11 already portable
   - SVGAlib (limited)
   - OpenGL fully portable

---

## Version Management

### **Build Version**
- Defined in `configure.in`: `AC_INIT(CthughaNix, 1.5)`
- Generates `config.h` with `VERSION` macro
- Displayed at startup: `title()`

### **Feature Versioning**

Track feature availability in `cthugha.h`:
```cpp
#define CTHUGHA_MAJOR 1
#define CTHUGHA_MINOR 5
#define CTHUGHA_MICRO 0

#define FEATURE_OPENGL    (CTHUGHA_MINOR >= 5)
#define FEATURE_NETWORK   (WITH_NETWORK == 1)
```

### **Backward Compatibility**

Config file format (`.cthugha.ini`):
- Version number at top
- Parser checks version before loading
- Graceful degradation for old configs

---

## Testing Strategy

### **Unit Testing**
- Math functions: `imath.h`
- Option parsing: `Option.h`
- Palette operations: `display.cc`

### **Integration Testing**
- Full startup sequence
- Audio device initialization
- Display backend switching
- Menu navigation

### **Visual Testing**
- Play test audio files
- Verify smooth animation
- Check palette colors
- Test all flame effects

### **Performance Testing**
- Frame rate measurement
- CPU utilization profiling
- Memory usage tracking

---

## Maintenance Guidelines

### **Code Style**
- 4-space indentation
- Class names: CamelCase
- Function names: snake_case
- Constants: UPPER_CASE
- Comments in English

### **Documentation Requirements**
- Header comments for all classes
- Function documentation for complex logic
- Inline comments for non-obvious algorithms

### **Release Checklist**
1. Update version in `configure.in`
2. Update `NEWS` file with changes
3. Test all backends (SVGA, X11, GL)
4. Test all sound methods (DSP, File, Random)
5. Test configuration save/restore
6. Build on multiple platforms if possible

---

## Future Modernization Opportunities

1. **C++ Modernization**
   - Replace raw pointers with smart pointers
   - Use STL containers instead of manual allocation
   - Modern error handling (exceptions)
   - C++11 or later standard

2. **Threading**
   - Separate audio thread
   - Separate render thread
   - Lock-free data structures

3. **GPU Acceleration**
   - CUDA/OpenCL for effects
   - Compute shaders
   - Realtime ray tracing

4. **Audio Library**
   - Migrate from OSS to PulseAudio/ALSA
   - Support more formats (FLAC, Vorbis)
   - Better error recovery

5. **Cross-Platform Abstraction**
   - SDL2 for display
   - Minimal native code
   - Web version (WebGL, WebAudio API)

