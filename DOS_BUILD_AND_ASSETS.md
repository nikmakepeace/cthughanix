# DOS Build And Assets

## Original Build System

The DOS source uses `cthug53s/MAKEFILE`, written for Microsoft C/C++ 1.0 and MASM.

Compiler and linker settings:

```text
CC   = cl -nologo -AL -c -G3 -W3 -Zp /f- /O2zxb2 /J /Gys /Gf
ASM  = masm -ml -Mx -DMODELSIZE=4
LINK = link /STACK:4500 /F /EXEPACK /CPARM:1 /NODEF /PACKF /FARCALL
```

Important implications:

- `-AL` means large memory model.
- MASM is configured with `MODELSIZE=4`.
- The linker is a DOS segmented executable linker, not a modern COFF/ELF toolchain.
- The build assumes a case-insensitive filesystem.

## Output

The makefile builds:

```text
cthugha.exe
```

There are no separate libraries or test binaries in the DOS makefile.

## Binary Dependencies

The link step depends on:

```text
ultra0LM.lib
dma.lib
mvllib.lib
mvhllib.lib
llibce.lib
\sbpro\msc\lib\embed\sbcl.lib
uncrun_f.obj
```

Meanings by source usage:

- `ultra0LM.lib`: Gravis UltraSound API used by `AUDIOGUS.C`.
- `dma.lib`: DMA support used by the Sound Blaster driver.
- `mvllib.lib`, `mvhllib.lib`: Media Vision PAS/PCM/mixer support.
- `sbcl.lib`: Creative Labs SBPro SDK used by `AUDIOSDK.C`.
- `uncrun_f.obj`: decompressor for compressed text screens.

## Assembly Inputs

- `CPUCHECK.ASM`: detects CPU generation and lets `main()` reject pre-386 machines.
- `RECFILA.ASM`: Media Vision helper that turns PCM mixer channels off/on to avoid feedback.
- `MODEL.INC`, `MASM.INC`, `BINARY.INC`: MASM support includes.

## Runtime Hardware Expectations

The program expects some combination of:

- VGA mode 13h, or VESA 1.1-compatible 640x400 banked mode.
- A supported sound source: Gravis UltraSound, Pro Audio Spectrum, Sound Blaster Pro/SB16, or older SB SDK hardware.
- Optional MSCDEX CD-ROM support.
- Optional mouse driver through interrupt `0x33`.
- Optional EMS/XMS virtual memory for PCX and translation tables.

## Environment Variables

- `ULTRASND`: parsed by `AUDIOGUS.C`; if present, `main()` initially prefers GUS.
- `BLASTER`: parsed by `SB_DRIVE.C::sb_get_params()` for Sound Blaster port, IRQ, DMA, and high-DMA values.
- `PATH`: used by `CMDFILES.C::findpath()` to locate `CTHUGHA.INI`.

INI settings can override audio port/IRQ/DMA values before backend init.

## Command-Line Flags

`CTHUGHA.C` uses this option string:

```text
?DOGPXxLlMmCeidSsrT:aQ:V:v:R:f:w:p:t:q:b:B:c:
```

High-value flags include:

- `-G`, `-P`, `-S`, `-O`: select GUS, PAS, SBPro, or old SB.
- `-C`, `-L`, `-M`: select CD, line, or microphone input.
- `-c N`: start CD track `N` and select CD input.
- `-r`: enable vertical-refresh waiting.
- `-f`, `-w`, `-p`: initial flame, wave, palette.
- `-t FILE`: translation table file.
- `-q FILE`: quiet string file.
- `-T N`: minimum time before automatic change.
- `-R N`: random extra time before automatic change.
- `-v N`: VU bar sample rate.
- `-V N`: input volume.
- `-b N`, `-B N`: beat/peak frame count and level.
- `-e`, `-i`: disable external or internal palettes.
- `-s`, `-m`: stereo or mono.
- `-l`: lock automatic changes.
- `-d`: pause before entering graphics mode.
- `-D` or `-x`: debug/random-audio mode.
- `-X`: disable PCX files.

The flag set is compact and stateful; it maps directly into globals.
The option string includes an `a` character, but there is no `case 'a'` handler in `CTHUGHA.C`, so I have not treated `-a` as a supported runtime option.

## `CTHUGHA.INI`

The DOS INI parser recognizes these sections:

```text
[general]
[startup]
[waves]
[flames]
[displays]
[oldini]
```

Examples of supported settings:

- `[general]`: `soundcard`, `source`, `stereo`, `vesa`, `pcx`, `pause`, `locked`, `string_file`, `table_file`, `stop_cd`, `fft`, `translate`, `cpu_check`, `internal_pal`, `external_pal`, `input_vol`, `vu_rate`, audio port/IRQ/DMA overrides.
- `[startup]`: `wave`, `palette`, `flame`, `display`, `randtime`, `mintime`, `cdtrack`, `quiet`, `beatframes`, `beatlevel`.
- `[waves]`: `wave-a=yes/no`, etc.
- `[flames]`: `flame-a=yes/no`, etc.
- `[displays]`: `disp-a=yes/no`, etc.
- `[oldini]`: numeric full-state presets saved by the runtime `i` key.

## Runtime Asset Formats

### Palette Maps

External palette maps are `*.map` text files:

```text
R G B
R G B
...
```

The DOS loader expects 256 rows. Values are read as 0-255 and shifted right by 2 because the VGA DAC uses 6-bit channels.

Active search in the DOS source:

```text
*.map in current directory
```

### PCX Images

PCX files are loaded from:

```text
*.pcx in current directory
```

Limits and behavior:

- Maximum of 20 PCX files.
- 8-bit PCX image data.
- RLE support through `SHOWP.C`.
- PCX palette is converted to 6-bit DAC values.
- Stored in EMS/XMS virtual memory when available.
- No `.gz` support in the DOS code.

### Translation Tables

Translation tables are raw binary `.tab` files:

```text
204 rows * 320 unsigned int values
```

Each value is an index into the 320x204 `buff` array and must be below `BUFF_SIZE`.

The DOS code stores the active table in two half-height arrays:

```text
mapping1[102][320]
mapping2[102][320]
```

That split avoids single data blocks over 64K and helps the old compiler optimize the remap loop.

### Quiet Strings

If a quiet string file is configured, `initstuff()` loads replacement silence messages. Otherwise the built-in 20-string table in `FLAMES.C` is used.

## Current Snapshot Caveats

- The DOS source package itself does not include the runtime `CTHUGHA.INI`, `*.map`, `*.pcx`, or `*.tab` assets expected by the original executable.
- The Linux project root contains many `map/`, `pcx/`, and `tab/` assets, but those are arranged for the Linux port's loader paths.
- The DOS makefile cannot be treated as a modern `make` file without toolchain and case-sensitivity work.
- I did not attempt to build the DOS executable on this machine.
