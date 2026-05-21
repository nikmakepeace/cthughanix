# CthughaNix - Module Breakdown & File Organization

## Source Directory Structure

```
src/
├── Core Framework
│   ├── Option.h / OptionInt.cc
│   ├── CoreOption.h / CoreOption.cc
│   ├── CoreOptionEntry.cc
│   └── IniFiles.cc              # Config file parsing
│
├── Sound Input & Analysis
│   ├── SoundDevice.h / SoundDevice.cc
│   ├── SoundDeviceDSP.cc        # /dev/dsp audio card
│   ├── SoundDeviceFile.cc       # MP3, WAV from files
│   ├── SoundDeviceFork.cc       # Child process pipe
│   ├── SoundDeviceNet.cc        # Network socket
│   ├── SoundDeviceRandom.cc     # Noise generator
│   ├── SoundAnalyze.h / SoundAnalyze.cc
│   ├── SoundServer.h / SoundServer.cc
│   ├── Mixer.cc                 # Audio mixer control
│   ├── Sound.h / Sound.cc        # General sound utilities
│   └── sound_tables.cc          # Lookup tables
│
├── Audio Output & CD
│   ├── CDPlayer.h / CDPlayer.cc
│   └── network.h / network.cc
│
├── Visualization Rendering
│   ├── CthughaBuffer.h / CthughaBuffer.cc
│   ├── CthughaDisplay.h / CthughaDisplay.cc
│   ├── CthughaDisplayX11.cc
│   ├── CthughaDisplaySVGA.cc
│   ├── CthughaDisplayGL.cc
│   ├── cth_buffer.h             # Frame buffer dimensions
│   ├── flames.h / flames.cc     # Flame/scroll algorithms
│   ├── waves.h / waves.cc       # Wave display options
│   ├── translate.h / translate.cc
│   ├── SoundProcess.h / SoundProcess.cc
│   ├── Flashlight.cc            # Brightness effects
│
├── Display Backends
│   ├── DisplayDevice.h / DisplayDevice.cc
│   ├── DisplayDeviceX11.cc
│   ├── DisplayDeviceX11-Panel.cc
│   ├── DisplayDeviceSVGA.cc (cthugha main)
│   ├── DisplayDeviceGL.cc (glcthugha main)
│   ├── disp-ncurses.cc          # Terminal rendering
│   ├── display.h / display.cc   # Palette management
│   ├── GL_display.cc
│   ├── GL_Light.cc
│   ├── GL_Background.cc
│   ├── GL_Fly.cc
│   ├── GL_keys.cc
│   ├── GL_options.cc
│   └── nonGL_stubs.cc
│
├── User Interface & Control
│   ├── Interface.h / Interface.cc
│   ├── InterfaceList.cc
│   ├── InterfaceHelp.cc
│   ├── InterfaceCredits.cc
│   ├── keymap.h / keymap.cc
│   ├── AutoChanger.h / AutoChanger.cc
│   ├── default.keymap           # Key binding file
│   ├── default.keymap.str       # Compiled keymap strings
│   └── information.h / info_title_usage.cc
│
├── Input Handling
│   ├── keys.h / keys.cc         # Generic key handling
│   ├── xwin_keys.cc             # X11-specific keys
│   ├── nonx_keys.cc             # Non-X11 keys
│   ├── GL_keys.cc               # OpenGL-specific keys
│   ├── joystick.h / joystick.cc
│   └── [backend]_options.cc     # Backend-specific options
│
├── Graphics & Palettes
│   ├── pcx.h / pcx.cc           # PCX image format
│   ├── palettes.cc              # Palette loading
│   └── translate.cc             # Effect transforms
│
├── Utilities & Support
│   ├── imath.h / imath.cc       # Integer math library
│   ├── misc.cc                  # Miscellaneous functions
│   ├── vroot.h                  # Virtual root (X11 background)
│   ├── disp-sys.h               # Display system abstractions
│   ├── xcthugha.h               # X11-specific definitions
│   ├── glcthugha.h              # OpenGL definitions
│   ├── cthugha.h                # Main header
│   ├── information.h            # Constants & strings
│   └── tab_header.h
│
├── Platform-Specific Options
│   ├── xwin_options.cc          # X11 options
│   ├── svga_options.cc          # SVGA options
│   ├── GL_options.cc            # OpenGL options
│   ├── nonx_options.cc          # Non-X11 options
│   └── serv_options.cc          # Server options
│
├── Main Programs
│   ├── initExitDisp.cc          # xcthugha/cthugha/glcthugha main()
│   ├── serv_main.cc             # cthugha-server main()
│   ├── serv_display.cc          # Server display stubs
│   ├── serv_stubs.cc            # Server stubs
│   ├── nonGL_stubs.cc           # Non-GL stubs
│   ├── tabheader.cc             # Tab tool
│   └── tabinfo.cc               # Tab info tool
│
└── Build Files
    ├── Makefile.am              # Automake config
    ├── Makefile.in              # Generated
    └── Makefile                 # Final makefile
```

