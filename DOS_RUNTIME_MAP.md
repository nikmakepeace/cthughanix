# DOS Runtime Map

This map follows the executable described by `cthug53s/MAKEFILE`, with `cthug53s/CTHUGHA.C::main()` as the entrypoint.

## Startup Flow

```text
main(argc, argv)
  title()
    BIOS text mode 3
    start_screen()
    memory/version display

  probe/log VESA info into cthugha.dbg
  if ULTRASND exists, prefer GUS
  set default config file to CTHUGHA.INI

  set random initial flame/wave/display/palette/table
  cmdfiles("CTHUGHA.INI")
  parse command line with getopt()
  optionally require cpu_check() >= 386
  seed rand()

  initstuff()
    mouse_init()
    choose sound backend
    load optional quiet string file
    pcx_and_palettes()
    init_pete()
    init_translate()
    initialize wave color tables

  optional pause/delay
  set_display_mode(useVesa)
  normalize selected effect numbers
  change_wave(), change_flame(), change_display()
  FillLUTBuffer(curpal)
  maybe apply one [oldini] personal preset
  init_cd(starttrack)

  enter outer visualizer loop
```

## Audio Backend Selection

`initstuff()` chooses one backend based on command-line/INI state or auto-detection:

```text
OLDSB  -> SDK_init()
PAS    -> PAS_init()
GUS    -> GUS_init()
SBPRO  -> SB_init()
UNKNOWN/default:
          try GUS, then PAS, then SBPRO, then OLDSB
```

Each backend installs the same function pointers used by `AUDIO.C`. The drawing code does not know whether samples came from GUS, PAS, SBPro/SB16, the old SB SDK, or debug random noise.

## Outer Control Loop

The outer loop in `CTHUGHA.C` alternates between running frames and reacting to state changes:

1. When `current == 0`, it resets timers, clears hidden bottom rows, calls `flame_cro()`, and schedules PCX events.
2. When `current == 1`, it randomizes or applies a personal `[oldini]` preset: table, palette, flame, wave, display, translation, FFT flag, and related timing/noise values.
3. After a frame batch, it handles one action from `z_keypress()`.

The key handler can change effect families, toggle lock/stereo/FFT/translation, change sample/noise thresholds, enter CD/mixer UI, show help/stats, open effect option screens, or append the current setup to `CTHUGHA.INI` as an `[oldini]` line.

## Frame Loop

`FLAMES.C::flame_cro()` is the high-frequency runtime loop. It calculates a random frame countdown from `min_time` and `rand_time`, then runs until a keypress, timer expiry, quiet/peak trigger, or forced change.

Per frame:

```text
if translate is enabled:
  translate_screen()

if pcxtime is active:
  keep the PCX image visible
else:
  flame()

sound = get_stereo()

if sound is noisy:
  if allow_fft && use_fft:
    FFT(0)
  wave()
  check quiet-change and beat/peak triggers
else:
  count silence and eventually draw quiet text

if refresh wait enabled:
  wait_vsync()

display()
```

This ordering is important. The DOS wave is drawn after the flame/translation step, so new sound-reactive pixels are placed into the current frame after the prior image has already been diffused.

## Buffer Model

The DOS visual state is two fixed buffers:

```text
unsigned char xshadow[204][320]
unsigned char xbuff[204][320]
unsigned char *buff   = &xbuff[0][0]
unsigned char *shadow = &xshadow[0][0]
```

Only 320x200 is displayed. Rows 200-203 act as a hidden working/border area for flame effects.

`buff` is the normal working image. `shadow` is staging for transforms such as display mappings and translation. `DISPLAY.C::flip_screens()` swaps the pointers after translation.

## Audio Pipeline

The common format is:

```text
stereo[320][2]  # unsigned 8-bit left/right-ish samples
```

`AUDIO.C::get_stereo()`:

1. On first real use, calls `audio_firsttime()`.
2. Calls `audio_everytime(wait)` to fill `stereo`.
3. Scans each channel for min/max range.
4. Returns noisy if either channel range exceeds `minnoise`.
5. Sets `peaknoise` if range exceeds `peaklevel` and `peakframes > 0`.

Debug mode bypasses hardware and fills `stereo` with a random walk.

Backend sample handling is hardware-specific:

- GUS uses `UltraRecordData()` and converts signed samples with `^ 128`.
- SBPro/SB16 uses `dsp_read()` from `SB_DRIVE.C`.
- PAS and old SB SDK use their vendor APIs and DMA buffers.

## Display Pipeline

`DISPLAY.C::set_display_mode()` chooses:

- VESA 640x400 mode `0x100`, then sets scanline length to 320 and bank A to 1.
- Fallback VGA mode `0x13`, 320x200, 256 colors.

The display functions are:

```text
Upwards
Downwards
Hor. Split out
Hor. Split in
Kaleidescope
90deg rot. mirror
90deg rot. mirror 2
90deg Kaleidescope
```

`display_up()` copies directly to segment `A000`. The VESA path performs bank switching and copies the buffer in chunks. Other display functions first write a transformed image into `shadow`, then call `display_shadowup()`.

Palette writes are immediate VGA DAC writes through ports `0x3c6`, `0x3c8`, and `0x3c9`.

## Palettes, PCX, And Translation Tables

`pcx_and_palettes()` loads visual assets before the main display starts.

Palettes:

- Built-in palettes come from `MAPS.C`.
- External palettes are `*.map` files in the current directory.
- Format is 256 lines of `R G B`, read as 0-255 and shifted down to 6-bit VGA DAC values.

PCX:

- Up to 20 `*.pcx` files are loaded from the current directory.
- Image data and PCX palettes are stored in EMS/XMS virtual memory when available.
- The loader supports uncompressed or RLE 8-bit PCX, not `.gz`.

Translation:

- `TRANSLAT.C` loads one explicit `table_file` or scans `*.tab` in the current directory.
- Tables are fixed to the DOS buffer shape: 204 rows of 320 `unsigned int` mappings.
- The code splits storage into two 102-row arrays to avoid 64K block limits.
- Extra tables are stored in EMS/XMS handles.

Some comments still mention `PCX\*.pcx` and `MAP\*.map`, but the active DOS source scans `"*.pcx"` and `"*.map"` in the current directory.

## Configuration Flow

Config is split between `CTHUGHA.INI` and short command-line flags.

INI sections:

- `[general]`: sound card, source, VESA, PCX, stereo, FFT, translation, hardware overrides, palette flags, quiet string file.
- `[startup]`: initial wave/flame/display/palette, timing, CD track, quiet/beat thresholds.
- `[waves]`, `[flames]`, `[displays]`: enable/disable individual effect entries.
- `[oldini]`: saved full presets in the older underscore-separated numeric format.

`cmdfiles()` looks up `CTHUGHA.INI` with DOS `_searchenv(filename, "PATH", ...)`.

## Runtime Controls

Keyboard input is BIOS-polled by `ZORILKEY.C`. Important actions include:

- `Esc`: quit.
- Space or unknown keys: request a change.
- `w`, `f`, `p`, `t`, `d`: change wave, flame, palette, table, display.
- `s`: toggle FFT.
- `*`: toggle stereo.
- `!`: toggle translation.
- `l`: lock/unlock automatic changes.
- `c`: enter CD/mixer UI.
- `x`: trigger PCX image.
- `+`/`-`: sample rate up/down.
- `,`/`.`: noise threshold down/up.
- `F1`: help.
- `F2`/`F3`/`F4`: flame/wave/display enable screens.
- `F10`: stats.
- `Alt+A` through `Alt+X`: direct wave selection.

## Shutdown

On exit, the DOS code:

```text
BIOS text mode 3
free high-memory PCX/palette handles
close audio backend
optionally stop CD playback
terminate virtual memory heap
show credits and quit screen
```
