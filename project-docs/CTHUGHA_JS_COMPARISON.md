# cthugha-js Comparison Notes

This is a first-pass code comparison between this Linux port and
`delaneyparker/cthugha-js`.

The JS checkout inspected was:

```text
3a839fa Merge pull request #1 from zaphus/email
```

The JS project describes itself as a TypeScript/PIXI.js port of Cthugha V5.3
for DOS from 1994.  That matters: this Linux tree is also descended from old
Cthugha code, but it has accumulated Unix display abstractions, buffer swaps,
palette smoothing, multiple display backends, object loading, and other
platform-era choices.

## Executive Summary

The JS port is visually interesting because it has a very direct frame loop:

```text
optional boom boxes
optional palette cycling
flameEffects.update()
framebuffer.translateScreen()
waveEffects.update()
framebuffer.updateTexture()
```

See `cthugha-js/src/app.ts:40`.

The Linux port's core loop is:

```text
soundProcess()
flashlight()
border setup
flame()
translate()
wave()
smoothPalette()
swap active/passive
```

See `src/CthughaBuffer.cc:146`.

The ordering is surprisingly similar in the important part: flame/translation
first, then wave drawing over the processed buffer.  The big differences are
not mostly stage order.  They are:

- JS uses a single `320x200` indexed framebuffer by default.
- Linux uses a `160x100` logical buffer by default, then display code maps it
  to the actual device. A variety of buffer sizes are available at the command line
- JS implements flames and translation as direct readable pixel operations on
  one buffer.
- Linux uses an active/passive buffer pair, optimized pointer loops, and some
  combined flame+translate paths.
- JS palettes are immediate and optionally cycled.
- Linux palettes are selected/smoothed through `CthughaBuffer`, then expanded
  by the display backend.
- JS wave audio is a browser time-domain buffer aligned to a zero crossing.
- Linux wave audio comes from `soundDevice->dataProc`, whose contents depend on
  the selected `sound-process` mode.

Working hypothesis is the JS port feels faithful because it keeps the display model 
close to the DOS mental model: one 320x200 indexed image, simple palette lookup, 
direct pixel operations, and wave marks drawn into that image at the end of the 
frame. The Linux port is more capable, but the abstraction layers make it easier for
small timing, size, and palette differences to compound.

## Frame Pipeline

### JS

`cthugha-js/src/app.ts:40` runs the visual frame:

```text
flameEffects.update()
framebuffer.translateScreen()
waveEffects.update()
framebuffer.updateTexture()
```

`updateTexture()` expands indexed pixels through the current palette into RGBA
canvas pixels.  There is no separate display backend doing a later screen
mapping step.

### Linux

`src/CthughaBuffer.cc:146` runs the indexed-buffer frame:

```text
soundProcess()
flashlight()
border setup
flame()
translate()
wave()
smoothPalette()
swap active/passive
```

The display backend later reads `passive_buffer` and applies the selected
screen/display mapping.  In X11, the relevant display pass is in
`src/CthughaDisplayX11.cc`.

### Important Difference

The JS function named `translateScreen()` is closer to Linux `translate()` than
to Linux `display.cc` screen functions.  In JS, the "display functions" in
`Framebuffer` mutate the live framebuffer before waves are drawn.  In Linux,
`display.cc` screen functions map `passive_buffer` to device output after the
simulation buffer has already been produced.

This naming mismatch can hide a real behavioral difference:

- JS tab/display transforms participate in feedback immediately.
- Linux display transforms are usually presentation-only, unless the equivalent
  operation exists as a `translate` table.

In Cthugha terms, "participate in feedback" means "the transformed pixels are
the pixels that the next frame's flame/convolution step will read."  That is a
big deal because most of the visual texture comes from repeatedly processing
the previous frame.

In the JS port, `Framebuffer.applyTab()` builds a new `pixels` array from the
old one:

```ts
newPixels[index] = this.pixels[tab[index]];
this.pixels = newPixels;
```

That happens before `waveEffects.update()`.  The frame then ends with this
mutated array still being the canonical framebuffer.  On the next tick,
`flameEffects.update()` reads that already-shifted/warped image.  So if a tab
twists the image, the flame effect on the next frame smears the twisted image,
then another tab transform is applied, then waves are drawn into the result.
The transform is part of the simulation.

In the Linux port, the `translate` option has that same kind of role.  It reads
from one simulation buffer and writes into the other before `wave()` draws.
Those translated pixels are then swapped into place at the end of
`CthughaBuffer::run()`, so they feed the next frame.

By contrast, the functions in `src/display.cc` are later output mappings.  They
choose how `passive_buffer` is copied into `cthughaDisplay->buffer` for this
screen refresh.  For example, `screen_up()` maps each displayed row to a row in
`passive_buffer`; other screen functions mirror, split, roll, or bend the
image on the way to the display buffer.  But unless a similar transform is also
performed by `translate`, the simulation buffer itself has not been changed by
that display mapping.  The next frame's flame step still reads the un-displayed
simulation buffer, not the visually mirrored/bent/rolled output.

The practical difference:

```text
feedback transform:
  old simulation buffer -> transform -> wave draw -> next simulation buffer
  The transform accumulates and smears over time.

presentation transform:
  simulation buffer -> temporary display image
  The transform is visible this frame, but does not feed the next frame.
```

This is why a visually similar operation can feel very different depending on
where it lives.  A rotate/spiral/kaleidoscope transform in the feedback path
creates evolving trails and recursive structure.  The same transform applied
only at display time looks more like a camera or screen effect over a normal
Cthugha buffer.

## Buffer Model

