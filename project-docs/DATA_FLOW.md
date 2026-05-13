# CthughaNix - Detailed Data Flow & Execution Model

## Complete Frame Processing Pipeline

### **Phase 1: Frame Initialization**
```
FRAME START (initExitDisp.cc::run())
    ↓
cthughaDisplay->nextFrame()
    - Calculate deltaT (time since last frame)
    - Update 'now' (current time in animation)
    - Check FPS (enforce maximum frame rate)
    - Update zoom/scale values
    - Return control
```

### **Phase 2: Audio Input**
```
(*soundDevice)()  [SoundDevice.cc]
    ↓
    ├─ Call read() (platform-specific)
    │   ├─ SoundDeviceDSP: ioctl(/dev/dsp)
    │   ├─ SoundDeviceFile: fread() from audio file
    │   ├─ SoundDeviceNet: recvfrom() socket
    │   ├─ SoundDeviceRandom: generate random samples
    │   └─ SoundDeviceFork: read from forked child pipe
    │
    ├─ Conversion to standard format
    │   ├─ Input: various formats (8/16-bit, mono/stereo, BE/LE)
    │   ├─ Process: convert() method handles endianness
    │   └─ Output: stereo 8-bit signed (-128 to +127)
    │
    ├─ Data buffering
    │   ├─ rawSize = bytesPerSample × size (1024 samples)
    │   ├─ Keep last 1024 samples in buffer
    │   ├─ Append new samples from read()
    │   └─ Discard oldest samples (sliding window)
    │
    └─ Result: soundDevice->data[1024] filled
```

### **Phase 3: Sound Analysis**
```
soundAnalyze()  [SoundAnalyze.cc]
    ↓
    ├─ RMS Amplitude Calculation
    │   ├─ Load soundDevice->data[1024]
    │   ├─ Sum: al += (left_sample)² for all samples
    │   ├─ Sum: ar += (right_sample)² for all samples
    │   ├─ RMS: sqrt(al/1024) and sqrt(ar/1024)
    │   ├─ Combine: (left_RMS + right_RMS) / 2
    │   └─ Storage: soundAnalyze.amplitude (0-256)
    │
    ├─ Silence Detection
    │   ├─ If amplitude < minnoise threshold → noisy = 0
    │   ├─ Else → noisy = 1
    │   └─ Used by AutoChanger for change triggers
    │
    ├─ Attack Detection
    │   ├─ Compare: amplitude vs lastAmplitude - 9
    │   ├─ If sudden increase > threshold
    │   │   ├─ fire = 1 (attack flag set)
    │   │   ├─ attackLevel += increase amount
    │   │   └─ Used for flashlight & effect trigger
    │   └─ Else → fire = 0
    │
    ├─ Intensity Calculation
    │   ├─ Smooth amplitude: intensity = 0.9×intensity + 0.1×normalized_amplitude
    │   ├─ Range: 0.0 to 1.0
    │   └─ Used for parameter scaling
    │
    └─ Result: soundAnalyze metrics updated for next frame
```

### **Phase 4: Automatic Parameter Changing**
```
(*autoChanger)()  [AutoChanger.cc]
    ↓
    ├─ Check silence timeout
    │   ├─ silence_duration = now - lastSoundTime
    │   ├─ If silence_duration > changeQuiet
    │   │   ├─ Trigger display change
    │   │   ├─ Display silence message
    │   │   └─ Reset timer
    │   └─ Else → no change
    │
    ├─ Check attack-based change
    │   ├─ If soundAnalyze.attackLevel > changeFireLevel
    │   │   ├─ Maybe trigger change
    │   │   └─ Depends on randomization
    │   └─ Else → no change
    │
    ├─ Enforce minimum wait time
    │   ├─ timeSinceLastChange = now - lastChangeTime
    │   ├─ If timeSinceLastChange < (changeWaitMin + random(changeWaitRandom))
    │   │   └─ Don't change (too soon)
    │   └─ Else → allow change
    │
    ├─ Change parameters if triggered
    │   ├─ CoreOption::changeRandom() for selected effects
    │   ├─ Optionally only one parameter if change_little=1
    │   └─ Save lastChangeTime = now
    │
    └─ Result: Visualization parameters may be updated
```

