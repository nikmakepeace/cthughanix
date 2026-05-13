# DOS And Linux Project Differences

This compares the original DOS source in `cthug53s/` with the Linux/Unix port in the main source tree.

## Short Version

The Linux port preserves the Cthugha idea and a lot of effect DNA, but it is not a thin platform shim over the DOS program. It changes the architecture from one real-mode hardware-owning executable into a C++ Unix application with display frontends, sound-device classes, named runtime options, scalable buffers, external asset search paths, and network/file/program audio sources.

The DOS version is centered on:

```text
fixed 320x204 globals + direct hardware + function pointer arrays
```

The Linux version is centered on:

```text
CoreOption registry + SoundDevice/CthughaBuffer/CthughaDisplay/DisplayDevice objects
```

## Language And Structure

DOS:

- Mostly C, plus MASM assembly and old vendor libraries.
- One flat source directory.
- One target: `cthugha.exe`.
- Global variables and raw function pointers are the main structure.
- Effect selection is numeric/letter-position based.

Linux:

- C++ application code, with some C table generators.
- Subdirectories for `src/`, `tab/`, `map/`, `pcx/`, `doc/`, and `precompiled/`.
- Multiple targets: `xcthugha`, `cthugha`, `glcthugha`, `cthugha-server`, `tabheader`, `tabinfo`.
- Runtime behavior is organized around classes such as `SoundDevice`, `SoundAnalyze`, `AutoChanger`, `CthughaBuffer`, `CthughaDisplay`, `DisplayDevice`, `CoreOption`, `Interface`, and `Keymap`.
- Effect selection is named through `CoreOptionEntry` lists.

## Main Runtime Loop

DOS:

- `FLAMES.C::flame_cro()` is the main frame scheduler.
- Audio sampling happens inside the visual loop after flame/translation and before wave drawing.
- Quiet/peak/change logic is inside the flame loop.

Linux:

- `src/initExitDisp.cc::run()` is the shared scheduler.
- Sound reading, sound analysis, auto-changing, network server, buffer updates, display, and CD updates are separate steps.
- Quiet/beat/time change logic lives in `AutoChanger`, driven by `SoundAnalyze`.

Linux frame order:

```text
CthughaDisplay::nextFrame()
SoundDevice::operator()()
SoundAnalyze::operator()()
AutoChanger::operator()()
SoundServer::operator()()
CthughaBuffer::run()
CthughaDisplay::operator()()
CDPlayer::operator()()
```

## Buffer Model

DOS:

- Fixed `320x204` indexed-color buffer.
- Visible area is effectively `320x200`.
- Uses two global arrays, `xbuff` and `xshadow`.
- Hidden bottom rows act as flame working rows.
- Many loops assume 320-wide memory directly.

Linux:

- Default buffer is `160x100`, but sizes are configurable.
- Each buffer has active/passive allocations with 3 hidden rows above and below.
- Supports up to 3 visual buffers.
- Buffer options are per-buffer `CoreOption` objects.
- Display scaling/zoom is separated from buffer resolution.

## Effect Pipeline

DOS frame pipeline:

```text
translate
flame or PCX hold
get_stereo
FFT if enabled
wave
display
```

Linux buffer pipeline:

```text
sound-process
flashlight
border setup
flame
translate
wave
palette smoothing
swap active/passive
```

The Linux order makes sound processing an explicit option and moves palette smoothing into the buffer pipeline. DOS writes the VGA palette immediately.

## Flames, Waves, And Displays

Preserved:

- The core idea of flames diffusing previous pixels and waves drawing fresh audio-seeded pixels.
- Many effect names and algorithms: up/left/right/water/skyline/weird/zzz/fade flames, line/dot/spike/lissa/lightning/Pete/fractal waves, and the classic mirrored display modes.

Changed:

- DOS has 15 flames, 24 waves, and 8 displays in fixed arrays.
- Linux expands to 19 flames, 29 waves, more 2D displays, GL-only displays, flashlight, borders, objects, sound-process options, and multi-buffer composition.
- Linux can load object line data from `obj/`; the DOS source has no matching runtime object loader.

## Audio Approach

DOS:

- Talks directly to specific sound cards.
- Backends install C function pointers.
- Uses IRQ/DMA/DSP/vendor APIs for GUS, PAS, SBPro/SB16, and old SB SDK.
- Common sample window is `stereo[320][2]`, unsigned 8-bit.
- Noise detection is min/max range over 320 samples.

Linux:

- Normalizes input through `SoundDevice` subclasses.
- Supports OSS `/dev/dsp`, file/program/fifo input, forked playback, network client, and random noise.
- Maintains a rolling `1024`-sample signed 8-bit stereo window.
- `SoundAnalyze` computes RMS amplitude, attack/fire, fire level, intensity, speed, and noisy/silent state.
- FFT and filters are `sound-process` CoreOptions rather than flags inside the flame loop.

## Display Approach

DOS:

- Uses BIOS mode 13h or VESA 640x400 banked mode.
- Copies indexed pixels directly to segment `A000`.
- Waits for vertical sync by polling VGA port `0x03da`.
- Writes palette registers through VGA DAC ports.

Linux:

- Uses frontend-specific display devices: X11, SVGAlib, OpenGL/GLUT, and ncurses server mode.
- Converts or expands indexed buffers into frontend display buffers.
- Supports window sizing, full-screen X11 behavior, root-window drawing, GL textured geometry, and display zoom.
- Palette handling is abstracted through frontend display code rather than direct VGA DAC writes.

## Configuration And Runtime Options

DOS:

- `CTHUGHA.INI` has named sections such as `[general]`, `[startup]`, `[waves]`, `[flames]`, `[displays]`, `[oldini]`.
- Effect choices are mostly letters/numbers.
- Pressing `i` appends an old underscore-separated full-state preset to the INI file.
- Command-line flags are compact single-letter options.

Linux:

- INI entries use `cthugha.option: value` and `cthugha.feature.buffer.entry: on/off`.
- Supports wildcard `?` entries.
- Reads several config locations, including system libdir, home auto/user files, local files, `--path`, X resources, and command line.
- Pressing `a` writes `~/.cthugha.auto`.
- Core options support names, locks, history, hotkey save/restore, and use flags.

## Asset Loading

DOS:

- Active source scans current directory for `*.map`, `*.pcx`, and `*.tab`.
- PCX files are capped at 20 and stored through EMS/XMS if available.
- Translation tables are raw fixed-size 320x204 integer maps.
- Palettes are converted to 6-bit VGA DAC values.

Linux:

- Uses search paths like `./`, `./map/`, `./pcx/`, `./tab/`, `CTH_LIBDIR/...`, and `--path`.
- Can load `.gz` assets by spawning `gzip -cd`.
- Translation tables can be generated from `.cmd` descriptors, loaded on demand, cached late, stretched to the current buffer size, and stored with headers.
- Palettes remain 8-bit RGB, are brightened if too dark, can be smoothed over time, and can be generated randomly.
- PCX images are CoreOptions and are centered into the active/passive buffer.

## CD And Mixer

DOS:

- Uses MSCDEX device requests and sound-card mixer registers/vendor APIs.
- CD/mixer UI is a DOS text-mode screen with keyboard and optional mouse support.
- Entering CD UI changes video mode and then restores the visualizer display mode.

Linux:

- Uses Unix CD-ROM ioctls where available.
- Mixer support is OSS-style and optional through configure checks.
- CD updates run as a module in the shared scheduler.
- `cthugha-server` can broadcast sound to visualizer clients, which has no DOS equivalent.

## Build And Distribution

DOS:

- MSVC 1.0-era makefile.
- MASM assembly.
- Old binary `.LIB` dependencies.
- Runtime asset files are expected externally.
- License text in `CTHUGHA.H` allows copying but forbids commercial use without permission and asks for credit.

Linux:

- Autotools project with generated `configure` and `Makefile`s.
- Includes docs, example config, palette maps, compressed PCX files, table generators, and old precompiled Linux binaries.
- Relicensed/renamed lineage is documented in `README`, `COPYING`, and `ChangeLog`.

## What Linux Kept

The Linux port clearly inherits:

- "Audio seeded image processing" as the program model.
- The flame/wave/display vocabulary.
- PCX image flash support.
- Fractint-style `.map` palette format.
- Translation table remapping.
- CD input/CD-track control as a first-class workflow.
- Silence messages and automatic effect changes.
- The idea that audio both draws pixels and changes the visual state.

## What Linux Reworked Most

The biggest rewrites are:

- Hardware ownership became OS device/front-end abstraction.
- Global fixed-size effect state became per-buffer `CoreOption` state.
- Card-specific recording became normalized `SoundDevice` input.
- Peak/minmax detection became RMS/attack/fire analysis.
- Fixed binary translation tables became command/header/on-demand/stretchable tables.
- BIOS text screens became configurable keymaps and interface screens.
- One executable became several build targets with shared source groups.

## Practical Takeaway

For porting or modernization, the DOS code is best treated as the original effect and behavior reference, while the Linux port is the architectural reference for separating sound, analysis, options, buffers, display, and UI. The DOS algorithms are valuable, but its seams are hardware-era seams; the Linux project shows the later Unix-era seams.
