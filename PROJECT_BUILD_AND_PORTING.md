# Build and Porting Notes

## Build System

The project uses old autoconf/automake:

- `configure.in`
- generated `configure`
- generated `Makefile`, `src/Makefile`, `tab/Makefile`, `doc/Makefile`
- `Makefile.am` files in root, `src/`, `tab/`, and `doc/`

Root `Makefile.am` recurses into:

```text
src tab doc
```

The install rules copy runtime assets into `$(pkglibdir)` subdirectories:

```text
pcx/
map/
tab/
```

## Configure Options

Major feature toggles in `configure.in`:

- `--disable-svga`: skip SVGAlib console target.
- `--disable-xwin`: skip X11 target.
- `--disable-serv`: skip sound server.
- `--disable-gl`: skip OpenGL target.
- `--disable-tabtools`: skip `tabheader` and `tabinfo`.
- `--with-dsp=DEV` / `--without-dsp`: OSS DSP.
- `--with-cdrom=DEV` / `--without-cdrom`: CD-ROM ioctl support.
- `--with-mixer=DEV` / `--without-mixer`: OSS mixer.
- `--without-network`: disable networking and server.
- `--without-mpg123`: prefer `l3dec` over `mpg123`.
- `--with-arch=ARCH`: old CPU optimization selection.

## Current Generated Configuration

From current generated Makefiles/config files:

- Selected programs: `xcthugha`, `cthugha-server`, `tabheader`, `tabinfo`.
- No selected setuid programs.
- `cthugha` exists as an extra program but is not selected.
- `glcthugha` exists as an extra program but is not selected.
- `WITH_DSP`, `WITH_CDROM`, `WITH_MIXER`, and `WITH_NETWORK` are enabled in `config.h`.
- `MP3_PLAYER` is configured as `mpg123` at `/usr/local/bin/mpg123`.
- `USE_XPM` is enabled.
- Current `config.log` is from a Solaris 11/i386 configure run in 2009, not from this machine.

There are generated-substitution oddities:

- `PACKAGE` remains `@PACKAGE_NAME@`.
- `pkglibdir` is `$(libdir)/@PACKAGE_NAME@`.
- `CTH_LIBDIR` in generated `CXXFLAGS` is `"${pkglibdir}"`.

Those may be artifacts of old automake/autoconf macro usage and regenerated files.

## Verification Attempt

I ran a dry-run build:

```sh
make -n all
```

It failed before compilation:

```text
cd . && aclocal-1.9
/bin/sh: aclocal-1.9: command not found
make: *** [aclocal.m4] Error 127
```

Cause: generated Makefile dependency rules consider `configure.in` newer than `aclocal.m4`, so make tries to regenerate autotools files with the pinned old tool name `aclocal-1.9`.

I did not run `./configure`, `make`, or edit generated files.

## Native Dependencies

Core/common:

- C and C++ compiler
- make/autoconf/automake if regenerating build files
- curses or ncurses
- POSIX process/file APIs
- gzip at runtime for `.gz` assets

X11 frontend:

- X11
- Xt
- Xaw
- Xmu
- Xext
- optional Xpm
- optional MIT-SHM

SVGAlib frontend:

- `libvga`
- `libvgagl`
- Linux console access, historically via setuid root

OpenGL frontend:

- OpenGL
- GLU
- GLUT
- `GL_EXT_paletted_texture`
- optional `GL/xmesa.h`

Sound and media:

- OSS `/dev/dsp`
- OSS mixer ioctls
- Linux/SysV or Unix CD-ROM ioctls
- optional `mpg123` or `l3dec`
- optional `xmp`
- optional `fortune` for silence messages

Networking:

- sockets
- possible `xnet`, `socket`, `nsl` libraries depending on platform

## Historical Build Failure

`build_errors.txt` records a previous build attempt that reached linking `xcthugha` but failed with many multiple-definition errors such as:

```text
multiple definition of `ferror_unlocked'
multiple definition of `fclose0(_IO_FILE*&)'
multiple definition of `getline'
multiple definition of `vprintf'
```

It also warns:

```text
the use of `tmpnam' is dangerous, better use `mkstemp'
```

That log appears to come from a different environment and should be treated as evidence of modernization friction, not as the current machine's direct build output.

## Debugging Hooks

The graphical main loop has a built-in timing profiler in `src/initExitDisp.cc`. It is guarded by `PROF`; changing the local `#undef PROF` to `#define PROF` makes `run()` accumulate and print timing buckets for frame setup, sound read, sound analysis, auto-change, sound server, buffer rendering, display, and CD update.

Runtime verbosity is controlled with `--verbose` / `-v`. Existing code uses `printfv(...)`, `printfe(...)`, and `printfee(...)` for diagnostics, so adding temporary instrumentation usually means following those helpers rather than introducing a new logging path.

## Porting Strategy

### Phase 1: Freeze and Build One Target

Start with `xcthugha` or a new headless/test target.

Recommended approach:

- Avoid regenerating autotools until the intended toolchain is chosen.
- Either install era-appropriate autotools or replace the build with a small modern build file.
- Preserve the target source sets exactly, including wrapper files that include `options.cc` and `keys.cc`.
- Disable SVGAlib and OpenGL initially.
- Consider disabling CD, mixer, DSP, and network for a first compile-only pass.

### Phase 2: Make Platform Edges Replaceable

Replace platform interfaces one at a time:

- OSS DSP -> ALSA/PulseAudio/PipeWire/SDL audio through `SoundDevice`.
- X11/SVGAlib/GLUT -> SDL2/GLFW texture display through `DisplayDevice`.
- Shell/fifo decoder path -> library or explicit subprocess wrapper with safer arguments.
- setuid SVGAlib path -> remove or quarantine.

### Phase 3: Test Pure Domain Code

Before refactoring visuals, add tests around:

- palette loading;
- PCX metadata/loading;
- translation table loading and stretching;
- sound sample conversion;
- keymap parsing;
- option parsing;
- a few deterministic flame/wave transforms.

### Phase 4: Reduce Globals Gradually

The code's core behavior lives in global state. A safe reduction path is:

1. Introduce a context struct around existing globals without changing behavior.
2. Move one subsystem at a time into that context.
3. Keep the `CoreOption` user-facing model stable until tests cover visual changes.

## Build-System Gotchas To Preserve

- Do not compile `options.cc` directly for every target unless replacing the wrapper scheme deliberately.
- `xwin_options.cc`, `svga_options.cc`, `GL_options.cc`, and `serv_options.cc` produce different variants.
- `xwin_keys.cc`, `nonx_keys.cc`, and `GL_keys.cc` produce different key handling variants.
- `nonGL_stubs.cc` is needed for non-GL targets because shared code references GL option globals.
- `serv_stubs.cc` and `serv_display.cc` are needed because server code links shared option/interface pieces without full visual code.
- `default.keymap.str` is generated from `default.keymap`.
