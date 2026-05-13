# DOS Cthugha 5.3 Project Summary

`cthug53s/` is the original Cthugha 5.3 DOS source package: "Cthugha - Audio Seeded Image Processing", also described by `FILE_ID.DIZ` as "An Oscilliscope on Acid". The code is a 1993-1995 real-mode DOS program by Zaph, Digital Aasvogel Group, and Torps Productions, with later sound-card contributions and third-party code folded in.

The source builds one program, `cthugha.exe`, with Microsoft C/C++ 1.0-era tools, MASM, and several DOS audio support libraries. It is not a library-oriented project. It is a single, hardware-owning application with global state, fixed-size buffers, direct BIOS/VGA/VESA calls, interrupt/DMA sound drivers, and text-mode UI screens.

## Mental Model

The core runtime is:

```text
DOS startup/config
  -> sound-card backend selection
  -> palettes, PCX files, translation tables, text screens
  -> 320x204 indexed-color working buffer
  -> per-frame flame/translation/wave loop
  -> direct VGA/VESA copy to screen
  -> BIOS keyboard/mouse/CD/mixer controls feed back into globals
```

The hottest path is `cthug53s/FLAMES.C::flame_cro()`. Each frame:

```text
optional translate_screen()
flame() or held PCX image
get_stereo()
optional FFT()
wave()
quiet/beat/change checks
optional vertical-sync wait
display()
```

The project is much closer to demo-scene hardware programming than to a modern application framework. The major seams are C function-pointer tables and global variables, not interfaces or modules.

## Source Shape

- `cthug53s/` is flat: no subsystem directories.
- The tree has about 21k lines across C, headers, assembly, include files, and one DOS makefile.
- The active build uses 30 C files, 2 assembly files, external `.LIB` files, and `UNCRUN_F.OBJ`.
- Several files in the source package are historical, generated, or unused by the makefile.

## Architectural Center

The central state lives in:

- `cthug53s/CTHUGHA.C`: startup, global defaults, command-line handling, init/shutdown, palette loading, main control loop.
- `cthug53s/CTHUGHA.H`: fixed dimensions, hardware memory helpers, shared global declarations.
- `cthug53s/FLAMES.C`: the per-frame loop and flame effect table.
- `cthug53s/MODES.C`: wave drawing functions.
- `cthug53s/DISPLAY.C`: VGA/VESA screen copy and display mapping functions.
- `cthug53s/AUDIO.C`: common audio sampling/analysis wrapper.

The main extension mechanism is:

```c
typedef struct {
    void (*function)(void);
    char flag_when;
    char name[21];
} function_opt;
```

`FLAMES.C`, `MODES.C`, and `DISPLAY.C` each define one of these arrays. The current effect is stored as a raw function pointer.

## Important Subsystems

- Runtime and configuration: `CTHUGHA.C`, `CMDFILES.C`, `GETOPT.C`, `OPTIONS.C`, `ZORILKEY.C`.
- Effects: `FLAMES.C`, `MODES.C`, `PETE.C`, `TRANSLAT.C`, `DISPLAY.C`, `MAPS.C`, `SHOWP.C`.
- Audio hardware: `AUDIO.C`, `AUDIOGUS.C`, `AUDIOSBP.C`, `SB_DRIVE.C`, `AUDIOPAS.C`, `AUDIOSDK.C`, `RECFILA.ASM`.
- CD and mixer UI: `CDPLYR.C`, `CDROM.C`, `CDMASTER.C`, `CDMIXRG.C`.
- DOS platform support: `VESA.C`, `HI.C`, `MOUSE.C`, `PATCH.C`, `CPUCHECK.ASM`.
- Text screens and compressed resources: `BANNER.*`, `STARTUP.*`, `CREDITS.*`, `CTHU_HLP.*`, `STATS.*`, `OPTS.*`, `CDMIX.*`, `UNCRUNCH.H`.

## What This Package Does Not Contain

This DOS source package does not include the same asset layout as the Linux tree. It expects runtime files such as `*.map`, `*.pcx`, `*.tab`, and `CTHUGHA.INI` to be near the executable or found by DOS `PATH` rules. The Linux port later grew explicit `map/`, `pcx/`, and `tab/` directories.

## Recommended Reading Order

1. `cthug53s/CTHUGHA.H` for the fixed buffer and shared globals.
2. `cthug53s/MAKEFILE` for what is actually built.
3. `cthug53s/CTHUGHA.C` for startup, command line, init, and outer loop.
4. `cthug53s/FLAMES.C` for the frame scheduler.
5. `cthug53s/AUDIO.C` plus one backend, usually `AUDIOSBP.C`/`SB_DRIVE.C`.
6. `cthug53s/DISPLAY.C` for the screen copy model.
7. `cthug53s/TRANSLAT.C`, `SHOWP.C`, and `MAPS.C` for external visual assets.

## Companion Documents

- `DOS_PROJECT_STRUCTURE.md`: file-by-file source map.
- `DOS_RUNTIME_MAP.md`: startup, frame, audio, display, config, and shutdown flow.
- `DOS_SEAMS_AND_RISKS.md`: extension points, hidden couplings, and dangerous code paths.
- `DOS_BUILD_AND_ASSETS.md`: build system, external libraries, runtime files, and command-line/config inputs.
- `DOS_VERIFICATION.md`: checks used to back these notes.
- `DOS_LINUX_DIFFERENCES.md`: significant approach differences between this DOS code and the Linux port.
