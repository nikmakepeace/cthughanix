# Project Structure Map

## Top Level

```text
.
|-- src/                 application source
|-- tab/                 translation-table generators and .cmd descriptors
|-- map/                 256-color palette maps
|-- pcx/                 compressed PCX images
|-- doc/                 original Texinfo, info, and manpage docs
|-- precompiled/         old 32-bit Linux binaries
|-- Makefile.am          automake root
|-- configure.in         old autoconf input
|-- configure            generated configure script
|-- Makefile             generated Makefile
|-- config.h             generated feature config
|-- cthugha.ini.eg       example config file
|-- build_errors.txt     captured historical build failure
`-- CVS/                 imported CVS metadata
```

This is a source snapshot, not a Git repository. The `CVS/` directories and `precompiled/` contents are historical artifacts, not active build inputs for normal source builds.

## Source Layout

The source tree is organized by subsystem rather than by one namespace or library boundary.

### Entrypoints

- `src/initExitDisp.cc`: main entrypoint for graphical visualizers, shared by `cthugha`, `xcthugha`, and `glcthugha`.
- `src/serv_main.cc`: main entrypoint for `cthugha-server`.
- `src/tabheader.cc`, `src/tabinfo.cc`: table utility programs.

### Shared Runtime Core

- `src/cthugha.h`: global platform config, common declarations, timing, logging, and `run()`.
- `src/CthughaBuffer.*`: per-buffer visual pipeline state and core options.
- `src/CthughaDisplay.*`: converts current visual buffer(s) into frontend display buffers.
- `src/CoreOption.*`, `src/CoreOptionEntry.cc`: effect registry, history, hotkeys, loading helpers.
- `src/Option.*`, `src/OptionInt.cc`: scalar option classes.
- `src/AutoChanger.*`: automatic effect changes based on time, silence, and beat/fire level.
- `src/imath.*`: integer math tables/helpers used by visual code.
- `src/misc.cc`: logging, time helpers, `systemf()`.

### Audio

- `src/SoundDevice.*`: base class, format conversion, device selection.
- `src/SoundDeviceDSP.cc`: OSS `/dev/dsp` input/output implementation.
- `src/SoundDeviceFile.cc`: `.wav`, `.mp3`, `.mod`, raw, fifo, and external decoder playback.
- `src/SoundDeviceFork.cc`: separate sound-reading process using SysV shared memory.
- `src/SoundDeviceNet.cc`: UDP network sound client.
- `src/SoundDeviceRandom.cc`: random-noise debug device.
- `src/SoundAnalyze.*`: amplitude, stereo amplitude, attack/fire, silence, intensity, speed.
- `src/SoundProcess.cc`: `none`, two filters, and FFT as core options.
- `src/sound.cc`, `src/sound.h`: global sound options and sound interface screen.
- `src/Mixer.cc`: OSS mixer integration.
- `src/CDPlayer.*`: CD-ROM ioctl integration.
- `src/SoundServer.*`: sound broadcast server used by visualizers and standalone server.

### 2D Visual Effects

- `src/flames.*`: decay/propagation functions that transform the previous frame.
- `src/waves.*`: waveform and beat-driven drawing functions.
- `src/sound_tables.cc`: 10 built-in wave color lookup tables.
- `src/translate.*`: translation-table loading and per-pixel remapping.
- `src/Flashlight.cc`: optional buffer brightening effect.
- `src/display.cc`: 2D screen mapping functions such as mirrored, split, heightfield, roll, bent, plate.
- `src/palettes.cc`, `src/initial_palettes.cc`: internal and external palette handling.
- `src/pcx.*`: PCX loading, display, and screenshot save.

### Display Frontends

- `src/DisplayDevice.*`: abstract display device and text rendering base.
- `src/DisplayDeviceX11.cc`: X11/Xt/Xaw display, window/root modes, MIT-SHM, palette setup.
- `src/DisplayDeviceX11-Panel.cc`: optional X11 panel.
- `src/CthughaDisplayX11.cc`: X11 display-buffer expansion and copy.
- `src/DisplayDeviceSvga.cc`: SVGAlib console display and setuid-dependent startup.
- `src/CthughaDisplaySVGA.cc`: SVGAlib display-buffer expansion and copy.
- `src/DisplayDeviceGL.cc`: GLUT/OpenGL window, callbacks, GL text, texture prep.
- `src/CthughaDisplayGL.cc`: GL drawing sequence.
- `src/GL_display.cc`, `src/GL_Background.cc`, `src/GL_Light.cc`, `src/GL_Fly.cc`: OpenGL display modes and GL-only effects.

### UI, Input, and Configuration

- `src/Interface.*`: interface registry, screens, selectable option rows, error display.
- `src/InterfaceHelp.cc`, `src/InterfaceCredits.cc`, `src/InterfaceList.cc`: specific interface screens.
- `src/keymap.*`: configurable keymaps and action dispatch.
- `src/default.keymap`, `src/default.keymap.str`: default keymap source and generated C string include.
- `src/keys.cc`: key symbol translation and terminal/X/GL key polling.
- `src/nonx_keys.cc`, `src/xwin_keys.cc`, `src/GL_keys.cc`: compile wrappers for key handling.
- `src/options.cc`: command-line and ini option handling.
- `src/nonx_options.cc`, `src/xwin_options.cc`, `src/svga_options.cc`, `src/GL_options.cc`, `src/serv_options.cc`: compile wrappers/stubs for option handling.
- `src/IniFiles.cc`: ini search order, wildcard matching, generated `.cthugha.auto`.
- `src/info_title_usage.cc`: title/help/usage output.
- `src/disp-ncurses.cc`: ncurses setup/teardown.
- `src/joystick.*`: Linux joystick input for GL camera movement.

### Stubs

- `src/nonGL_stubs.cc`: defines dummy GL-related options/effects for non-GL builds.
- `src/serv_stubs.cc`, `src/serv_display.cc`: define dummy visual/display pieces for the server build.

## Build Targets and Source Groups

`src/Makefile.am` defines common source groups:

- `GENSRC`: shared option, sound, UI, network, misc, and base classes.
- `DISPSRC`: `GENSRC` plus visual effect code, palettes, PCX, translation, buffer, and display init.
- `NONXSRC`: non-X key/options wrappers.

Target-specific source sets:

| Target | Purpose | Distinct source pieces |
| --- | --- | --- |
| `cthugha` | SVGAlib console frontend | `DisplayDeviceSvga.cc`, `CthughaDisplaySVGA.cc`, `svga_options.cc`, `nonx_keys.cc`, `nonGL_stubs.cc` |
| `xcthugha` | X11 frontend | `DisplayDeviceX11.cc`, `DisplayDeviceX11-Panel.cc`, `CthughaDisplayX11.cc`, `xwin_keys.cc`, `xwin_options.cc`, `nonGL_stubs.cc` |
| `glcthugha` | OpenGL/GLUT frontend | `DisplayDeviceGL.cc`, `CthughaDisplayGL.cc`, `GL_display.cc`, `GL_Light.cc`, `GL_Background.cc`, `GL_Fly.cc`, `GL_keys.cc`, `GL_options.cc` |
| `cthugha-server` | ncurses sound server | `serv_main.cc`, `serv_display.cc`, `serv_stubs.cc`, `serv_options.cc`, `nonx_keys.cc`, `nonGL_stubs.cc` |
| `tabheader` | Add/emit `.tab` headers | `tabheader.cc` |
| `tabinfo` | Inspect `.tab` headers | `tabinfo.cc` |

The wrapper files are important: several compile modes include implementation files directly after defining a macro, for example `xwin_options.cc` defines `CTH_XWIN` and includes `options.cc`.

## Asset Directories

### `map/`

Contains 169 palette maps. Format: up to 256 lines of `R G B` values, 0-255, with optional comments. Loader: `src/palettes.cc`.

Search path:

```text
./
./map/
CTH_LIBDIR/map/
--path DIR -> DIR/map/
```

### `pcx/`

Contains 6 gzip-compressed PCX files. Loader: `src/pcx.cc`, through `CoreOption::load`, which can read `.gz` by spawning `gzip -cd`.

Search path:

```text
./
./pcx/
CTH_LIBDIR/pcx/
--path DIR -> DIR/pcx/
```

### `tab/`

Contains `.cmd` descriptors and generator programs. `.cmd` files are text descriptors:

```text
cmdtab
Description
command %d %d [other args]
```

The `%d %d` placeholders receive `BUFF_WIDTH` and `BUFF_HEIGHT`. Translation generators write a `long` table to stdout. Loader: `src/translate.cc`.

Search path:

```text
./
./tab/
CTH_LIBDIR/tab/WIDTHxHEIGHT/
CTH_LIBDIR/tab/
--path DIR -> DIR/tab/
```

### `obj/`

No `obj/` directory is present in this snapshot, but `src/waves.cc` can load `.obj` line objects from:

```text
./
./obj/
CTH_LIBDIR/obj/
--path DIR -> DIR/obj/
```

Object format is text lines like:

```text
x1,y1,z1 - x2,y2,z2
```
