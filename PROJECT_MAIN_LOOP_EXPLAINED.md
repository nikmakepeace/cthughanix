# Main Loop Explained

This is a guided walk through the Linux/Unix CthughaNix main loop. It ignores the DOS source and follows the modern port in `src/`.

The short version:

```text
frontend event loop
  -> run()
      -> update frame time
      -> read sound
      -> analyze sound
      -> maybe change visual options
      -> update all visual buffers
      -> draw current buffer(s) to the frontend
      -> update CD state
      -> handle suspend
```

The important files to keep open are:

- `src/initExitDisp.cc`: process startup and `run()`.
- `src/CthughaBuffer.*`: the visual buffer pipeline.
- `src/SoundDevice.*`: sound input and normalization.
- `src/SoundAnalyze.*`: amplitude, attack, and silence analysis.
- `src/AutoChanger.*`: automatic effect changes.
- `src/flames.cc`: flame effects.
- `src/waves.cc`: wave effects.
- `src/translate.cc`: coordinate remapping effects.
- `src/display.cc`: 2D screen mapping effects.
- `src/CthughaDisplay*.cc`: frontend display composition.
- `src/DisplayDevice*.cc`: X11, SVGAlib, or OpenGL event loops and screen IO.
- `src/Interface.*`, `src/keymap.*`: key handling and on-screen UI.

## 1. Startup: `main()`

Graphical frontends enter at `src/initExitDisp.cc::main()`.

Startup does this:

```text
srand(time(0))
drop elevated uid
get_pre_params()
cth_init()
get_params()
title()
init_imath()
SoundDevice::newSD()
new SoundServer
new CDPlayer
init_mixer()
CthughaBuffer::initAll()
newDisplayDevice()
newCthughaDisplay()
CoreOption::changeToInitial()
Interface::set("main")
Keymap::init()
new AutoChanger
displayDevice->mainLoop()
```

Read this as "build all the long-lived singletons, then hand control to the selected frontend." The code is C++, but it is not deeply object-owned. A lot of important objects are globals: `soundDevice`, `soundServer`, `cdPlayer`, `cthughaDisplay`, `displayDevice`, `autoChanger`, and `soundAnalyze`.

## 2. The Frontend Loop Calls `run()`

The program has several frontends. Each frontend owns the platform event loop, but they all converge on `run()`.

- X11: `src/DisplayDeviceX11.cc::DisplayDeviceX11::mainLoop()` handles pending X events, polls keys, resizes the display, then calls `run(1)`.
- SVGAlib: `src/DisplayDeviceSvga.cc::DisplayDeviceSvga::mainLoop()` calls `Interface::current->run()` and `run(1)` in a loop.
- OpenGL: `src/DisplayDeviceGL.cc::idleCB()` calls `run(0)` and schedules a GLUT redraw; `drawCB()` later calls `(*cthughaDisplay)()`.

The `doDisplay` argument decides whether `run()` itself calls the display step. X11 and SVGAlib pass `1`; OpenGL passes `0` because GLUT separates idle updates from drawing callbacks.

## 3. The Shared Scheduler: `run(int doDisplay)`

The core scheduler is short and worth reading directly in `src/initExitDisp.cc`.

```text
cthughaDisplay->nextFrame()
(*soundDevice)()
soundAnalyze()
(*autoChanger)()
(*soundServer)()
CthughaBuffer::run()
if(doDisplay)
    (*cthughaDisplay)()
(*cdPlayer)()
pause/suspend handling
```

That is the main loop. Everything else explains one of these calls.

## 4. Concept: CoreOption

Before following the frame, understand `CoreOption`.

`CoreOption` is the runtime registry for visual choices. It is more than a scalar setting. A `CoreOption` owns a list of `CoreOptionEntry` objects, tracks the current entry, supports locks, supports random changes, and can be addressed by config/keymap/UI code.

Examples:

- `flame`
- `wave`
- `palette`
- `translate`
- `display`
- `sound-process`
- `border`
- `flashlight`

The pattern is:

```cpp
current->flame();
current->wave();
screen();
```

Those calls do not call a hard-coded function. They call the currently selected `CoreOptionEntry::operator()()`.

The main implementation is in `src/CoreOption.cc` and `src/CoreOption.h`.

## 5. Concept: CthughaBuffer

`CthughaBuffer` is the off-screen indexed-color image that flames and waves modify.

Key fields in `src/CthughaBuffer.h`:

```cpp
unsigned char * activeBuffer;
unsigned char * passiveBuffer;
```

The names are easy to misread. During `CthughaBuffer::run()`:

- `activeBuffer` is the buffer being written for the next finished image.
- `passiveBuffer` is the previous/current finished image.
- At the end of the buffer step, the pointers are swapped.
- The display code reads from `passive_buffer`, so after the swap it sees the newly completed frame.

Default dimensions are in `src/CthughaBuffer.cc`:

```cpp
int BUFF_WIDTH = 160;
int BUFF_HEIGHT = 100;
```

`src/cth_buffer.h` defines `BUFF_SIZE` as `BUFF_WIDTH * BUFF_HEIGHT`.

Each buffer allocation has three hidden rows above and below the visible buffer. The flame code uses those rows as boundary data.

## 6. Step 1 In `run()`: Start The Frame

`cthughaDisplay->nextFrame()` is implemented in `src/CthughaDisplay.cc`.

It:

- updates global `now`;
- computes `deltaT`;
- updates `fps`;
- enforces `maxFPS` by sleeping if needed.

This gives later modules a consistent frame time and keeps the visualizer from running completely unbounded.

## 7. Step 2: Read Sound

`(*soundDevice)()` calls `SoundDevice::operator()()` in `src/SoundDevice.cc`.

The selected backend fills `tmpData` by implementing `read()`. Backends include:

- `SoundDeviceDSPIn`: OSS `/dev/dsp`.
- `SoundDeviceFile`: file/program/fifo source.
- `SoundDeviceFork`: forked playback/reader path.
- `SoundDeviceNet`: UDP client from `cthugha-server`.
- `SoundDeviceRandom`: no-sound/random test source.

Then `SoundDevice::convert()` normalizes the input into:

```cpp
char2 * data;        // 1024 stereo signed 8-bit samples
char2 dataProc[1024] // processed copy used by waves
```

The read size is not always 1024 fresh samples. `SoundDevice::operator()()` keeps a rolling 1024-sample window: it slides old samples down and converts the newest backend data into the tail.

## 8. Step 3: Analyze Sound

`soundAnalyze()` calls `SoundAnalyze::operator()()` in `src/SoundAnalyze.cc`.

It computes:

- `amplitudeLeft` and `amplitudeRight`: RMS-ish amplitude for each channel.
- `amplitude`: average amplitude.
- `noisy`: whether either side is above `sound_minnoise`.
- `attackLevel`: accumulated rise while the sound gets louder.
- `fire`: a beat/attack event emitted when amplitude begins falling after a rise.
- `fireLevel`: accumulated fire used by automatic changing.
- `intensity`: smoothed normalized amplitude.
- `speed`: approximate rate of recent fire events.

"Fire" here is not the visual flame. It is the analysis term for a detected attack/beat.

## 9. Step 4: AutoChanger

`(*autoChanger)()` calls `AutoChanger::operator()()` in `src/AutoChanger.cc`.

This decides whether to change visual options automatically:

- If the sound has been quiet long enough, it may show a silence message.
- If silence ends after a quiet gap, it can trigger a change.
- If `soundAnalyze.fireLevel` exceeds `changeFireLevel`, it can trigger a change.
- If enough time has passed, it can trigger a change.

The actual change is:

```cpp
CoreOption::changeOne()
// or
CoreOption::changeAll()
```

