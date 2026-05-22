# cthugha-js Porting / Experiment Plan

This is a practical follow-up to `CTHUGHA_JS_COMPARISON.md`.  The goal is not
to rewrite the Linux port into the JS port, but to identify small, reversible
experiments that test why the JS visuals feel closer to DOS Cthugha.

## Guiding Principle

Prefer feature flags and side-by-side modes over replacing existing behavior.
The Linux port has useful extensions: PCX loading, object waves, X11/SVGA
backends, palette smoothing, and multiple buffers.  We should isolate
"DOS-faithful" behavior as selectable modes, not erase the Linux personality.

## Highest-Value Experiments

~~### 1. Native 320x200 Logical Buffer

JS defaults to `320x200`; Linux defaults to `160x100`.

Experiment:

- Add or test a `320x200` logical buffer mode.
- Keep nearest-neighbor output scaling.
- Compare simple wave/flame combinations before touching algorithms.

Why:

Feedback visuals are resolution-sensitive.  A flame kernel that looks right at
320x200 can feel chunky or too fast at 160x100.

Risks:

- More CPU.
- Some existing display assumptions may rely on current logical dimensions.
- Translation tables and object scale may need review.~~

~~### 2. JS-Style Immediate Palette Mode

JS applies the selected palette directly during texture expansion.  Linux
smooths toward selected palettes and also has transient palette modifications.

Experiment:

- Add an option to disable palette smoothing.
- Add optional palette cycling similar to JS `cyclePalette()`.
- Keep current smoothing as default until tested.

Why:

Classic Cthugha feel is strongly palette-driven.  Immediate palette changes and
cycling may explain part of the perceived DOS faithfulness.

Risks:

- PCX palette behavior must remain coherent.
- Flashlight and palette smoothing currently interact with current palette
  state; do not regress the PCX fixes.~~

### 3. Zero-Crossing Wave Audio Mode

JS aligns waveform data to a zero crossing before drawing.  Linux waves sample
`soundDevice->dataProc`, whose contents vary by `sound-process`.

Experiment:

- Add a sound-process mode or wave sampling helper that aligns raw time-domain
  samples to a positive-going zero crossing.
- Use it with the simple line/dot waves first.

Why:

Stable oscilloscope-style waveforms often depend more on phase alignment than
on geometry.  This could make the normal waves feel less jittery and closer to
the JS/DOS look.

Risks:

- Existing FFT-like or smoothed modes are useful; keep this selectable.

### 4. Clarify Translation vs Display

JS "display functions" mutate the framebuffer before wave drawing.  Linux
`display.cc` functions map the finished buffer to output after the simulation
frame is done.

Experiment:

- Audit which Linux `display.cc` effects should exist as `translate` tables
  instead.
- For the JS tab set, prefer importing them as translate tables when the effect
  should smear through feedback.

Why:

If an effect is only a final presentation transform, it will not feed back into
the flame/translation/wave loop.  That can look much less Cthugha-like.

### 5. Wire Overlay Slot

We have discussed running wire/object waves over normal waveform effects.

Experiment:

- Add an optional `waveOverlay` / `wireOverlay` CoreOption.
- Main frame path:

```text
flame()
translate()
wave()
wireOverlay()
smoothPalette()
swap
```

Why:

This keeps wire models inside the feedback loop while allowing classic
FFT/waveform drawing underneath.  It also matches the "draw wave last into the
indexed buffer" shape used by JS.

Risks:

- The current object-wave startup bookkeeping assumes one active wave function.
  Use generation counters or per-option lifecycle state before adding the
  overlay slot.

### 6. Reference Flame Implementations

JS flame effects are simple direct loops.  Linux flame effects are optimized
and sometimes combine translate and flame.

Experiment:

- Add one or two direct/reference flame implementations behind debug or
  comparison names.
- Compare them visually with the optimized equivalents.

Why:

This gives us a correctness target.  If the direct version feels more faithful,
we can tune the optimized version with confidence.

Risks:

- Slow at larger resolutions.
- Must not become the only implementation unless performance is acceptable.

## Specific Findings To Track

### JS Palette Blue-Channel Index

`external/cthugha-js/src/framebuffer.ts:165` uses:

```ts
const b = palette[i + 3 + 2];
```

This looks like a typo for:

```ts
const b = palette[i * 3 + 2];
```

Do not copy this blindly.  If comparing visual captures, test whether fixing it
changes the JS look substantially.

### JS Flame Count

`FlameEffects.NUM_FLAMES` is `12`, but the switch includes `case 12` for
`flameWeird()`.  As written, random/next flame selection will not reach that
case.

### Tables Match Closely

The built-in color-table formulas in JS and Linux are very similar.  This is
probably not where major visual divergence comes from.

## Suggested Work Order

1. Capture a baseline matrix:
   - JS at default settings.
   - Linux current default.
   - Linux `320x200` if easy to configure or hack temporarily.
2. Add immediate palette/no-smoothing option.
3. Add zero-crossing time-domain wave sampling.
4. Import or compare JS tab translation behavior.
5. Add wire overlay slot once lifecycle state is made per-slot.
6. Only then consider deeper flame rewrites.

## Comparison Checklist

For any test clip, record:

- buffer size
- target FPS
- flame effect
- translation/display function
- wave effect
- color table
- palette
- palette smoothing on/off
- sound-process mode
- audio input file/segment

Without those, "looks more faithful" will be hard to reproduce.