### **Phase 5: Network Sound Broadcasting**
```
(*soundServer)()  [SoundServer.cc]
    ↓
    ├─ If WITH_NETWORK not enabled
    │   └─ No-op (empty class)
    │
    ├─ Check client connections
    │   ├─ Receive connection requests on REQ_PORT
    │   ├─ Validate client address
    │   ├─ Add to clientAddrs[] array (up to 255)
    │   └─ Track nClients
    │
    ├─ Broadcast current audio frame
    │   ├─ Send soundDevice->data[1024] via UDP
    │   ├─ One packet per connected client
    │   ├─ Destination: clientAddrs[i]:CLT_PORT
    │   ├─ Continue even if some sends fail
    │   └─ No flow control (fire-and-forget)
    │
    ├─ Handle disconnections
    │   ├─ Timeout inactive clients (if implemented)
    │   ├─ Remove from clientAddrs[]
    │   └─ Decrement nClients
    │
    └─ Result: Network clients receive live audio stream
```

### **Phase 6: Visualization Rendering Pipeline**
```
CthughaBuffer::run()  [CthughaBuffer.cc]
    ↓
    for(int j=0; j < nBuffers; j++) {  // Usually 1-3 buffers
        current = buffers + j;
        
        ┌─ Buffer State at Start
        │ - activeBuffer: currently being rendered to
        │ - passiveBuffer: currently displayed on screen
        │ - Both are 160×100 with 3-pixel border (3-row boundary)
        │ - Flames scroll within this area
        │
        ├─ STEP 1: Sound Processing
        │   current->soundProcess()  [SoundProcess.cc]
        │   
        │   Frequency Analysis:
        │   ├─ Perform FFT on sound frame
        │   ├─ Compute frequency bins (bass, mids, treble)
        │   ├─ Map frequencies to visual effects
        │   │   ├─ Bass → bottom rows (low freq = low spatial freq)
        │   │   ├─ Mids → middle rows
        │   │   └─ Treble → top rows (high freq = high spatial freq)
        │   ├─ Modify activeBuffer pixels based on frequencies
        │   │   └─ Increase intensity in frequency bands
        │   └─ Result: Audio-responsive graphics
        │
        ├─ STEP 2: Flashlight Effect
        │   current->flashlight()  [Flashlight.cc]
        │   
        │   Brightness Burst:
        │   ├─ If soundAnalyze.fire == 1 (attack event)
        │   │   ├─ Increase brightness across buffer
        │   │   ├─ Add: activeBuffer[i] += brightness_amount
        │   │   ├─ Cap at 255 (palette size)
        │   │   └─ Creates flash effect on percussion hits
        │   └─ Else → no modification
        │
        ├─ STEP 3: Border Effect
        │   
        │   Fill 3-pixel border (top and bottom):
        │   
        │   switch(CthughaBuffer::border) {
        │   
        │   case 0: Clear border
        │       memset(activeBuffer + BUFF_SIZE, 0, 3×BUFF_WIDTH)
        │       memset(activeBuffer - 3×BUFF_WIDTH, 0, 3×BUFF_WIDTH)
        │       // Blank bottom and top borders
        │   
        │   case 1: Waveform display
        │       memcpy(activeBuffer ± border, soundDevice->data, BUFF_WIDTH)
        │       // Copy current waveform to border
        │   
        │   case 2: Amplitude display
        │       memset(activeBuffer ± border, soundAnalyze.amplitude)
        │       // Fill border with current amplitude value
        │   
        │   case 3: White border
        │       memset(activeBuffer ± border, 255)
        │       // Fill border with brightest color
        │   }
        │
        ├─ STEP 4: Flame Effect (Core Algorithm)
        │   current->flame()  [flames.cc]
        │   
        │   Flame = scrolling/movement pattern:
        │   
        │   Examples:
        │   
        │   flame_upslow:
        │   ├─ Copy rows upward (y→y-1)
        │   ├─ Bottom row filled with new data
        │   └─ Effect: Smoke/particles moving up
        │   
        │   flame_water:
        │   ├─ Compute wave pattern
        │   ├─ Apply ripple distortion
        │   └─ Effect: Water surface visualization
        │   
        │   flame_weird:
        │   ├─ Complex mathematical transformation
        │   ├─ Fractal-like or chaotic effect
        │   └─ Effect: Psychedelic patterns
        │   
        │   Result: activeBuffer transformed with scrolling
        │
        ├─ STEP 5: Translation (Image Processing)
        │   current->translate()  [translate.cc]
        │   
        │   Effect Transforms:
        │   ├─ Color remapping (palette substitution)
        │   ├─ Distortion (pixels displaced)
        │   ├─ Mirroring (horizontal/vertical flip)
        │   ├─ Rotation (pixel grid rotation)
        │   ├─ Zoom (scaling up/down)
        │   │   └─ Only if use_translates enabled
        │   └─ Result: Distorted/transformed image
        │
        ├─ STEP 6: Wave Display
        │   current->wave()  [waves.cc]
        │   
        │   Waveform Overlay:
        │   ├─ If use_objects enabled:
        │   │   ├─ Draw 3D object into buffer
        │   │   └─ Object coordinates: WObject[2][3]
        │   ├─ Else:
        │   │   ├─ Draw waveform: soundDevice->data[1024]
        │   │   ├─ Map samples to pixels
        │   │   ├─ Apply scaling (waveScale option)
        │   │   └─ Apply drawing table (border style)
        │   └─ Result: Waveform rendered on top
        │
        ├─ STEP 7: Palette Smoothing
        │   current->smoothPalette()  [CthughaBuffer.cc]
        │   
        │   Color Transition:
        │   ├─ Current palette: CthughaBuffer::palette
        │   ├─ Target palette: CthughaBuffer::currentPalette
        │   ├─ Interpolate: palette = 0.95×palette + 0.05×target
        │   ├─ Updated: bitmap_colors[] cache
        │   └─ Effect: Smooth color fade between palettes
        │
        ├─ STEP 8: Buffer Swap
        │   
        │   prepare for next frame:
        │   ├─ swap(activeBuffer, passiveBuffer)
        │   │   • activeBuffer was rendering target
        │   │   • passiveBuffer was display source
        │   │   • After swap: reversed roles
        │   ├─ Next frame will render to previous display buffer
        │   └─ Result: Flicker-free double-buffering
        │
        └─ Buffer completed
    }
    
    Set current = buffers[nCurrent]  // Select active buffer for display
```

