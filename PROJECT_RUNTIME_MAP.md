# Runtime Map

## Startup Flow

Graphical frontends use `src/initExitDisp.cc::main`.

```text
main(argc, argv)
  srand(time(0))
  seteuid(getuid())                    # drop root until a frontend needs it
  get_pre_params()                     # early --path/--verbose/text options
  cth_init()                           # frontend-specific init
  get_params()                         # read ini files, then command line
  title()
  init_imath()
  atexit(deleter)
  init_ncurses() if needed
  SoundDevice::newSD()
  new SoundServer()
  new CDPlayer()
  init_mixer()
  CthughaBuffer::initAll()
  newDisplayDevice()
  newCthughaDisplay()
  CoreOption::changeToInitial()
  Interface::set("main")
  Keymap::init()
  new AutoChanger()
  displayDevice->mainLoop()
```

`cthugha-server` uses `src/serv_main.cc::main`. It skips visual buffers and display frontends, forces ncurses text output, initializes sound/server/CD/mixer/keymaps, sets the `server` interface, and loops over sound reads plus server broadcasts.

## Frame Loop

`run(int doDisplay)` in `src/initExitDisp.cc` is the shared per-frame scheduler.

```text
CthughaDisplay::nextFrame()
  updates now, deltaT, FPS, and optional max-FPS sleep

SoundDevice::operator()()
  reads raw sound from current backend
  maintains a rolling 1024-sample signed 8-bit stereo `char2` window

SoundAnalyze::operator()()
  computes RMS left/right amplitude
  computes smoothed amplitude, attack/fire, fireLevel, noisy/silent state

AutoChanger::operator()()
  may change one/all CoreOptions based on silence, fire, or time

SoundServer::operator()()
  broadcasts sound to clients and accepts connect/disconnect requests

CthughaBuffer::run()
  applies sound processing, flashlight, border, flame, translate, wave, palette smoothing

CthughaDisplay::operator()()
  frontend-specific display composition

CDPlayer::operator()()
  updates CD playback state
```

The frame loop is cooperative. There are no threads in the visualizer. The code uses processes for some sound/table-loading work.

### Pause and Resume

Graphical frontends install a `SIGTSTP` handler after initialization. Pressing `^Z` sets `cthugha_pause`; the main loop waits until graphics work is finished, calls `exit_sound()`, and raises `SIGTSTP` for the default terminal suspend behavior.

On `SIGCONT`, the continuation handler calls `init_sound()` and reinstalls the stop handler before the display loop continues. This keeps suspend/resume out of the middle of drawing code.

## Audio Pipeline

### Input Selection

`SoundDevice::newSD()` creates one of:

- `SoundDeviceDSPIn`: OSS `/dev/dsp` read.
- `SoundDeviceNet`: UDP client from `cthugha-server`.
- `SoundDeviceRandom`: random noise for `--no-sound`.
- `SoundDeviceFile`: direct file/program/fifo sound source.
- `SoundDeviceFork`: parent/child wrapper used for non-silent `--play`, so playback can continue while the parent visualizer reads shared data.

The selected device reads raw bytes into `tmpData`. `SoundDevice::convert()` normalizes supported formats into:

```text
soundDevice->data[1024][2]      # signed 8-bit stereo, [left/right-ish] pairs
soundDevice->dataProc[1024][2]  # processed copy used by wave drawing
```

`SoundDevice::operator()()` does not necessarily read 1024 fresh samples every frame. It asks the backend for `size` samples, where `size` is derived from the larger buffer dimension, then slides/updates the 1024-sample window. With the default `160x100` buffer, the fresh read size starts at 128 samples.

CD playback is separate from `SoundDevice`: `CDPlayer` controls the CD-ROM drive through ioctls, while audio still reaches Cthugha through the machine's mixer/DSP input path.

### File/Program Playback

`SoundDeviceFile` recognizes format by filename extension:

- `.wav`: direct read after parsing a WAV header.
- `.mod`: external `xmp`, if configured.
- `.mp3`: external `mpg123` or `l3dec`, if configured.
- other: raw data.

External players are started through `/bin/sh -c` and communicate through a fifo. `tmpnam()` is used to generate a fifo name unless `--fifo` overrides it.

### Analysis

`SoundAnalyze` computes:

- `amplitude`, `amplitudeLeft`, `amplitudeRight`: RMS-ish amplitude.
- `noisy`: above `minnoise`.
- `fire`: attack peak when amplitude falls after rising.
- `fireLevel`: accumulated fire used by the auto changer.
- `intensity`: smoothed normalized amplitude.
- `speed`: approximate event speed based on recent fires.

### Sound Processing CoreOption

`src/SoundProcess.cc` registers:

- `none`: copy raw normalized samples to `dataProc`.
- `Filter1`: slope-limited sample smoothing.
- `Filter2`: low-pass-ish filter.
- `FFT`: custom 1024-sample FFT using left as real and right as imaginary.

## Visual Buffer Pipeline

`CthughaBuffer` owns active/passive 8-bit indexed-color buffers. Default buffer size is `160x100`; `--buff-size` can choose predefined or explicit sizes.

Each active buffer has two actual allocations with a 3-line border above and below. Macros in `src/CthughaBuffer.h` expose:

```text
active_buffer
passive_buffer
```

### Per-Buffer Step Order

`CthughaBuffer::run()` loops over `nBuffers` and applies:

```text
soundProcess()
flashlight()
border setup
flame()
translate()
wave()
smoothPalette()
swap(activeBuffer, passiveBuffer)
```

The order matters:

- `flame` propagates/decays the previous image into the next active buffer.
- `translate` remaps pixels using translation tables unless the flame already folded translation into its own loop.
- `wave` draws fresh sound-reactive points/lines into the buffer.
- palette smoothing moves the current palette toward the selected palette.

## CoreOption System

`CoreOption` is the runtime effect registry. It stores:

- a per-option linked-list membership for "change one/all";
- current value;
- initial entry from config/CLI;
- `use` flag on entries;
- per-option lock;
- history stack for restore;
- 10 hotkey values.

`CoreOptionEntry` is callable via `operator()()`. Subclasses wrap functions, loaded assets, display functions, GL functions, or no-op entries.

Initial values come from ini files and command-line arguments. `CoreOption::changeToInitial()` applies them after buffers and display are initialized.

## Display Pipeline

The classic 2D display path is:

```text
displayDevice->preDraw()
choose direct or temporary indexed buffer
screen() maps passive_buffer into CthughaDisplay::buffer
optional horizontal mirror
palette expansion if target is not 8-bit indexed
optional vertical mirror
clear border
zoom2Screen()
displayDevice->prePrint()
Interface::display()
errors.display()
displayDevice->postPrint()
displayDevice->postDraw()
```

The display function selected by the `display` CoreOption comes from `src/display.cc` for 2D frontends. It maps one `BUFF_WIDTH x BUFF_HEIGHT` buffer into an image that is commonly mirrored to `2*BUFF_WIDTH x 2*BUFF_HEIGHT`.

For OpenGL, `src/CthughaDisplayGL.cc` uses a different sequence:

```text
background() before screen
set projection/modelview
light()
screen()
fly()
background() after screen
```

GL display functions in `src/GL_display.cc` upload passive buffers as paletted textures and draw textured geometry.

## User Input and Interface

The UI is not a separate event framework. Each frontend polls/receives keys, then calls `Interface::current->run()`. `Interface` dispatches keys through `Keymap`.

Default mappings come from `src/default.keymap`, transformed into `src/default.keymap.str`. A user keymap can be loaded with `--keymap`.

Keymaps are named contexts such as:

- `default`
- `main`
- `Help`
- `CD`
- `sound`
- `Options`
- `CoreOptions`
- `OptionElement`
- `CoreOptionElement`

Actions are registered by static `ACTION(name)` objects in `keymap.cc`, `Interface.cc`, `InterfaceHelp.cc`, `InterfaceList.cc`, `CDPlayer.cc`, and related modules.

## Configuration Flow

`read_ini()` reads multiple config locations in order, with later files overriding earlier settings:

1. `CTH_LIBDIR/cthugha.ini`
2. `~/.cthugha.auto`
3. `~/.cthugha.ini`
4. `./cthugha.ini`
5. `--path DIR` -> `DIR/cthugha.ini`
6. X11 resource database for `xcthugha`
7. command line options

Entry format is:

```text
cthugha.option: value
cthugha.feature.buffer: value
cthugha.feature.buffer.entry: on/off
```

The ini reader supports `?` wildcards in entry names. Pressing `a` at runtime writes `~/.cthugha.auto`.
