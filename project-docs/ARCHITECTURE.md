# CthughaNix Architecture Overview

## Project Identity
**CthughaNix** is a Linux port of Cthugha, a real-time music visualization program originally created for DOS. It's often described as "an oscilloscope on acid" - a dynamic audio-visual synthesizer that responds to sound input in real-time.

- **Original Author**: Kevin "Zaph" Burfitt (DOS version)
- **Linux Port Maintainer**: Brandon Barker (as of version 1.5)
- **Language**: C++ (legacy 1999-era C++)
- **License**: LGPL (relicensed from original proprietary license)
- **Official Site**: http://cthughanix.sourceforge.net/

---

## High-Level Architecture

```
┌────────────────────────────────────────────────────────────────┐
│                    SOUND DEVICES (Input)                       │
│  ┌──────────┬────────────┬──────────┬─────────┬───────────┐   │
│  │ DSP Card │ Audio File │ CD-ROM   │ Network │ Random    │   │
│  └──────────┴────────────┴──────────┴─────────┴───────────┘   │
└────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌────────────────────────────────────────────────────────────────┐
│              SOUND ANALYSIS & PROCESSING                       │
│  • FFT (Fast Fourier Transform)                                │
│  • Amplitude detection (RMS)                                   │
│  • Attack level tracking                                       │
│  • Noise filtering                                             │
└────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌────────────────────────────────────────────────────────────────┐
│         VISUALIZATION ENGINE (Multiple Buffers)                │
│  ┌─────────────────────────────────────────────────────────┐  │
│  │ For each visualization buffer:                          │  │
│  │  1. Sound Processing (frequency-dependent effects)      │  │
│  │  2. Flashlight effect (brightness burst)                │  │
│  │  3. Flame algorithm (scrolling/movement)                │  │
│  │  4. Translation (image processing transforms)           │  │
│  │  5. Wave display (waveform visualization)               │  │
│  │  6. Palette smoothing (color transitions)               │  │
│  │  7. Buffer swap (active ↔ passive)                      │  │
│  └─────────────────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌────────────────────────────────────────────────────────────────┐
│           DISPLAY BACKENDS (Output)                            │
│  ┌──────────┬─────────┬──────────┬────────────────────────┐   │
│  │ X11      │ SVGA    │ OpenGL   │ ncurses (text mode)    │   │
│  │(Window)  │(Console)│(3D)      │                        │   │
│  └──────────┴─────────┴──────────┴────────────────────────┘   │
└────────────────────────────────────────────────────────────────┘
```

---

## Core Components

### 1. **Sound Input Layer** (`SoundDevice` hierarchy)

The project supports multiple sound sources through a factory pattern:

**Classes:**
- `SoundDevice` (base class)
- `SoundDeviceDSP` - Direct soundcard access (/dev/dsp)
- `SoundDeviceFile` - Read from audio files
- `SoundDeviceFork` - Fork child process for sound
- `SoundDeviceNet` - Receive sound over network
- `SoundDeviceRandom` - Generate random noise

**Key Interface:**
```cpp
class SoundDevice {
    char2 * data;              // stereo, signed, 8-bit sound
    void operator()();         // read next frame
    virtual int read() = 0;    // device-specific reading
};
```

**Data Flow:**
- Reads raw audio in various formats
- Converts to standardized stereo, signed 8-bit format
- Buffers data in 1024-sample frames
- Maintains consistent interface across all backends

---

### 2. **Sound Analysis** (`SoundAnalyze`)

Real-time audio analysis driving the visualization:

**Key Metrics:**
- `amplitude` - RMS (root mean square) volume level
- `amplitudeLeft` / `amplitudeRight` - Per-channel amplitude
- `attackLevel` - Cumulative sudden volume increases
- `intensity` - Smoothed, normalized amplitude (0.0-1.0)
- `noisy` - Boolean flag for silence detection
- `fire` - Triggered on attack events

**Processing:**
```cpp
void SoundAnalyze::operator()() {
    // Calculate RMS amplitude
    for(int i=1024; i!=0; i--) {
        al += *d * *d;  // accumulate squares
    }
    amplitude = sqrt(al / 1024);
    
    // Track attack (sudden volume increase)
    if(amplitude > lastamp)
        attackLevel += amplitude - lastamp;
}
```

