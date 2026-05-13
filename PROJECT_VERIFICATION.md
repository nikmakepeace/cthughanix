# Verification Log

This file records what I checked while mapping the project.

## Repository and Inventory

Commands used:

```sh
pwd
ls
ls -a
rg --files
find . -maxdepth 2 -type d | sort
find . -maxdepth 2 -type f | sed 's#^./##' | awk -F/ '{count[$1]++} END {for (d in count) print d, count[d]}'
find map -maxdepth 1 -type f | wc -l
find pcx -maxdepth 1 -type f | wc -l
find tab -maxdepth 1 -type f | wc -l
```

Findings:

- Not a Git checkout: `git status --short` returns `fatal: not a git repository`.
- `map/` has 169 files.
- `pcx/` has 6 files.
- `tab/` has 23 files.
- `src/` has 106 top-level files in this snapshot.
- There are historical `CVS/` directories.
- There is a hidden `.DS_Store`.

## Existing Docs Read

Files read:

- `README`
- `INSTALL`
- `TODO`
- `ChangeLog`
- `COPYING`
- `cthugha.ini.eg`
- `cthugha-L.lsm`
- `doc/overview.texi`
- `doc/technical.texi`
- `doc/parameters.texi`
- `doc/configure.texi`
- `doc/mixed.texi`
- `doc/other.texi`

Cross-checks:

- Upstream docs identify the executable flavors: `cthugha`, `xcthugha`, `glcthugha`, and `cthugha-server`.
- Texinfo docs confirm palette, PCX, and translation-table formats.
- `TODO` confirms known risk areas: portability, big-endian, sound stability, network code, OpenGL build, X11 geometry, and old UI concerns.
- `ChangeLog` confirms major architecture changes such as C++ rewrite, load-on-demand translation tables, OpenGL support, keymap rewrite, and `--play` simplification.

## Build Files Checked

Files read:

- `Makefile.am`
- `src/Makefile.am`
- `tab/Makefile.am`
- `configure.in`
- generated `Makefile`
- generated `src/Makefile`
- `config.h`
- `config.log`
- `config.status`

Commands used:

```sh
rg -n "^(TARGETS|TARGETS_SUID|bin_PROGRAMS|noinst_PROGRAMS|EXTRA_PROGRAMS|SUBDIRS|pkglibdir|CXXFLAGS|X_LIBS|GL_LIBS|S_LIBS|N_LIBS)\b" Makefile src/Makefile tab/Makefile doc/Makefile
rg -n "#define (WITH_|HAVE_|USE_|MP3_|MOD_|CTH_)|CTH_LIBDIR|DEV_DSP|DEV_CDROM|DEV_MIXER" config.h
stat -f '%Sm %N' Makefile.am aclocal.m4 configure.in configure Makefile src/Makefile.am src/Makefile
make -n all
```

Findings:

- Current generated target list is `xcthugha cthugha-server tabheader tabinfo`.
- Dry-run build stops at missing `aclocal-1.9`.
- `configure.in` is newer than `aclocal.m4`, explaining why make wants to regenerate autotools metadata.
- Current `config.log` is from Solaris/i386 in 2009.

## Entrypoint and Runtime Tracing

Files read:

- `src/initExitDisp.cc`
- `src/serv_main.cc`
- `src/CthughaDisplay.*`
- `src/CthughaBuffer.*`
- `src/DisplayDevice.*`
- `src/DisplayDeviceX11.cc`
- `src/DisplayDeviceSvga.cc`
- `src/DisplayDeviceGL.cc`
- `src/CthughaDisplayX11.cc`
- `src/CthughaDisplaySVGA.cc`
- `src/CthughaDisplayGL.cc`

Commands used:

```sh
rg -n "\bmain\s*\(" src tab
rg -n "class |struct |enum " src --glob '*.{h,cc}'
```

Cross-checks:

- Main graphical loop found in `src/initExitDisp.cc`.
- Server loop found in `src/serv_main.cc`.
- Display frontend seam confirmed through per-target `cth_init()`, `newDisplayDevice()`, and `newCthughaDisplay()`.

## Audio and Visual Pipeline Tracing

Files read:

- `src/SoundDevice.*`
- `src/SoundDeviceDSP.cc`
- `src/SoundDeviceFile.cc`
- `src/SoundDeviceFork.cc`
- `src/SoundDeviceNet.cc`
- `src/SoundDeviceRandom.cc`
- `src/SoundServer.*`
- `src/SoundAnalyze.*`
- `src/SoundProcess.cc`
- `src/sound.cc`
- `src/Mixer.cc`
- `src/CDPlayer.*`
- `src/flames.*`
- `src/waves.*`
- `src/sound_tables.cc`
- `src/translate.*`
- `src/palettes.cc`
- `src/pcx.*`
- `src/display.cc`

Cross-checks:

- `SoundDevice::operator()()` always normalizes into 1024 signed 8-bit stereo samples.
- `CthughaBuffer::run()` applies sound processing, flashlight, border, flame, translate, wave, palette smoothing, then swaps buffers.
- `CthughaDisplayX11` and `CthughaDisplaySVGA` share the same classic 2D display flow.
- OpenGL path has separate screen/background/light/fly sequencing.

## Option, Config, UI, and Keymap Tracing

Files read:

- `src/options.cc`
- `src/IniFiles.cc`
- `src/Option.*`
- `src/CoreOption.*`
- `src/Interface.*`
- `src/InterfaceHelp.cc`
- `src/InterfaceCredits.cc`
- `src/InterfaceList.cc`
- `src/keymap.*`
- `src/keys.cc`
- `src/default.keymap`
- wrapper files `xwin_options.cc`, `svga_options.cc`, `GL_options.cc`, `serv_options.cc`, `xwin_keys.cc`, `nonx_keys.cc`

Cross-checks:

- `cthugha.ini.eg` and `doc/configure.texi` agree with `IniFiles.cc` on config entry style and search order.
- `default.keymap` agrees with `keymap.cc` action names.
- Wrapper files confirm macro-variant compilation by directly including `.cc` implementations.

## Asset Format Checks

Commands used:

```sh
file pcx/*.pcx.gz precompiled/* tab/* map/basic.map
sed -n '1,80p' tab/hurricane.cmd
sed -n '1,80p' tab/cmdRead.cmd
sed -n '1,120p' map/basic.map
```

Findings:

- PCX assets are gzip-compressed PCX files.
- Precompiled binaries are 32-bit i386 ELF executables.
- `.cmd` files match the format implemented by `TranslateEntry::loaderCmd`.
- `.map` palette files match Texinfo documentation and `read_palette()`.

## Risk Search

Commands used:

```sh
rg -n "TODO|FIXME|BUG|bug|not working|segmentation|WARNING|XXX|should not|wrong|unknown|not implemented|broken|disabled|panic|error" src tab doc TODO README INSTALL ChangeLog build_errors.txt cthugha.ini.eg
rg -n "popen|systemf|execv|fork|kill\(|tmpnam|mkfifo|shm|gethostbyname|sendto|recv|mmap|ioctl|seteuid|XShm|GL_EXT|glut|vga_|ncurses|curses" src tab configure.in
```

Findings:

- High-risk edges are external commands, old device ioctls, setuid SVGAlib, SysV shared memory, GLUT paletted textures, and network byte/sample handling.
- Upstream `TODO` agrees with several observed risk areas.