---

## Header Files Grouped by Purpose

### **Core Configuration**
- `cthugha.h` - Master header with feature flags
- `Option.h` - Option system interface
- `CoreOption.h` - Core option framework
- `information.h` - Compile-time constants

### **Audio Processing**
- `SoundDevice.h` - Sound input interface
- `SoundAnalyze.h` - Sound analysis interface
- `SoundServer.h` - Network sound distribution
- `CDPlayer.h` - CD-ROM audio
- `Sound.h` - General sound utilities

### **Visualization**
- `CthughaBuffer.h` - Main rendering pipeline
- `CthughaDisplay.h` - Display renderer interface
- `display.h` - Palette and display utilities
- `cth_buffer.h` - Buffer dimension constants
- `flames.h`, `waves.h` - Visual effects

### **Display Hardware**
- `DisplayDevice.h` - Display interface
- `disp-sys.h` - Display system abstraction
- `xcthugha.h` - X11-specific
- `glcthugha.h` - OpenGL-specific

### **User Interaction**
- `Interface.h` - Menu system
- `keymap.h` - Key binding system
- `keys.h` - Key handling
- `joystick.h` - Joystick input

### **Utilities**
- `imath.h` - Integer math
- `network.h` - Network utilities
- `pcx.h` - PCX image format
- `translate.h` - Image transforms
- `vroot.h` - X11 virtual root

---

## Key Data Structures

### **Sound Frame**
```cpp
char2 * soundDevice->data;  // [1024] stereo 8-bit samples
// Format: left_sample, right_sample, left_sample, right_sample, ...
```

### **Frame Buffer**
```cpp
class CthughaBuffer {
    unsigned char * activeBuffer;    // [BUFF_SIZE] current rendering target
    unsigned char * passiveBuffer;   // [BUFF_SIZE] currently displayed
    // BUFF_SIZE = 160 * 100 = 16,000 bytes (+ 6 rows border)
    // Each byte = palette index 0-255
};
```

### **Palette**
```cpp
typedef unsigned char Palette[256][3];  // 256 colors × 3 bytes (RGB)
extern Palette bitmap_colors0[256];    // Compiled/cached palette
extern unsigned long bitmap_colors0[256];  // Device-native color format
```

### **Display Coordinates**
```cpp
typedef struct {
    int x, y;
} xy;

extern xy disp_size;          // Drawing area size
extern xy draw_size;          // Buffer size before expansion
extern xy screenSizes[];      // Predefined screen sizes
extern xy bufferSizes[];      // Corresponding buffer sizes
```

### **Sound Analysis Result**
```cpp
class SoundAnalyze {
    int amplitude;            // RMS volume 0-256
    int amplitudeLeft;        // Left channel RMS
    int amplitudeRight;       // Right channel RMS
    int attackLevel;          // Accumulated sudden increases
    int fire;                 // Boolean: attack occurred this frame
    double intensity;         // Normalized 0.0-1.0
    double speed;             // Rate of change
};
```

### **Options Structure**
```cpp
class CoreOption {
    CoreOptionEntry * entries[];     // Array of choices
    int current;                      // Currently selected index
    virtual const char * text();      // Display as string
    virtual int operator()();         // Execute/apply option
};
```

---

## Major Algorithms

### **1. Flame Effects - Upward Scroll (Example)**
```cpp
void flame_upslow() {
    // Copy pixel columns upward with wrapping
    // Create scrolling effect by shifting framebuffer contents up
    // Refill bottom with new content (drawn from sound)
    
    for(int y=0; y < BUFF_HEIGHT-1; y++) {
        for(int x=0; x < BUFF_WIDTH; x++) {
            active_buffer[y * BUFF_WIDTH + x] = 
                active_buffer[(y+1) * BUFF_WIDTH + x];
        }
    }
    // Bottom row gets new data
}
```

