# DOS Seams And Risks

This document names the useful seams in the DOS source, then calls out the parts that are risky to change or hard to port.

## Useful Seams

### Effect Function Tables

The cleanest extension point is the trio of `function_opt` arrays:

- `FLAMES.C::flamearray`
- `MODES.C::wavearray`
- `DISPLAY.C::disparray`

Each entry has a function pointer, an enabled/disabled flag, and a display name. `OPTIONS.C` uses the same arrays to build the runtime toggle screens.

Adding a new DOS effect usually means:

1. Add the function.
2. Add it to the corresponding array before `<BAD>`.
3. Ensure it respects `BUFF_WIDTH`, `BUFF_HEIGHT`, `BUFF_SIZE`, and `buff`/`shadow`.

### Audio Backend Function Pointers

`AUDIO.C` is a facade over hardware backends. A backend installs:

- `audio_firsttime()`
- `audio_everytime(wait)`
- `close_audio()`
- mixer get/set/increment functions
- input-source selection

This made GUS, PAS, SBPro/SB16, and old SB SDK support coexist without changing the visual code.

### INI Sections

`CMDFILES.C` dispatches sections through a small `joblist` table. It is a seam for adding config categories, although the parser is old and assumes short lowercase variable names.

### Asset Loading

External visual assets are deliberate seams:

- `*.map` palettes.
- `*.pcx` image flashes.
- `*.tab` translation maps.
- quiet string files.

These are more flexible than the compiled effects, but the DOS implementation is fixed-size and current-directory oriented.

### Display Mode Boundary

`DISPLAY.C::set_display_mode()`, `display_up()`, and `display_shadowup()` are the boundary between visual buffers and hardware. Any platform port has to replace this boundary first.

## Hidden Couplings

### Fixed Buffer Geometry

Much of the code assumes 320-wide buffers and 200 visible rows:

```text
BUFF_WIDTH  = 320
BUFF_HEIGHT = 204
BUFF_BOTTOM = 200
BUFF_SIZE   = 65280
```

These constants are embedded in loops, display transforms, translation-table formats, audio sample windows, and VESA copies. Changing the dimensions is not a local edit.

### `buff` And `shadow` Are Mutable Pointers

The program often swaps `buff` and `shadow` rather than copying. A function may be reading "the screen" from one pointer before a call and from the other pointer afterward. `translate_screen()` and display transforms are especially pointer-sensitive.

### Audio Is Both Data And Timing

`get_stereo()` not only returns samples; it also controls noisy/quiet state, peak detection, waiting behavior, and sample-rate updates. Visual behavior and timing are coupled to backend read behavior.

### Effect Selection Is Numeric

Command-line flags, INI files, and `[oldini]` presets store effects as letters or numeric positions. Reordering entries in the arrays changes saved setups.

### Text UI Writes To Video Memory

The CD/mixer UI and screen helpers write directly to text segment `0xB800` or use BIOS text mode. These are not separate from runtime state: entering CD UI changes video mode and then returns via `set_display_mode(useVesa)`.

## Porting Risks

### Direct Hardware Access

The DOS code directly uses:

- BIOS interrupt `0x10` for video.
- BIOS keyboard services.
- Mouse interrupt `0x33`.
- VGA DAC/status ports.
- Segment `A000` and `B800` memory.
- Sound Blaster DSP/mixer ports.
- IRQ vector install/restore.
- ISA DMA setup.
- MSCDEX device requests.

This is the largest difference from normal Unix or modern OS code. The visual math ports more easily than the runtime shell around it.

### Real-Mode Memory Constraints

The code was written around 16-bit memory rules:

- Microsoft large model.
- Far pointers and `FP_SEG`/`FP_OFF`.
- Avoiding arrays over 64K.
- DMA buffers that must not cross a 64K physical page.
- EMS/XMS virtual memory through `_vheapinit`, `_vmalloc`, `_vlock`, `_vunlock`.

Translation tables and PCX data are shaped around those constraints.

### Old Compiler Assumptions

The source relies on behavior tolerated by old DOS compilers:

- K&R-style implicit `int` in imported CD code.
- Mixed declarations for `close_audio` (`int` in one place, `void` in headers/backends).
- DOS case-insensitive file lookup.
- Non-standard headers such as `<dos.h>`, `<conio.h>`, `<bios.h>`, and `<vmemory.h>`.
- Inline assembly in C and MASM modules.

### External Binary Dependencies

The makefile needs old binary libraries and one library at an absolute path:

```text
ultra0LM.lib
dma.lib
mvllib.lib
mvhllib.lib
llibce.lib
\sbpro\msc\lib\embed\sbcl.lib
uncrun_f.obj
```

Even with source changes, the original DOS build depends on reconstructing that toolchain.

### Asset Loader Limits

- PCX loading is capped at 20 images.
- `*.pcx`, `*.map`, and `*.tab` scanning is current-directory based in the active source.
- PCX and palette data use 6-bit VGA DAC values.
- Translation tables are fixed to the DOS 320x204 layout and raw binary integer storage.
- `.gz`, search paths, variable-size tables, and command-generated tables are Linux-port features, not DOS features.

### Error Handling And Cleanup

Several loaders return early on malformed files without full cleanup. Some hardware init paths print partial diagnostics or return inconsistent codes. That is understandable for a 1990s DOS app, but it makes modern automation and robust recovery difficult.

## Concrete Suspicious Spots

- `SB_DRIVE.C::sb_get_params()` applies `audio_dma16` only under an `if (audio_port != 0)` condition, which looks like a typo for checking `audio_dma16`.
- `AUDIO.C` defines `close_audio` with an `int` return type while `AUDIO.H` and backends treat it as `void`.
- `LUTS.C` defines another `FillLUTBuffer()` but is not part of the makefile; use the `CTHUGHA.C` definition when reasoning about the built program.
- `ENVIRON.C` parses `BLASTER` into `SbIOaddr`, `SbIRQ`, and `SbDMAchan`, but the active Sound Blaster path uses `SB_DRIVE.C::sb_get_params()`.
- Comments in `CTHUGHA.C::pcx_and_palettes()` mention `PCX\*.pcx` and `MAP\*.map`, but the active `findfirst()` calls use `"*.pcx"` and `"*.map"`.
- `CTHUGHA.C::pcx_and_palettes()` shifts only the first 255 bytes of each built-in palette even though `LUTSIZE` is 768 bytes. If that is not intentional, only the first 85 RGB triplets are converted to 6-bit DAC range.

## Safest Modernization Order

1. Preserve effect math but isolate it from DOS globals behind an explicit buffer/audio context.
2. Replace display copy first: `DISPLAY.C` is the hard OS boundary for visuals.
3. Replace audio backends with one normalized sample provider.
4. Make buffer size explicit only after effects are isolated.
5. Convert config and asset loading after the runtime is testable.
6. Treat CD/mixer UI as optional, because it is tightly tied to DOS hardware and text mode.
