# Seams and Risks

## Best Extension Seams

### Add a Palette

Drop a `.map` file into `map/`, the current directory, installed `CTH_LIBDIR/map/`, or `--path DIR/map/`.

Format is 256 RGB rows, compatible with Fractint-style maps:

```text
R G B optional comment
```

Loader: `src/palettes.cc`.

This is the lowest-risk extension seam.

### Add a PCX Image

Drop `.pcx` or `.pcx.gz` into `pcx/`, current directory, installed `CTH_LIBDIR/pcx/`, or `--path DIR/pcx/`.

Loader: `src/pcx.cc`.

PCX images become entries in the `pcx` CoreOption, and their palettes are added to the palette registry.

### Add a Translation Effect Without Recompiling

Add a `.cmd` descriptor and a generator program under `tab/`.

Descriptor format:

```text
cmdtab
Human-readable description
command %d %d [args]
```

The command receives `BUFF_WIDTH` and `BUFF_HEIGHT` and writes one `long` source-pixel index per destination pixel to stdout. `src/translate.cc` supports load-on-demand so large tables do not need to be generated at startup.

This is the best non-code visual effect seam.

### Add a Precomputed `.tab`

`.tab` files store a `tab_header` followed by table entries. `tabheader` and `tabinfo` exist to add/inspect headers.

Loader: `TranslateEntry::loaderTab` in `src/translate.cc`.

If table dimensions do not match the active buffer, stretching can be enabled.

### Add a 3D Line Object

The code supports external `.obj` line objects, though this snapshot does not include an `obj/` directory.

Format:

```text
x1,y1,z1 - x2,y2,z2
```

Loader: `read_object()` in `src/waves.cc`.

### Add a Compiled-In 2D Flame

Add a function to `src/flames.cc`, then add a `new FlameEntry(...)` in `_flames`.

Contract:

- read/write `active_buffer` and `passive_buffer`;
- respect `BUFF_WIDTH`, `BUFF_HEIGHT`, and the extra 3-line top/bottom border;
- return through `CoreOptionEntry::operator()()`.

### Add a Compiled-In Wave

Add a function to `src/waves.cc`, then add a `new WaveEntry(...)` in `_waves`.

Wave functions usually read `soundDevice->dataProc`, `soundAnalyze`, `waveScale`, and `table`, then draw directly into `active_buffer`.

### Add a Sound Processor

Add a `CoreOptionEntry` subclass in `src/SoundProcess.cc` and register it in `_soundProcessEntries`.

Contract:

- read `soundDevice->data`;
- write all of `soundDevice->dataProc`;
- keep the output as signed 8-bit stereo sample pairs.

### Add a Display Mode

2D: add a screen function in `src/display.cc` and register it in `_screens`.

GL: add a screen function in `src/GL_display.cc` and register it in that file's `_screens`.

2D `ScreenEntry` also declares an `xy size` that tells `CthughaDisplayX11/SVGA` whether to mirror horizontally and/or vertically.

### Add a Sound Backend

Subclass `SoundDevice`, implement `read()` and optionally `update()`, then extend `SoundDevice::newSD()`.

Contract:

- raw samples go into `tmpData`;
- return the number of samples read, not bytes;
- set `soundFormat`, `soundChannels`, and sample rate consistently so `SoundDevice::convert()` can normalize.

### Add a Display Frontend

Implement a new `DisplayDevice` subclass and a matching `CthughaDisplay` subclass or reuse the X11/SVGA-style one.

Required seam points:

- `cth_init()`
- `newDisplayDevice()`
- `newCthughaDisplay()`
- display `mainLoop()`
- `preDraw()`, `postDraw()`, `clearBox()`, `copyBox()`, text drawing

This is a large seam because many globals (`disp_size`, `bypp`, `bytes_per_line`, `draw_mode`, `text_size`, `fontSize`) are shared.

## Hidden Couplings

### Global State Is the Real Architecture

Subsystems communicate mainly through globals:

- `soundDevice`, `soundAnalyze`, `soundServer`, `cdPlayer`, `autoChanger`
- `displayDevice`, `cthughaDisplay`
- `BUFF_WIDTH`, `BUFF_HEIGHT`
- `active_buffer`, `passive_buffer`
- `screen`, `light`, `background`, `fly`
- many `Option` singletons

This makes direct reuse hard. Most modules assume initialization order rather than checking dependencies.

### Build Wrappers Include `.cc` Files

Files such as `xwin_options.cc`, `svga_options.cc`, `GL_options.cc`, and `serv_options.cc` define a macro and include `options.cc`. Similarly, `xwin_keys.cc` and `nonx_keys.cc` include `keys.cc`.

This is intentional here, but it is surprising. Any build-system rewrite must preserve separate compile units for those wrappers, not simply compile every `.cc` file once.

### Buffer Size Affects Asset Semantics