### **2. Sound Analysis - RMS Amplitude**
```cpp
void SoundAnalyze::operator()() {
    int al = 0, ar = 0;
    
    // Sum of squares for each channel
    for(int i=1024; i!=0; i--) {
        char sample = *data++;
        al += sample * sample;
        sample = *data++;
        ar += sample * sample;
    }
    
    // RMS = sqrt(mean of squares)
    amplitude = sqrt(al/1024) + sqrt(ar/1024) / 2;
    
    // Attack detection: sudden increase triggers flag
    if(amplitude > lastamplitude + threshold) {
        fire = 1;
        attackLevel += amplitude - lastamplitude;
    }
}
```

### **3. Palette Expansion - X11 Example**
```cpp
void CthughaDisplayX11::expandPalette(int narrow) {
    // Convert 8-bit palette indices to native color format
    // Palette-to-RGB mapping cached in bitmap_colors[]
    
    unsigned long * scrn = (unsigned long *)expandedBuffer;
    unsigned long * buff = (unsigned long *)internalBuffer;
    
    for(int i = pixels; i != 0; i--) {
        unsigned long b = *buff++;  // 4 palette indices packed
        unsigned long a;
        
        a  = bitmap_colors0[b & 0xff];        b >>= 8;
        a |= bitmap_colors1[b & 0xff];        b >>= 8;
        a |= bitmap_colors2[b & 0xff];        b >>= 8;
        a |= bitmap_colors3[b];
        
        *scrn++ = a;  // Write expanded color
    }
}
```

### **4. Frame Buffer Rendering Pipeline**
```cpp
void CthughaBuffer::run() {
    // For each visualization buffer in parallel:
    
    soundProcess();        // Apply frequency-dependent effects
    flashlight();          // Add brightness based on attack
    flame();              // Apply scroll/movement algorithm
    translate();          // Apply image processing transforms
    wave();               // Overlay waveform display
    smoothPalette();      // Transition colors smoothly
    
    // Swap active and passive buffers
    swap(activeBuffer, passiveBuffer);
}
```

### **5. Sound Source Abstraction - Factory Pattern**
```cpp
void SoundDevice::newSD() {
    // Select appropriate sound device based on options
    
    switch(soundDeviceNr) {
        case SDN_DSPIn:
            soundDevice = new SoundDeviceDSP();
            break;
        case SDN_File:
            soundDevice = new SoundDeviceFile(filename);
            break;
        case SDN_Net:
            soundDevice = new SoundDeviceNet(port);
            break;
        case SDN_Random:
            soundDevice = new SoundDeviceRandom();
            break;
    }
    
    soundDevice->change();  // Configure for current sample rate
}
```

---

## Compilation Units (Object Files)

### **Shared (All Builds)**
```
Option.o OptionInt.o CoreOption.o CoreOptionEntry.o
SoundDevice.o SoundDeviceDSP.o SoundDeviceFile.o
SoundDeviceFork.o SoundDeviceNet.o SoundDeviceRandom.o
SoundAnalyze.o SoundServer.o CDPlayer.o Mixer.o
sound.o network.o IniFiles.o misc.o
imath.o Interface.o InterfaceList.o InterfaceCredits.o
InterfaceHelp.o keymap.o AutoChanger.o info_title_usage.o
disp-ncurses.o sound_tables.o
```

### **X11 Build (xcthugha)**
```
[shared] +
DisplayDeviceX11.o CthughaDisplayX11.o DisplayDeviceX11-Panel.o
display.o palettes.o pcx.o flames.o waves.o translate.o
SoundProcess.o CthughaBuffer.o CthughaDisplay.o
Flashlight.o joystick.o xwin_keys.o xwin_options.o
keys.o nonGL_stubs.o
```

### **SVGA Build (cthugha)**
```
[shared] +
DisplayDeviceSvga.o CthughaDisplaySVGA.o
display.o palettes.o pcx.o flames.o waves.o translate.o
SoundProcess.o CthughaBuffer.o CthughaDisplay.o
Flashlight.o joystick.o nonx_keys.o svga_options.o
keys.o nonGL_stubs.o
```

### **OpenGL Build (glcthugha)**
```
[shared] +
DisplayDeviceGL.o CthughaDisplayGL.o
GL_display.o GL_Light.o GL_Background.o GL_Fly.o
GL_keys.o GL_options.o
display.o palettes.o pcx.o flames.o waves.o translate.o
SoundProcess.o CthughaBuffer.o CthughaDisplay.o
Flashlight.o joystick.o GL_keys.o nonGL_stubs.o
```

### **Server Build (cthugha-server)**
```
[shared] -
DisplayDevice*.o CthughaDisplay*.o display.o palettes.o
pcx.o flames.o waves.o translate.o Flashlight.o
[xwin stuff] +
serv_main.o serv_display.o serv_stubs.o serv_options.o
nonGL_stubs.o
```

