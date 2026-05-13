# CthughaNix Project Summary

This repository is a C/C++ source snapshot of CthughaNix 1.5, a Linux/Unix continuation of Cthugha-L: an audio-seeded music visualizer descended from the DOS Cthugha 5.x codebase. The code is old-school Unix graphics software: a small core loop, many global subsystem singletons, frontends for X11/SVGAlib/OpenGL, OSS sound input, and asset-driven visual effects.

The most useful mental model is:

```text
sound input -> normalized 1024 stereo samples -> analysis
            -> per-buffer visual pipeline -> display mapping -> frontend copy
            -> key/interface/autochanger feedback into options
```

The central runtime loop is in `src/initExitDisp.cc`. Each frame calls, in order:

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

## Documentation Created

- `PROJECT_STRUCTURE.md` maps directories, generated artifacts, source groups, and target binaries.
- `PROJECT_RUNTIME_MAP.md` traces startup, frame flow, visual buffer flow, audio flow, display flow, and user input.
- `PROJECT_SEAMS_AND_RISKS.md` identifies extension seams, modernization seams, hidden couplings, and risky code paths.
- `PROJECT_BUILD_AND_PORTING.md` records the build system, dependencies, current generated configuration, and porting strategy.
- `PROJECT_VERIFICATION.md` lists the checks I ran and the cross-checks behind these findings.

## Project Shape

- `src/` contains the application source, roughly 25k lines across C++ modules plus a few generated/wrapper include files.
- `tab/` contains C/C++ translation-table generators and `.cmd` descriptors.
- `map/` contains 169 Fractint-style palette maps.
- `pcx/` contains 6 gzip-compressed PCX images.
- `doc/` contains the original Texinfo/manual/manpage documentation.
- `precompiled/` contains old 32-bit Linux ELF binaries and precompiled table generators.
- `Makefile.am`, `configure.in`, generated `configure`, generated `Makefile`s, and `config.*` reflect a mixed old/new autotools state.

## Main Binaries

- `xcthugha`: X11 visualizer. This is the main practical frontend in this snapshot.
- `cthugha`: SVGAlib console visualizer. Intended to install setuid root; high-risk today.
- `glcthugha`: OpenGL/GLUT visualizer with 3D display modes and multiple buffers.
- `cthugha-server`: ncurses sound server that broadcasts sound data to clients.
- `tabheader`, `tabinfo`: translation-table inspection/conversion tools.
- `tab/cmd_*`: table generator programs used through `.cmd` descriptors.

The current generated Makefiles select `xcthugha`, `cthugha-server`, `tabheader`, and `tabinfo`. SVGAlib and OpenGL are not selected in the current generated configuration.

## Architectural Center

The core abstraction is `CoreOption`. It is not just settings; it is the runtime registry for effect choices. A visual buffer has a set of core options:

- `flame`
- `palette`
- `pcx`
- `translate`
- `wave`
- `object`
- `flame-general`
- `wave-scale`
- `table`
- `border`
- `sound-process`
- `flashlight`

Each current option entry is invoked as a callable object during the frame. Some entries are compiled-in functions; others are loaded from files in `map/`, `pcx/`, `tab/`, and optional `obj/` directories.

## Highest-Value Seams

- Display frontend seam: `DisplayDevice` plus `CthughaDisplay` subclasses.
- Sound input seam: `SoundDevice` subclasses for OSS DSP, file/program, network, random, and forked playback.
- Effect registry seam: `CoreOptionEntry` and `CoreOptionEntryList`.
- Asset seams: `.map` palettes, `.pcx(.gz)` images, `.cmd` table descriptors, `.tab` binary translation tables, `.obj` line objects.
- Control seam: `Keymap` action registry and `Interface` screens.
- Build target seam: wrapper source files such as `xwin_options.cc`, `GL_options.cc`, `serv_options.cc`, `svga_options.cc` compile the same `options.cc` with different macros.

## Current State Notes

This directory is not a Git checkout, so there is no repository history available locally. A dry-run build (`make -n all`) did not reach compilation because the generated Makefile attempts to run `aclocal-1.9`, which is not installed here. I did not modify generated autotools files.

The code is portable in the 1990s Unix sense, but not in the modern sense. It depends on APIs and assumptions that are now fragile: OSS `/dev/dsp`, SVGAlib, Xaw/Xt, MIT-SHM, GLUT paletted textures, process/fifo-based decoders, setuid console graphics, and many global buffers.

