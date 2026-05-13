# DOS Project Structure

The original DOS source tree is deliberately flat. `cthug53s/MAKEFILE` is the best authority for what belongs to the executable, because DOS case-insensitivity hides some assumptions that are awkward on Unix-like filesystems.

## Top Level

```text
cthug53s/
|-- *.C, *.H              C source and headers
|-- CPUCHECK.ASM          CPU detection routine
|-- RECFILA.ASM           Media Vision PAS mixer helper
|-- *.INC                 MASM support includes
|-- *.LIB                 bundled third-party sound/memory libraries
|-- UNCRUN_F.OBJ          prebuilt uncrunch object
|-- *.TD                  source data for text screens
|-- MAKEFILE              Microsoft C / MASM DOS makefile
`-- FILE_ID.DIZ           BBS package description
```

## Active Build Inputs

`cthug53s/MAKEFILE` builds `cthugha.exe` from:

- `SRCS1`: `cthugha.c`, `modes.c`, `audio.c`, `initscrn.c`, `getopt.c`, `charset.c`, `pete.c`, `patch.c`, `cdrom.c`, `cdmaster.c`
- `SRCS2`: `zorilkey.c`, `flames.c`, `display.c`, `maps.c`, `mouse.c`, `audiogus.c`, `audiosbp.c`, `cdplyr.c`, `sb_drive.c`, `translat.c`
- `SRCS3`: `options.c`, `showp.c`, `hi.c`, `cmdfiles.c`, `vesa.c`, `audiopas.c`, `audiosdk.c`
- `ASMSRC`: `cpucheck.asm`, `recfila.asm`
- Extra object: `uncrun_f.obj`

The linked libraries are `ultra0LM.lib`, `dma.lib`, `mvllib.lib`, `mvhllib.lib`, `llibce.lib`, and a Creative Labs SBPro SDK library at `\sbpro\msc\lib\embed\sbcl.lib`.

## Entrypoint And Shared Globals

- `CTHUGHA.C`: `main()`, title screen, command-line parsing, INI loading, audio/display init, palette/PCX discovery, outer event loop, shutdown.
- `CTHUGHA.H`: fixed dimensions, VGA memory helpers, global declarations, audio buffer declarations.
- `EXTERN.H`: additional global declarations used by older code.
- `COMMON.H`, `PTYPES.H`: compatibility/types for imported code.

Key fixed dimensions:

```text
BUFF_WIDTH  = 320
BUFF_HEIGHT = 204
BUFF_SIZE   = 65280
BUFF_BOTTOM = 200
```

The extra 4 rows are working rows for effects; the visible DOS display path is 320x200.

## Configuration And UI Control

- `CMDFILES.C`, `CMDFILES.H`: Fractint-derived INI parser. Recognizes `[general]`, `[startup]`, `[waves]`, `[flames]`, `[displays]`, and `[oldini]`.
- `GETOPT.C`: short-option parser for command-line flags.
- `OPTIONS.C`, `OPTIONS.H`: flame/wave/display enable screens and `check_*_ok()` filtering.
- `ZORILKEY.C`, `ZORILKEY.H`: BIOS keyboard polling and action constants.
- `MOUSE.C`, `MOUSE.H`: interrupt `0x33` mouse detection/calls for CD/mixer UI.
- `INITSCRN.C`, `INITSCRN.H`: text-mode title, help, stats, CD/mixer, credits, memory display.

## Visual Effects

- `FLAMES.C`: flame decay/diffusion functions, silence messages, peak-trigger logic, and the frame loop `flame_cro()`.
- `MODES.C`: waveform drawing functions and the 24-entry wave table.
- `PETE.C`, `PETE.H`: Pete/fractal waves plus the custom FFT path used by `use_fft`.
- `DISPLAY.C`, `DISPLAY.H`: display mode selection and 8 display mapping functions.
- `TRANSLAT.C`, `TRANSLAT.H`: translation table loading and per-pixel remapping.
- `MAPS.C`, `MAPS.H`: built-in 256-color palettes.
- `SHOWP.C`, `SHOWP.H`: PCX loader adapted from TapirSoft code.
- `CHARSET.C`, `CHARSET.H`: bitmap font for in-visualizer text.

Effect counts in the built DOS code:

- 15 flames plus a terminating `<BAD>` entry.
- 24 wave functions plus a terminating `<BAD>` entry.
- 8 display functions plus a terminating `<BAD>` entry.
- 10 color lookup tables for wave drawing.

## Audio

- `AUDIO.C`, `AUDIO.H`: common audio facade. It owns `get_stereo()`, `get_levels()`, peak/noise detection, and function pointers filled in by hardware backends.
- `AUDIOGUS.C`, `AUDIOGUS.H`: Gravis UltraSound recording through the UltraSound library and `ULTRASND`.
- `AUDIOSBP.C`, `AUDIOSBP.H`: Sound Blaster Pro/SB16 path using the in-tree `SB_DRIVE.C`.
- `SB_DRIVE.C`, `SB_DRIVE.H`: low-level Sound Blaster DSP, mixer, IRQ, and DMA driver.
- `AUDIOPAS.C`: Pro Audio Spectrum path using Media Vision libraries.
- `AUDIOSDK.C`, `AUDIOSDK.H`: older Creative Labs SB SDK path.
- `ENVIRON.C`: older BLASTER parser, not used by the current makefile.
- `RECFILA.ASM`: Media Vision mixer helper used by PAS support.
- `DMA.H`, `DMA.LIB`: DMA library interface and binary library.

The common audio seam is a set of globals:

```c
init_audio
close_audio
audio_firsttime
audio_everytime
get_level
set_level
level_incr
set_input
```

## CD And Mixer

- `CDPLYR.C`: text-mode CD/mixer UI, VU bars, track control, source selection, mixer level changes.
- `CDROM.C`, `CDROM.H`: MSCDEX device request wrapper.
- `CDMASTER.C`, `CDMASTER.H`: audio table-of-contents helpers and track playback.
- `CDMIXRG.C`, `CDMIX.H`, `CDMIX.TD`: CD/mixer screen region data.

CD playback is separate from audio sampling. The CD player controls the drive through MSCDEX; audio still has to reach the selected sound-card input path.

## DOS Platform Support

- `VESA.C`, `VESA.H`: public-domain VESA BIOS helper, used for 640x400 banked mode.
- `HI.C`, `HI.H`: Microsoft virtual memory wrapper for EMS/XMS-backed PCX, palette, and translation storage.
- `PATCH.C`, `PATCH.H`: MSVC compatibility and delay calibration.
- `CPUCHECK.ASM`: 8086/186/286/386/486 detection. `main()` requires at least 386 unless CPU checking is disabled.

## Screen Data And Compression

The text UI screens are stored as generated C/header pairs and `.TD` source data:

- `BANNER.*`
- `STARTUP.*`
- `CREDITS.*`
- `CTHU_HLP.*`
- `STATS.*`
- `OPTS.*`
- `CDMIX.*`
- `INIT.TD`

`UNCRUNCH.H` declares the decompressor API, and the built executable links `UNCRUN_F.OBJ`.

## Historical Or Unused Files

These files are present but not used by the active makefile:

- `LUTS.C`: an alternate/generated `FillLUTBuffer()` implementation. The built definition is in `CTHUGHA.C`.
- `ENVIRON.C`: older Sound Blaster environment parser.
- `FACEPREP.C`: parser/generator-like code, not part of `cthugha.exe`.

They are still useful as provenance, but they are not runtime seams in the makefile-backed DOS program.