### JS

`Framebuffer` owns one `Uint8Array` called `pixels`.

- `flameEffects` read and write through `getPixel()` / `setPixel()`.
- `applyTab()` builds a new pixel array from old pixel positions.
- `updateTexture()` maps each indexed byte to RGBA.

Out-of-bounds reads return zero.  Out-of-bounds writes are ignored.

### Linux

`CthughaBuffer` owns `activeBuffer` and `passiveBuffer`.  The buffers have guard
rows above and below the visible region.  Flame functions often swap active and
passive internally with the `PTR` macro in `src/flames.cc:156`, then the main
loop swaps again at the end of `CthughaBuffer::run()`.

Some Linux flame functions also combine translate and flame in one pass.  For
example `flame_general_subtle_trans()` reads through a translate table and sets
`done_translate`, so `TranslateEntry::operator()()` skips the later translate
stage.  See `src/flames.cc:477` and `src/translate.cc:335`.

### Consequence

The JS path is easier to reason about: one old image becomes one new image, then
waves are drawn.  The Linux path is faster and more flexible, but harder to
audit.  A small difference in which buffer is source/destination for a given
effect can change smear direction and persistence.

## Resolution And Scale

JS defaults to:

```text
320x200 @ 30 fps
```

See `cthugha-js/src/cthugha.ts:7`.

Linux defaults to:

```text
160x100 logical buffer
```

See `src/CthughaBuffer.cc:12`.

This is probably one of the largest visual differences.  Cthugha's effects are
pixel art and feedback math; halving each dimension is not neutral.  It changes
line density, flame diffusion distance, waveform geometry, and the apparent
speed of translations.

## Flames

The JS flame code is direct and readable.  Example: `flameUpSlow()` sums left,
center, right, and below, shifts right by two, subtracts one, clamps to zero,
then writes one row upward.  See `cthugha-js/src/flameEffects.ts:136`.

Linux implements equivalent families with optimized pointer math and lookup
tables.  Example: `flame_upslow()` uses `divsub[]` and rolling sums.  See
`src/flames.cc:162`.

### Key Difference

JS currently implements a smaller and simpler flame set: `NUM_FLAMES` is 12,
but there is a `case 12` for `flameWeird()`, so that case is unreachable unless
`NUM_FLAMES` changes.  Linux exposes more flame variants, including general
subtle/slow and falling/down.

Linux also has translate-aware flame paths.  JS always runs flame first, then
translation.

## Waves And Audio

The JS wave effects are mostly the simple waveform family.  `WaveEffects`
samples browser analyser time-domain data, aligns each channel to a zero
crossing, then maps it into a width-sized audio buffer.  See
`cthugha-js/src/waveEffects.ts:95`.

Linux's normal waves use `prepareSoundData()` in `src/waves.cc:688`, which
samples `soundDevice->dataProc`.  That processed buffer can be raw, FFT-like,
or smoothed depending on the selected sound process.  Separate beat/fire data
comes from `SoundAnalyze`, which computes RMS amplitude, attack level, `fire`,
and smoothed intensity.  See `src/SoundAnalyze.cc:18`.

### Consequence

Even if two wave functions have similar geometry, they may be driven by
different audio data.  JS's zero-crossing time-domain waveform can look more
stable and oscilloscope-like.  Linux can look more processed or less phase
stable depending on `sound-process`.

## Color Tables

The JS `WaveEffects.initTables()` formulas match Linux `init_tables()` closely:

- absolute distance from 128
- inverse distance
- identity
- inverse
- random table
- other shaped tables

Compare:

- `external/cthugha-js/src/waveEffects.ts:130`
- `src/sound_tables.cc:12`

This part is quite faithful.

## Palettes

JS expands pixels through a selected palette in `Framebuffer.updateTexture()`.
Palette changes are immediate, and `cyclePalette()` can rotate palette entries
over time.

Linux has a richer palette lifecycle: selected palettes, transient PCX palette
changes, flashlight modification, palette smoothing, and backend palette
expansion.  We have already fixed the X11 one-frame palette lag, but the Linux
system is still more stateful than JS.

One notable JS issue: `Framebuffer.setPalette()` appears to index the blue
channel as:

```ts
const b = palette[i + 3 + 2];
```

That probably wants `palette[i * 3 + 2]`.  See
`cthugha-js/src/framebuffer.ts:161`.  If the live demo still looks
good with that code, either the palette arrays compensate accidentally, or the
bug produces a distinctive but non-original color treatment.

## Display / Translation Tables

JS `Framebuffer` has:

- `shiftUp()`
- `shiftDown()`
- `applyTab()` for generated tab arrays

See `cthugha-js/src/framebuffer.ts:69`.

Linux separates:

- `translate` tables, which mutate simulation buffers;
- `display.cc` screen functions, which map the finished buffer to output.

For Cthugha faithfulness, the simulation-buffer translation path matters more
than the final display mapping.  Effects that should smear through feedback
must happen before wave drawing and before the active/passive swap.

## Why JS May Look More Like DOS

Likely reasons:

1. JS runs a true `320x200` indexed framebuffer by default.
2. JS's main frame loop is simple and easy to audit.
3. JS wave drawing always happens after flame and translation in the same
   framebuffer.
4. JS palettes are immediate and can cycle.
5. JS audio waveforms are zero-crossing aligned time-domain data.
6. JS avoids several Linux-specific display/backend stages that can alter feel.

## Caveats

This comparison is code-level only.  I have not yet run both programs with the
same audio, palette, flame, wave, and translation table and compared captures.
That visual test would be the next step before making large changes.