**Used For:**
- Triggering scene changes (auto-changer)
- Controlling effect intensity
- Border effects (amplitude-based)
- Waveform display
- Palette transitions

---

### 3. **Visualization Engine** (`CthughaBuffer`)

The core rendering pipeline. Manages 1-3 framebuffers running effects in parallel.

**Structure:**
```cpp
class CthughaBuffer {
    CoreOption flame;          // Flame algorithm selector
    CoreOption palette;        // Color palette
    OptionPCX pcx;            // Background image
    TranslateOption translate; // Image processing
    CoreOption wave;          // Waveform display
    CoreOption object;        // 3D object
    CoreOption waveScale;     // Wave size
    CoreOption border;        // Border effect
    CoreOption soundProcess;  // Frequency effects
    CoreOption flashlight;    // Brightness burst
    
    unsigned char * activeBuffer;   // Currently rendering
    unsigned char * passiveBuffer;  // Currently displaying
};
```

**Frame Buffer Dimensions:**
- Internal buffer: 160×100 pixels (BUFF_WIDTH × BUFF_HEIGHT)
- 3-pixel border on top/bottom (boundary for flame algorithms)
- Actual usable area: 160×100
- Expanded for display (2× up to 320×200 or larger depending on backend)

**Rendering Pipeline (per frame):**

```cpp
void CthughaBuffer::run() {
    for(int j=0; j < nBuffers; j++) {
        current = buffers + j;
        
        // 1. Process sound (frequency analysis)
        current->soundProcess();
        
        // 2. Apply brightness burst effect
        current->flashlight();
        
        // 3. Set border effects
        switch(border) {
            case 0: memset(active_buffer±border, 0);
            case 1: memcpy(active_buffer±border, waveform);
            case 2: memset(active_buffer±border, amplitude);
            case 3: memset(active_buffer±border, 255);
        }
        
        // 4. Apply flame effect (core algorithm)
        current->flame();
        
        // 5. Apply image processing transforms
        current->translate();
        
        // 6. Apply waveform overlay
        current->wave();
        
        // 7. Smooth color transitions
        current->smoothPalette();
        
        // 8. Swap buffers for next frame
        swap(activeBuffer, passiveBuffer);
    }
}
```

**Key Features:**
- Multiple buffers can run independently
- Allows smooth transitions and layering
- Each buffer has independent settings (flame, palette, wave, etc.)
- Buffer swapping provides flicker-free rendering

---

### 4. **Flame Effects** (`flames.cc`)

"Flame" effects are the core algorithms that create the scrolling/movement patterns in the visualization.

**Available Flames:**
- `flame_clear` - Blank the buffer
- `flame_up{slow|subtle|fast}` - Upward scroll with intensity variants
- `flame_left{slow|subtle|fast}` - Leftward scroll
- `flame_right{slow|subtle|fast}` - Rightward scroll
- `flame_water` - Water ripple effect
- `flame_skyline` - Skyline/mountain pattern
- `flame_weird` - Algorithmic chaos
- `flame_zzz` - Sleeping pattern
- `flame_fade` - Gradual fade to black
- `flame_down` - Falling/downward effect
- `flame_general_*` - General-purpose transformations with variants

**Implementation Pattern:**
Each flame is a function pointer wrapped in `FlameEntry` (subclass of `CoreOptionEntry`). When selected, the flame function is called once per frame to manipulate the active framebuffer.

---

### 5. **Display Backends**

The system abstracts display hardware through a class hierarchy:

**Backend Architecture:**

```
DisplayDevice (abstract base)
├── DisplayDeviceX11 (X11/Xaw)
├── DisplayDeviceSVGA (Linux SVGA)
├── DisplayDeviceGL (OpenGL via GLUT)
└── (text console via ncurses)

CthughaDisplay (rendering/palette expansion)
├── CthughaDisplayX11
├── CthughaDisplaySVGA
└── CthughaDisplayGL
```