---

## Configuration Dependency Graph

```
OPTIONS (stored in ~/.cthugha.ini or command line)
    ├─ flame               (changes CthughaBuffer::flame)
    ├─ palette             (changes CthughaBuffer::palette)
    ├─ wave                (changes CthughaBuffer::wave)
    ├─ border              (changes CthughaBuffer::border)
    ├─ sound-process       (changes CthughaBuffer::soundProcess)
    ├─ translate           (changes CthughaBuffer::translate)
    ├─ flashlight          (changes CthughaBuffer::flashlight)
    ├─ sound device        (SoundDevice::newSD())
    ├─ display backend     (DisplayDevice::newDisplayDevice())
    ├─ auto-change settings (AutoChanger behavior)
    └─ [many others]

EFFECT of option changes:
    CoreOption::change() 
        → selection index incremented/decremented
        → new CoreOptionEntry::operator()() called
        → visual effect updated next frame
        → value saved to ~/.cthugha.ini at exit
```

---

## Memory Layout

### **Stack (Each Frame)**
- Sound buffer: 1024 samples × 2 channels × 1 byte = ~2 KB
- Local variables in run() and effect functions
- Frame timing calculations

### **Heap**
- **Persistent:**
  - Frame buffers (2×): 160×100×2 = ~32 KB
  - Expanded display buffers: 320×200×4 = ~250 KB (X11/GL)
  - Palettes: 256×3 = 768 bytes + compiled versions: 256×4 = 1 KB
  - PCX images: variable
  
- **Per-frame allocation (should be minimal):**
  - Effect processing temporary buffers
  - Network packet buffers (server mode)

### **Typical Total Memory: ~5-10 MB**
- Small by modern standards
- Reasonable for 1999 hardware

---

## Threading Model

**Important: Single-threaded, non-blocking design**

- No explicit threading (predates Pthreads popularity)
- Sound reading can fork child process (SoundDeviceFork)
- IPC via pipes for forked sound processing
- Display backends handle blocking:
  - SVGA: Synchronous I/O
  - X11: Event loop
  - OpenGL: GLUT main loop

**Key Design Implications:**
- Frame rate limited by slowest operation
- No deadlock possible (single thread)
- CPU-bound optimization opportunities
- Could benefit from threading modernization

---

## Resource Files

### **Palettes (`pcx/` directory)**
- `.pal` files in Fractint format
- 256 RGB triplets (768 bytes each)
- Loaded on demand
- Cached in memory once selected

### **Effects (`map/` directory)**
- Translation table effects
- Border definitions
- Wave scale settings
- Loaded dynamically

### **Key Bindings (`default.keymap`)**
- Text format: `key keycode action [params]`
- Parsed at startup
- Per-backend variants
- Runtime rebindable

### **Configuration (`~/.cthugha.ini`)**
- INI format
- Current settings
- Restored on startup
- Auto-saved on exit

---

## Signal Handling

- **SIGTSTP (^Z):** Pause visualization, suspend process
- **SIGCONT:** Resume visualization after suspend
- **SIGTERM:** Graceful shutdown
- **SIGHUP:** Reload configuration (if implemented)

Special handling in `initExitDisp.cc`:
```cpp
signal(SIGTSTP, sig_tty_stop);  // Set handler
signal(SIGCONT, sig_tty_cont);  // Set resume handler
```

---

## Device Driver Dependencies

### **Sound Input**
- `/dev/dsp` - Digital Signal Processing device (OSS/ALSA)
- `/dev/cdrom` - CD-ROM device (for direct audio)
- `/dev/mixer` - Mixer control

### **Display Output**
- X11: X server process
- SVGA: `/dev/vga` - VGA registers
- OpenGL: X11 + OpenGL driver

### **Terminal**
- `/dev/tty` - Terminal control
- stdin/stdout/stderr - Text I/O

---

## Platform Adaptation Points

**Architecture-Specific (i386 assumed):**
- Byte order detection in `cthugha.h`
- Endian conversion for network
- Integer size assumptions

**Platform-Specific Code:**
- `SoundDeviceDSP.cc` - Linux /dev/dsp (OSS)
- `SoundDeviceFile.cc` - File I/O (portable)
- `DisplayDeviceSvga.cc` - SVGAlib (Linux)
- `DisplayDeviceX11.cc` - X11 (portable Unix)

**Portable Abstractions:**
- `imath.h` - Integer operations
- `display.h` - Palette management
- `interface.h` - Menu system
- `Sound.h` - Audio interface