### **Phase 7: Display Device Update**
```
(*cthughaDisplay)()  [CthughaDisplay subclass]
    ↓
    ├─ Palette Expansion
    │   
    │   CthughaDisplayX11::expandPalette(int narrow)
    │   
    │   ├─ Input: 8-bit indexed buffer (0-255 palette indices)
    │   ├─ Convert each byte to native color format:
    │   │
    │   │   For each 4 palette indices packed in 32 bits:
    │   │   ├─ Fetch bitmap_colors0[index0] → 32-bit color
    │   │   ├─ Fetch bitmap_colors1[index1] → 32-bit color
    │   │   ├─ Fetch bitmap_colors2[index2] → 32-bit color
        │   ├─ Fetch bitmap_colors3[index3] → 32-bit color
        │   └─ Combine into output: out = c0 | c1 | c2 | c3
        │
        │   Backend-specific:
        │   ├─ X11: Arrange as RGB/ARGB based on display depth
        │   ├─ SVGA: Direct VGA palette index (no expansion)
        │   ├─ OpenGL: Texture palette lookup
        │   └─ NCurses: Map to 16 terminal colors
        │
        └─ Result: Display-ready pixel format
    │
    ├─ Scale/Zoom
    │   
    │   Apply zoom factor (if not 1.0):
    │   ├─ zoomBuffer = scale(expandedBuffer, zoomFactor)
    │   ├─ Interpolation: bilinear or nearest-neighbor
    │   └─ Result: Scaled image ready for display
    │
    ├─ Mirroring (if enabled)
    │   
    │   Optional horizontal/vertical flip:
    │   ├─ flipHorizontal() - reverse pixel order in rows
    │   ├─ flipVertical() - reverse row order
    │   └─ Result: Mirrored image
    │
    └─ Send to Graphics Hardware
        
        Backend-specific blitting:
        
        X11:
        ├─ XPutImage() or XShmPutImage()
        ├─ Window ID from X11 display
        └─ Swaps immediately to screen (or buffers in XShm)
        
        SVGA:
        ├─ gl_putbox() or direct framebuffer write
        ├─ /dev/vga I/O port access
        └─ Immediate screen update
        
        OpenGL:
        ├─ glTexSubImage2D() update texture
        ├─ glDrawArrays() render triangle with texture
        ├─ glSwapBuffers() (double-buffered)
        └─ 3D transformations applied
        
        Result: Image displayed on screen
```

### **Phase 8: Peripheral Updates**
```
(*cdPlayer)()  [CDPlayer.cc]
    ↓
    ├─ If WITH_CDROM not enabled
    │   └─ No-op
    │
    ├─ Poll CD status
    │   ├─ CDROM_DRIVE_STATUS ioctl
    │   └─ Update: track, position, status
    │
    ├─ Auto-play logic
    │   ├─ If cd_loop enabled and track finished
    │   │   ├─ Play next track (or wrap to first)
    │   │   └─ Call play(nextTrack)
    │   ├─ If cd_randomplay enabled
    │   │   ├─ Randomly select next track
    │   │   └─ Call play(randomTrack)
    │   └─ Else → continue current track
    │
    ├─ Auto-eject logic
    │   ├─ If cd_eject_on_end enabled and disc finished
    │   │   ├─ Call eject()
    │   │   └─ Open CD tray
    │   └─ Else → keep disc in drive
    │
    └─ Result: CD playback continues or changes
```