`BUFF_WIDTH` and `BUFF_HEIGHT` affect:

- amount of sound read per frame;
- translation table dimensions;
- display mirror and zoom assumptions;
- GL texture dimensions;
- table generator output.

Changing buffer size is a system-wide operation.

### Translation Can Be Folded Into Flame

Some flame functions detect the current translation table and apply it internally, setting `done_translate`. Then `translate()` skips its own remap. This is a speed optimization and a hidden coupling between `flames.cc` and `translate.cc`.

### Display Functions May Self-Reject

Some screen functions return nonzero when the buffer aspect ratio is unsuitable. `CthughaDisplayX11/SVGA` calls `while (screen())` so the current display option advances until something works.

### Palette Handling Depends on Frontend Color Mode

`palettes.cc` maintains palette entries, but frontend display code decides whether to use true palette hardware, pseudo-colors, or expanded true-color lookup tables.

### Text Rendering Alters Palette/Copy Behavior

`DisplayDevice::textOnScreen`, `darkenPalette`, and `needsFullCopy` interact with palette updates and dirty copying. Text is not just an overlay; in 8-bit modes it changes palette strategy.

## Modernization Seams

### Build System

Best first move is to freeze a known target list and replace or quarantine autotools. A modern CMake/Meson build could map each old target source set directly, but it must respect wrapper include patterns.

### Sound

OSS `/dev/dsp`, OSS mixer, and CD-ROM ioctl code are the most obsolete runtime interfaces.

Modern sound replacement should target the `SoundDevice` seam first. A PulseAudio/PipeWire/ALSA backend can feed `tmpData` and reuse `SoundDevice::convert()` initially. A later pass can replace the global sound format model.

### Display

X11 is the most salvageable classic frontend. SVGAlib is likely archival. OpenGL depends on GLUT and paletted texture extensions that are now uncommon.

Modern display replacement should preserve the indexed 8-bit core buffer first, then render it as a texture through SDL2, GLFW, or a similar thin frontend.

### Effects

The effect code is mostly self-contained and should be preserved. The safest modernization path is:

1. Keep `CthughaBuffer`, `CoreOption`, `flames`, `waves`, `translate`, and palettes intact.
2. Replace only platform I/O at the edges.
3. Add tests around pure loaders and buffer transforms before deeper refactors.

### Configuration and UI

The `Option` and `CoreOption` model is workable but stringly typed. A modern UI should treat `CoreOption` entries as the existing domain model, then gradually wrap with safer metadata.

## Risk Register

### High Risk

- Setuid root SVGAlib path: `DisplayDeviceSvga.cc` regains root with `seteuid(0)` to call `vga_init()`.
- External command execution: `CoreOption::load()` uses `gzip -cd`, `SoundDeviceFile` uses `/bin/sh -c`, translation load-on-demand uses `/bin/sh -c`, silence messages can run `fortune`.
- Temporary fifo creation: `SoundDeviceFile` uses `tmpnam()` and `mkfifo()`.
- OSS and CD ioctl paths: Linux-specific, obsolete, and hard to test on modern systems.
- SysV shared memory and forked sound reader: `SoundDeviceFork` shares raw memory and mutable option state between processes.
- OpenGL paletted texture requirement: `DisplayDeviceGL.cc` has a compile-time `#error` if `GL_EXT_paletted_texture` is unavailable.

### Medium Risk

- Many fixed-size string buffers use `sprintf`, `strncpy`, and `strncat` with historical assumptions.
- `CoreOptionEntry` destructor uses `delete` for memory allocated with `new char[]`, which is undefined behavior in modern C++.
- Some code assumes little-endian behavior despite partial big-endian branches; TODO explicitly calls out endian concerns.
- Build-generated macros are odd in the current snapshot: generated files leave `PACKAGE` as `@PACKAGE_NAME@`, and `CTH_LIBDIR` uses `"${pkglibdir}"` in current generated config.
- The network server/client path has suspicious port byte-order handling and raw-size/sample-format coupling.
- `build_errors.txt` records linker multiple-definition failures from an older attempted build.

### Lower Risk

- Palette/map loading is simple and isolated.
- PCX loading is isolated, although format support is intentionally narrow.
- Keymap parsing is standalone and a good candidate for targeted tests.
- Translation table generators are separate command-line programs and can be tested independently.

## Places To Put Tests First

- `OptionInt`, `OptionTime`, `OptionOnOff` parsing.
- `CoreOption::hasExtension()`, `isCompressed()`, and loader behavior with temporary files.
- `.map` palette loader with short and full files.
- `.cmd` parser and generated command assembly.
- `.tab` header parser and stretch behavior.
- `SoundDevice::convert()` for all sample formats and endian branches.
- `Keymap::parseBinding()` with default keymap examples.
- Pure flame/wave functions against small fixed buffers after introducing a harness.