#### **X11 Backend** (`DisplayDeviceX11.cc`, `CthughaDisplayX11.cc`)
- Uses X11 Toolkit (Xaw)
- Supports XShm (shared memory) for fast blitting
- Supports color modes: 256-color (palette), 24-bit, 32-bit
- Screen sizes: 320×200 to 1280×1024
- Internal buffer: 160×100 to 600×512

#### **SVGA Backend** (`DisplayDeviceSvga.cc`, `CthughaDisplaySVGA.cc`)
- Uses SVGAlib for direct console graphics
- 256-color palette (VGA standard)
- Screen sizes: 320×200 to 1600×1200
- Must run with elevated privileges
- Direct framebuffer access for maximum performance

#### **OpenGL Backend** (`DisplayDeviceGL.cc`, `CthughaDisplayGL.cc`)
- Uses GLUT for windowing
- 3D visualization
- Features:
  - Perspective rendering
  - Lighting and materials
  - Paletted textures
  - Fog effects
  - Fly-through camera
  - Background rendering
  - Real-time light positioning

#### **NCurses Backend** (text mode)
- Terminal-based visualization
- ASCII art rendering
- Used as fallback when no graphics available

**Palette Expansion:**
- Internal buffer is 8-bit indexed color (0-255)
- Expanded to native color format:
  - X11: RGB/RGBA values
  - SVGA: Direct VGA color
  - OpenGL: Texture palette
- Expansion happens during `CthughaDisplay::nextFrame()`

---

### 6. **Configuration System** (`CoreOption` hierarchy)

Dynamic runtime configuration without recompilation:

**Class Hierarchy:**
```
Option (base)
├── OptionInt
│   ├── OptionTime (time values in 1/100 sec)
│   └── OptionOnOff (binary on/off)
└── [specialized options]

CoreOption (group option selector)
├── palette selector
├── flame selector
├── wave selector
├── border selector
├── sound process selector
└── etc.

CoreOptionEntry (individual option item)
├── PaletteEntry
├── FlameEntry
├── WaveEntry
├── PCXEntry (background image)
├── TranslateEntry (effect transform)
└── etc.
```

**Option Sources (priority order):**
1. Command-line arguments
2. `.cthugha.ini` (user home directory)
3. `/etc/cthugha.ini` (system-wide)
4. Compiled defaults

**Persistence:**
- Options automatically saved to `~/.cthugha.ini` on exit
- Restored on next run

---

### 7. **Interface System** (`Interface.cc`, `InterfaceList.cc`)

Text-based menu interface for runtime control:

**Key Classes:**
- `Interface` - Base interface/menu
- `InterfaceElement` - Menu item
- `InterfaceList` - List-based menu
- `InterfaceHelp`, `InterfaceCredits` - Info displays

**Interface Modes:**
- `main` - Main visualization control
- `server` - Sound server interface
- `[others]` - Various specialized interfaces

**Navigation:**
- Up/Down arrows: Move selection
- Enter: Select/activate
- Help: Display context help
- Status display: Real-time statistics

---

### 8. **Key Binding System** (`keymap.cc`, `keymap.h`)

Dynamic key rebinding system:

**Architecture:**
```
Keymap (singleton) manages keybindings
├── reads keymap file (default.keymap)
├── maps key codes to Action objects
└── Action subclasses define behaviors

Action (abstract)
├── class for each control action
└── virtual act() method
```

**Bindings File:**
- `/usr/local/share/cthugha/default.keymap`
- ASCII format: `key key-code action [parameters]`
- Read at startup, applied at runtime
- Per-backend customization (X11, SVGA, GL)

---

### 9. **Auto-Changer** (`AutoChanger`)

Automatically changes visualization parameters based on sound or time:

**Triggers:**
- Silence detection (quiet for N seconds → change)
- Loud sound burst (attack level → change)
- Timeout-based (minimum wait between changes)

**Options:**
- `changeQuiet` - Duration of silence to trigger change
- `changeMsgTime` - Display silence message duration
- `changeWaitMin` - Minimum time between changes
- `changeWaitRandom` - Extra random delay
- `changeFireLevel` - Attack level threshold
- `lock` - Enable/disable auto-changing
- `change_little` - Change only one parameter vs. all