### **Phase 9: Pause/Resume Handling**
```
Final check in run():
    
    if(cthugha_pause) {  // Set by SIGTSTP signal (^Z)
        ├─ cthugha_pause = 0 (clear flag)
        ├─ exit_sound()  // Stop audio reading
        ├─ raise(SIGTSTP)  // Trigger STOP signal
        └─ Process suspends (user can resume with fg)
        
        When fg resumes:
        ├─ sig_tty_cont() handler called
        ├─ init_sound()  // Restart audio
        ├─ displayDevice->preDraw()  // Resume rendering
        └─ Continue mainLoop()
```

---

## Sound Format Conversions

### **SoundDevice Conversion Pipeline**

```
Raw Audio Input (device-dependent)
    ↓
    ├─ Format Detection
    │   ├─ Sample size: 8-bit or 16-bit
    │   ├─ Channels: mono or stereo
    │   ├─ Byte order: little-endian or big-endian
    │   ├─ Signedness: signed or unsigned
    │   └─ Sample rate: 8kHz to 48kHz
    │
    ├─ Temporary Buffer (tmpData)
    │   ├─ Size: rawSize = bytesPerSample × 1024 samples
    │   ├─ Example: 2 channels × 2 bytes = 4 bytes/sample
    │   └─ Example: 4 × 1024 = 4096 bytes buffer
    │
    ├─ convert() Method
    │   
    │   void convert(char2 * dst, void * src, int n) {
    │   
    │   Handles format conversions:
    │   
    │   ├─ Sample unpacking (8 or 16 bits)
    │   ├─ Byte order conversion (little/big endian)
    │   ├─ Channel selection (stereo → if mono, duplicate)
    │   ├─ Amplitude normalization (fit to 8-bit range)
    │   ├─ Signed adjustment (if unsigned input, shift)
    │   └─ Output: dst[n] = {left, right, left, right, ...}
    │
    └─ Output Buffer (soundDevice->data)
        ├─ Type: char2 * (array of stereo pairs)
        ├─ Length: 1024 samples (fixed)
        ├─ Format: stereo, signed 8-bit (-128 to +127)
        ├─ Used by: all visualization & analysis
        └─ Cached: soundDevice->dataProc[1024]
```

### **Format Enumeration**
```cpp
enum soundFormat_t {
    SF_u8,         // Unsigned 8-bit (0-255)
    SF_s8,         // Signed 8-bit (-128 to +127)
    SF_u16_le,     // Unsigned 16-bit little-endian
    SF_s16_le,     // Signed 16-bit little-endian
    SF_u16_be,     // Unsigned 16-bit big-endian
    SF_s16_be      // Signed 16-bit big-endian
};
```

---

## Memory Layout Diagram

### **Heap Allocation**

```
Sound Buffer
├─ soundDevice->data[1024]        2KB  stereo 8-bit samples
└─ soundDevice->tmpData           4-8KB  (format-dependent temporary)

Frame Buffers
├─ activeBuffer[16600]            16.6KB  (160×100 + 6-row border)
├─ passiveBuffer[16600]           16.6KB  (double-buffered)
├─ expandedBuffer[51200]          51.2KB  (expanded to 320×200×1 byte)
└─ [OpenGL]                       up to 256KB for packed RGBA

Palettes
├─ currentPalette[256][3]         768B   RGB triplets
├─ bitmap_colors0[256]            1KB    compiled palette (4 bytes/entry)
├─ bitmap_colors1[256]            1KB
├─ bitmap_colors2[256]            1KB
└─ bitmap_colors3[256]            1KB

PCX Images
├─ PCXEntry::data[width×height]   variable  (typically < 100KB each)
└─ pcx_palettes[]                 variable

Options
├─ CoreOptionEntry[][]            variable  (hundreds of entries)
└─ Configuration strings          variable

Network (if enabled)
├─ clientAddrs[255]               ~4KB     (client socket addresses)
└─ send buffers                   ~2KB     (UDP packets)

Total Typical Usage: 5-15 MB
```

### **Stack Allocation (Per Frame)**