So this does not directly know about specific flame or wave functions. It asks the CoreOption system to move current selections.

## 10. Step 5: SoundServer

`(*soundServer)()` updates the network sound server path. In normal local visualizer use this is not the central visual operation, but it allows a separate `cthugha-server` process or network clients to share sound data.

The relevant files are `src/SoundServer.*`, `src/SoundDeviceNet.cc`, and `src/network.*`.

## 11. Step 6: Update Visual Buffers

`CthughaBuffer::run()` in `src/CthughaBuffer.cc` is where the frame becomes pixels.

For every active buffer:

```text
current = buffers + j
soundProcess()
flashlight()
set border rows
done_translate = 0
flame()
translate()
wave()
smoothPalette()
swap(activeBuffer, passiveBuffer)
```

This order matters.

### 11.1 Sound Process

`current->soundProcess()` calls the selected entry from `src/SoundProcess.cc`.

The built-in entries are:

- `none`: copy `soundDevice->data` to `soundDevice->dataProc`.
- `Filter1`: slope-limits sharp sample jumps.
- `Filter2`: low-pass-ish smoothing.
- `FFT`: transforms the 1024-sample stereo window and writes the result to `dataProc`.

Waves normally read `dataProc`, not raw `data`. This lets a visual use raw waveform samples, filtered samples, or FFT bins without changing the wave code.

### 11.2 Flashlight

`current->flashlight()` calls an optional brightening effect from `src/Flashlight.cc`.

It is another CoreOption entry. Despite its position inside the buffer update, it does not change the indexed pixels in the buffer. It copies the current palette, brightens low palette entries according to `soundAnalyze.fire`, and installs that temporary palette with `current->setPalette(Pal)`.

That means flashlight is a color/intensity effect for the current frame, not a drawing effect like a wave or a feedback effect like a flame.

### 11.3 Border

The border step fills the three hidden rows above and below `activeBuffer`.

The selected `border` CoreOption decides whether those rows are:

- zero;
- copied from sound data;
- filled with current amplitude;
- filled with 255.

Flames read neighboring pixels. These hidden rows are boundary conditions for that diffusion.

### 11.4 Concept: What Is A Flame?

A flame is a feedback/decay function over the previous image.

It does not usually draw the audio waveform. Instead, it takes pixels that already exist and propagates them through the buffer: up, down, left, right, watery, faded, generalized, and so on. It gives the visualizer memory. Without a flame, a wave would draw points or lines and they would simply sit there or vanish. With a flame, the drawn sound marks smear, cool, drift, and become fluid patterns.

In source:

- The flame entries are registered in `src/flames.cc::_flames`.
- Each entry is a `FlameEntry`, which wraps a C function like `flame_upslow()`.
- `init_flames()` precomputes lookup tables such as `divsub`.

A representative flame:

```cpp
void flame_upslow()
```

It swaps active/passive pointers using the `PTR` macro, then writes new pixels by averaging neighboring pixels from the old frame and decaying the result through `divsub`. The effect is "old image moves upward and cools."

So, mentally:

```text
old finished image -> neighboring-pixel math -> new decayed image
```

### 11.5 Concept: What Is Translate?

A translation table is a coordinate remap. It says, for each destination pixel, which source pixel to read.

In source:

- Loading is in `src/translate.cc::init_translate()`.
- `.cmd` files can generate tables with helper programs.
- `.tab` files can be loaded directly.
- Tables can be loaded on demand.
- `TranslateEntry::operator()()` applies the remap.

The key operation is:

```text
dst_pixel[i] = src_pixel[translation_table[i]]
```

This can swirl, stretch, rotate, fold, or otherwise warp the current buffer. Some flame paths can fold translation into the flame step, so `done_translate` prevents duplicate translation.

### 11.6 Concept: What Is A Wave?

A wave is the fresh drawing seeded by the current sound.