---

### 10. **Sound Server** (`SoundServer`)

Optional network component (when compiled with `WITH_NETWORK=1`):

**Architecture:**
- Broadcasts sound frames to multiple network clients
- Allows remote visualization of same audio
- Port configuration:
  - `SRV_PORT` - Server broadcast port
  - `CLT_PORT` - Client request port
  - `REQ_PORT` - Request port

**Usage:**
- Multiple clients can connect
- Server manages connection list
- Maintains up to 255 clients
- Useful for exhibition setups or distributed systems

---

### 11. **CD-ROM Support** (`CDPlayer`)

Direct audio input from CD-ROM (when compiled with `WITH_CDROM=1`):

**Features:**
- Track selection
- Play/pause/stop control
- Forward/backward seeking
- Track information display
- Auto-eject on completion
- Loop/random playback modes

**Requirements:**
- CD-ROM physically connected to soundcard
- Audio cable from CD-ROM to audio input

---

## Main Programs

### **xcthugha** (X11 Version)
- Interactive X11 window display
- Full features available
- Requires X11 libraries
- Build: `xcthugha_SOURCES` in Makefile.am

### **cthugha** (SVGA/Console Version)
- Direct Linux console graphics
- Highest performance (direct framebuffer)
- Requires root privileges (for /dev/vga access)
- Build: `cthugha_SOURCES` in Makefile.am

### **glcthugha** (OpenGL Version)
- 3D visualization
- Requires OpenGL and GLUT
- Most visually impressive
- Build: `glcthugha_SOURCES` in Makefile.am

### **cthugha-server** (Sound Server)
- Audio stream distribution
- Minimal visualization (text/ncurses)
- Build: `cthugha_server_SOURCES` in Makefile.am

### **tabheader / tabinfo** (Utility Tools)
- `tabheader` - Parse/process effect tables
- `tabinfo` - Display table information

---

## Main Execution Flow

```
main(argc, argv)
│
├─ Random seed initialization
├─ seteuid(getuid()) - drop privileges
├─ get_pre_params() - handle --verbose, etc.
├─ cth_init() - platform-specific init (SVGA/X11/GL)
├─ get_params() - parse CLI args and config files
├─ title() - display splash screen
│
├─ SUBSYSTEM INITIALIZATION
├─ init_imath() - math library
├─ SoundDevice::newSD() - select sound source
├─ soundServer = new SoundServer()
├─ cdPlayer = new CDPlayer()
├─ init_mixer() - mixer settings
├─ CthughaBuffer::initAll() - visualization buffers
├─ newDisplayDevice() - select display backend
├─ newCthughaDisplay() - select display renderer
├─ CoreOption::changeToInitial() - set initial values
├─ Interface::set("main") - activate main menu
├─ Keymap::init() - load key bindings
├─ autoChanger = new AutoChanger()
├─ signal(SIGTSTP, sig_tty_stop) - handle ^Z
│
└─ displayDevice->mainLoop()  // MAIN LOOP
    │
    └─ for each frame:
        ├─ cthughaDisplay->nextFrame() - time management
        ├─ soundDevice->operator()() - read sound
        ├─ soundAnalyze() - analyze sound
        ├─ autoChanger->operator()() - maybe change settings
        ├─ soundServer->operator()() - broadcast sound
        ├─ CthughaBuffer::run() - render effects
        │   └─ for each buffer: flame→translate→wave→palette→swap
        ├─ cthughaDisplay->operator()() - send to display
        ├─ cdPlayer->operator()() - update CD player
        └─ handle ^Z pause if needed
```

---

## Data Flow Summary

```
AUDIO INPUT
    ↓
SoundDevice (converts to stereo 8-bit)
    ↓
SoundAnalyze (FFT, amplitude, attack)
    ↓
CthughaBuffer::run()
    ├─ SoundProcess (frequency effects)
    ├─ Flame (scrolling patterns)
    ├─ Translate (image processing)
    ├─ Wave (waveform display)
    └─ Palette transitions
    ↓
DisplayDevice-specific rendering
    ├─ Palette expansion (8-bit → device color)
    ├─ Scaling/stretching
    └─ Device blitting
    ↓
SCREEN OUTPUT
```