```
run() stack frame
├─ local variables (timing)       ~100 bytes
├─ profiling array (PROF mode)    ~80 bytes
└─ called function stacks         (varies)

SoundAnalyze() stack
├─ amplitude accumulation         ~10 bytes
└─ temporary values               ~50 bytes

Flame effect stack
├─ loop counters                  ~10 bytes
└─ temporary pixels               ~100 bytes

Display stack
├─ color calculations             ~50 bytes
└─ coordinate adjustments         ~50 bytes

Total Per Frame: < 500 bytes
```

---

## Timing & Synchronization

### **Frame Rate Management**

```
cthughaDisplay->nextFrame()
    ├─ Query system timer: getTime()
    ├─ Calculate deltaT = now - lastFrameTime
    ├─ Check FPS constraint
    │   ├─ minFrameTime = 1.0 / maxFramesPerSecond
    │   ├─ If deltaT < minFrameTime
    │   │   ├─ sleep(minFrameTime - deltaT)
    │   │   └─ Wait until time budget exhausted
    │   └─ Else → proceed immediately
    ├─ Update now = system time
    └─ Return control to main loop
```

### **Audio Synchronization**

Not explicit in code - relies on:
- Audio device blocking reads (waits for buffer)
- Display vsync (if available, X11 may sync to monitor)
- CPU frame budget timing

**Note:** Original DOS version had tighter sync; Linux version is opportunistic.

---

## State Transitions

### **Initialization State Machine**

```
START
 ├─ Random seed
 ├─ Drop privileges (seteuid)
 ├─ Parse arguments (get_pre_params)
 ├─ Platform init (cth_init) → SVGA/X11/GL init
 ├─ Parse config (get_params) → read .ini, apply CLI
 ├─ Display title
 │
 └─ SUBSYSTEMS (in order)
     ├─ Math init
     ├─ Sound device: SDN_DSPIn / File / Net / Random
     ├─ Sound server (if network)
     ├─ CD player (if CDROM)
     ├─ Mixer device
     ├─ Visualization buffers (1-3)
     ├─ Display backend (X11/SVGA/GL)
     ├─ Load palettes & resources
     ├─ User interface
     ├─ Key bindings
     └─ Auto-changer
            ↓
      MAIN LOOP (displayDevice->mainLoop())
            ↓
      SHUTDOWN
     ├─ Save options to .cthugha.ini
     ├─ Close sound device
     ├─ Close CD player
     ├─ Free buffers
     ├─ Close display
     └─ Exit
```

---

## Option Value Ranges

### **Timing Options** (in 1/100 second)

```cpp
changeQuiet         50-1000     (0.5 - 10.0 seconds) [default: 150]
changeWaitMin       50-1000     (0.5 - 10.0 seconds) [default: 500]
changeWaitRandom    0-1000      (0 - 10.0 seconds)   [default: 1000]
changeMsgTime       0-500       (0 - 5.0 seconds)    [default: 500]
```

### **Analysis Options**

```cpp
sound_minnoise      0-256       (silence threshold)   [default: 5]
changeFireLevel     0-1000      (attack trigger)      [default: 100]
```

### **Display Options**

```cpp
maxFramesPerSecond  5-120       (frame rate cap)      [default: 60]
zoom                0.1-4.0     (display magnification)[default: 1.0]
```

### **Buffer Options**

```cpp
nCurrent            0-2         (active buffer selector) [default: 0]
maxNBuffers         1-3         (parallel buffers)       [default: 1]
```

---

## Error Recovery

### **Sound Device Failures**

```
SoundDevice::read() returns 0 or error

Handling:
├─ Maintain last valid frame in buffer
├─ soundDevice->data[] unchanged
├─ Slide window: keep old + discard corrupted
├─ Visualization continues with stale audio
└─ Allows recovery without crashing
```

### **Display Device Failures**

```
DisplayDevice rendering error

Handling:
├─ preDraw() returns NULL (no buffer)
├─ Skip frame drawing
├─ Continue to next frame
├─ Audio keeps playing
└─ Screen may freeze but app continues
```

### **Palette Loading Failures**

```
Palette file not found / corrupted

Handling:
├─ Load default palette (internal_palettes.cc)
├─ Continue with grayscale fallback
├─ Log warning message
└─ Allow selection of next palette
```

---

## CPU & GPU Load Distribution

### **CPU-Bound Operations**
- Flame effect (pixel manipulation)
- Palette expansion
- FFT (sound analysis)
- Coordinate transformations

### **I/O-Bound Operations**
- Sound device reading (blocks on /dev/dsp)
- Display blitting (GPU transfer)
- CD player status

### **Memory-Bound Operations**
- Framebuffer operations (cache-sensitive)
- Palette lookups (tight loops)

### **GPU-Bound Operations** (OpenGL)
- Texture updates
- Geometric transformations
- Lighting calculations
- Rasterization