If a flame is memory, a wave is new input. Wave functions look at the processed sound samples in `soundDevice->dataProc` and draw points, vertical lines, horizontal lines, spikes, Lissajous shapes, lightning, objects, spirals, and other geometry into `active_buffer`.

In source:

- The wave entries are registered in `src/waves.cc::_waves`.
- Each entry is a `WaveEntry`, which wraps a function like `wave_dotHor()`.
- Helpers such as `prepareSoundData()`, `putat()`, `putat_cut()`, `do_vwave()`, and `do_hwave()` turn samples into pixels.
- Wave colors use `tcolor(x)`, which indexes one of the 10 sound color tables.

Example:

```cpp
void wave_dotHor()
```

This:

1. Calls `prepareSoundData(BUFF_WIDTH)`.
2. Scales 1024 processed samples down to one sample per x position.
3. Draws left-channel and right-channel dots in different horizontal halves.
4. Uses `putat_cut()` to write small cross-shaped marks into `active_buffer`.

Mentally:

```text
current sound samples -> positions/colors -> new marks in active_buffer
```

Those marks become fuel for the next frame's flame.

### 11.7 Palette Smoothing

`current->smoothPalette()` moves `currentPalette` toward the selected palette over time.

Palettes are 256 entries of RGB:

```cpp
typedef unsigned char Palette[256][3];
```

The buffer stores only 8-bit color indexes. The palette decides what those indexes mean on screen. Palette smoothing is why color changes can drift rather than snap.

### 11.8 Swap Buffers

At the end:

```cpp
swap(activeBuffer, passiveBuffer)
```

The freshly generated image becomes the passive/current image, ready for display. The old passive image becomes the next active write target.

## 12. Step 7: Draw To The Frontend

If `doDisplay` is true, `run()` calls:

```cpp
(*cthughaDisplay)()
```

This means "compose Cthugha's indexed buffer into the selected frontend."

For X11, read `src/CthughaDisplayX11.cc::CthughaDisplayX11::operator()()`.
For SVGAlib, read `src/CthughaDisplaySVGA.cc::CthughaDisplaySVGA::operator()()`.
For OpenGL, read `src/CthughaDisplayGL.cc::CthughaDisplayGL::operator()()`.

The classic 2D path does this:

```text
displayDevice->preDraw()
choose output buffer
checkZoom()
while(screen()) draw passive buffer into display buffer
maybe mirror horizontally
expand indexed palette to target pixel format
maybe mirror vertically
clear border
zoom2Screen()
display interface text and errors
displayDevice->postDraw()
```

## 13. Concept: What Is `screen()`?

`screen()` is another `CoreOption`, defined in `src/display.cc`:

```cpp
CoreOption screen(-1, "display", screenEntries);
```

It is unfortunately named if you are new to the code. A "screen" entry is not the OS window. It is a display mapping from the Cthugha buffer into a larger display buffer.

Examples from `src/display.cc`:

- `screen_up`: copy rows in normal order.
- `screen_down`: flip vertically.
- `screen_2hor`: horizontal split/mirror.
- `screen_4hor`: kaleidoscope.
- `screen_hfield`: heightfield.
- `screen_roll`, `screen_bent`, `screen_plate`: more 3D-ish mappings.

These functions read `passive_buffer`, which is the completed Cthugha image after the buffer swap.

## 14. Concept: CthughaDisplay vs DisplayDevice

There are two layers with similar names:

`CthughaDisplay`

- Knows Cthugha buffer geometry.
- Runs `screen()`.
- Mirrors and zooms.
- Expands palette indexes to frontend pixels.
- Draws UI text through the device.

`DisplayDevice`

- Knows the platform.
- X11 creates windows/images, handles X events, copies to the X server.
- SVGAlib owns console graphics buffers.
- OpenGL owns GLUT callbacks, GL state, and buffer swaps.

If you are tracing "pixels to screen", the route is:

```text
CthughaBuffer passive_buffer
  -> src/display.cc screen function
  -> CthughaDisplay buffer/expandedBuffer
  -> DisplayDevice preDraw/postDraw
  -> X11/SVGA/GL screen
```

## 15. Step 8: CDPlayer

`(*cdPlayer)()` updates CD-ROM/player state after drawing. The CD player is separate from sound input: it controls playback, while actual audio still reaches Cthugha through the chosen sound device/mixer path.

See `src/CDPlayer.*`.

## 16. Step 9: Suspend Handling

At the end of `run()`, the code checks `cthugha_pause`.

The signal handlers in `src/initExitDisp.cc` set this flag for `SIGTSTP`/`SIGCONT` behavior. The actual suspend happens only after the graphics work finishes, so the process is not stopped in the middle of a drawing operation.

## 17. Where Keyboard Input Fits

Keyboard input is not inside `run()` directly. It is handled by the frontend loop and the `Interface` system.

For example, X11:

```text
DisplayDeviceX11::mainLoop()
  -> collect X events
  -> keys_x11(...)
  -> Interface::current->run()
  -> run(1)
  -> Interface::current->run()
```

`Interface::run()` in `src/Interface.cc` calls `getkey()` until no key remains. It dispatches keys through `Keymap::action()`.

The keymap system can:

- change CoreOptions;
- enter option/help/CD screens;
- lock options;
- save/restore hotkeys;
- request quit;
- change sound/display options.

Look at:

- `src/default.keymap`
- `src/keymap.cc`
- `src/Interface.cc`
- `src/InterfaceList.cc`
- frontend key translators such as `src/xwin_keys.cc`, `src/nonx_keys.cc`, and `src/GL_keys.cc`.

## 18. A Concrete Source Walk

If you want to step through one whole frame in your editor, use this route:

1. Open `src/initExitDisp.cc`.
2. Read `main()` until `displayDevice->mainLoop()`.
3. Jump to your frontend's `mainLoop()`, probably `src/DisplayDeviceX11.cc::mainLoop()`.
4. Return to `src/initExitDisp.cc::run()`.
5. Step into `src/CthughaDisplay.cc::nextFrame()`.
6. Step into `src/SoundDevice.cc::SoundDevice::operator()()`.
7. Step into the active backend's `read()`, such as `src/SoundDeviceDSP.cc::SoundDeviceDSPIn::read()`.
8. Step into `src/SoundAnalyze.cc::SoundAnalyze::operator()()`.
9. Step into `src/AutoChanger.cc::AutoChanger::operator()()`.
10. Step into `src/CthughaBuffer.cc::CthughaBuffer::run()`.
11. In the buffer step, jump to the current `sound-process` in `src/SoundProcess.cc`.
12. Jump to the current flame in `src/flames.cc`.
13. Jump to `src/translate.cc::TranslateOption::operator()()` if translate is enabled.
14. Jump to the current wave in `src/waves.cc`.
15. Return to `CthughaBuffer::smoothPalette()`.
16. Step into `src/CthughaDisplayX11.cc::operator()()` or `src/CthughaDisplaySVGA.cc::operator()()`.
17. Jump to the current `screen()` function in `src/display.cc`.
18. Finish in the frontend `DisplayDevice` `postDraw()` path.

## 19. How To Think About One Frame

A single frame is easiest to picture as two layers of behavior.

The sound/control layer:

```text
read samples
normalize to signed stereo bytes
compute amplitude/fire/silence
maybe change CoreOptions
```

The image layer:

```text
process sound data
diffuse old pixels with a flame
warp pixels with translate
draw fresh audio marks with a wave
smooth palette
swap buffers
map buffer to frontend display
```

The essential Cthugha feedback loop is:

```text
wave draws sound into the buffer
flame turns that drawing into motion/trails next frame
translate warps the motion
palette determines the color feel
autochanger uses sound analysis to keep changing the choices
```

That is the project in miniature.