---

## Key Architectural Patterns

### 1. **Factory Pattern**
- `SoundDevice::newSD()` creates correct sound backend
- `DisplayDevice::newDisplayDevice()` creates correct display
- Compile-time configuration determines which classes are linked

### 2. **Strategy Pattern**
- Flame effects as function pointers in `FlameEntry`
- Wave effects as pluggable options
- Translate effects as pluggable transforms

### 3. **Observer Pattern (implicit)**
- Auto-changer observes sound level changes
- Interface observes key presses
- Display redraws on buffer updates

### 4. **Template Method**
- `CthughaDisplay` base class with virtual methods for palette expansion
- Subclasses override for format-specific optimization

### 5. **Singleton Pattern**
- `CthughaBuffer::current` - current visualization buffer
- `Interface::current` - current menu interface
- `Keymap::current` - current key bindings

---

## Compilation Architecture

The project uses **GNU Autoconf/Automake**:

**Key Build Configuration:**
```
configure
├─ --enable-svga (default: yes)
├─ --enable-xwin (default: yes)
├─ --enable-gl (default: yes)
├─ --enable-serv (default: yes)
├─ --with-dsp=/dev/dsp (default: /dev/dsp)
├─ --with-cdrom=/dev/cdrom (default: /dev/cdrom)
└─ [standard GNU options]
```

**Binary Targets:**
- `xcthugha` - X11 version (installed to bin/)
- `cthugha` - SVGA version (installed setuid root)
- `glcthugha` - OpenGL version
- `cthugha-server` - Sound server
- `tabheader`, `tabinfo` - Utility programs

**Resource Files:**
- Palettes: `$(pkglibdir)/pcx/*.pal`
- Maps/effects: `$(pkglibdir)/map/*.eff`
- Keymaps: `/usr/local/share/cthugha/default.keymap`

---

## Dependencies & Portability

**Required:**
- C++ compiler (GCC preferred, 1999-era code)
- GNU Autoconf/Automake
- Standard C library

**Optional (for features):**
- X11 libraries (for xcthugha)
- SVGAlib (for cthugha)
- OpenGL & GLUT (for glcthugha)
- ncurses (for text mode)
- CDROM kernel headers (for CDPlayer)
- Soundcard kernel headers (for DSP)

**Tested Platforms:**
- Linux x86/i386 (primary)
- Solaris (some support in build system)
- Other UNIX-like systems (possible with porting)

---

## Notable Design Decisions

1. **No C++ STL** - Uses raw pointers and manual memory management (1999 compatibility)
2. **Global state** - Extensive use of global variables and singletons
3. **Direct framebuffer access** - For performance (especially SVGA backend)
4. **Palette-based rendering** - 256-color mode as primary, others as extensions
5. **Multiple buffers** - Allows smooth transitions and overlays
6. **Configurable at runtime** - Not just at compile time
7. **Modular backends** - Can support new display types by subclassing

---

## Performance Characteristics

**Frame Rate:**
- Targets 25-60 FPS depending on backend
- Framerate capped and monitored in `CthughaDisplay::checkFPS()`

**Bottlenecks:**
- Sound reading (I/O bound)
- Display blitting (graphics backend dependent)
- Palette expansion (CPU bound, but fast)

**Optimizations:**
- Buffer doubling for flicker-free animation
- Palette caching (compile palette at startup)
- Direct framebuffer access in SVGA mode
- Selective redraw regions

---

## Extensibility Seams

Points where new features can be added:

1. **New Flame Effect** - Add function to `flames.cc`, register in `_flames[]`
2. **New Wave Display** - Add class derived from `CoreOptionEntry`, register in wave entries
3. **New Display Backend** - Subclass `DisplayDevice` and `CthughaDisplay`
4. **New Sound Source** - Subclass `SoundDevice`, register in factory
5. **New Palette/PCX** - Add `.pal` or `.pcx` file to resource directory
6. **New Interface** - Create `InterfaceXXX` subclass
7. **Network Protocol** - Modify `SoundServer` implementation
8. **New Color Mode** - Add palette expansion variant

