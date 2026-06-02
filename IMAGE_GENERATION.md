# Image Generation Notes

This note describes how a visual frame is produced today. It replaces an older
sound-backend-oriented walkthrough that no longer matches the source tree.

## Per-Frame Shape

```text
CthughaDisplay::nextFrame()
audioFrameTick()
AudioVisualBridge::runFrame()
VideoFilterchain::run()
CthughaDisplay::present()
```

Audio is exposed to the visual engine through `AudioFrame`:

- `audioFrameRawData()` returns the current raw 1024-sample stereo window.
- `audioFrameProcessedWaveData()` returns the selected `sound-processing` output.
- `audioFrameCurrent()` returns the full frame object, including
  `centerSample`.

For file playback, `AudioRuntime` decodes PCM through `AudioInput`, stores it in
`AudioBuffer`, writes it through `AudioOutput`, and asks `AudioFrameBuilder` for
the audio window that should be audible at the current visual frame.

For live/random input, `AudioInputProcessor` maintains the rolling frame used by
the same facade.

## Visual Mutation

`AudioVisualBridge::runFrame()` performs sound processing, analysis, acoustic
state updates, and automatic option changes before visual mutation.

`VideoFilterchain::run()` executes the current video filters in order. The
default filterchain stages are:

```text
ImageStage
BorderStage
FlameStage
TranslateStage
WaveStage
TextStage
FrameCommitStage
PaletteStage
FlashlightStage
IndexedFrameStage
```

In filter form, this is currently:

```text
ImageFilter
BorderFilter
FlameFilter
TranslateFilter
WaveFilter
TextInjectionFilter
FrameCommitFilter
PaletteFilter
FlashlightFilter
IndexedFrameFilter
```

Image, flame, translate, and wave execute domain objects through the same
`VideoFrame`. The image stage overlays the selected `IndexedImage` when
`VideoDirector` arms it once. PCX and indexed PNG files are decoded into the
same image domain object; format-specific code does not run in the filterchain
stage. Before each frame, `VideoDirector` updates the stage bindings for the
selected image, flame, general-flame value, translation table, wave, border
mode, palette target, and flashlight mode.

## Bitmap Text Source

The app carries a runtime CP437 bitmap font for buffer-injected text stages:

- source font: `resources/font/BmPlus_IBM_VGA_9x14.otb`;
- converter: `tools/otb_to_bitmap_font.py`;
- generated runtime source: `src/DosVga9x14Font.cc`;
- runtime wrapper: `src/BitmapFont.h`.

The generated representation is 256 CP437 glyphs, 9 pixels wide by 14 pixels
high, with one `uint16_t` row mask per glyph row. `TextInjectionFilter` writes
wrapped text into the active indexed visual buffer before `FrameCommitStage`,
so subsequent frames can consume the committed text as real visual material.

## Where Audio Affects Pixels

- `src/AudioProcessor.cc` creates processed sample data for waves/effects.
- `src/AudioAnalyzer.cc` computes amplitude, channel balance, and noisy/silent
  state.
- `src/AudioVisualBridge.cc` updates `AcousticContext`, which drives
  cumulative-fire-level effects and `AutoChanger`.
- `src/AutoChanger.cc` reports quiet intervals to `VideoDirector`; the selected
  `SilenceMessage` text becomes a `SceneCueInjectText`, which `VideoDirector`
  observes to arm `TextInjectionFilter`.
- `src/Border.cc` can copy raw audio into the hidden border rows used by flame
  diffusion.
- `src/waves.cc` draws waveform and object effects from
  `audioFrameProcessedWaveData()`.
- `src/Flashlight.cc` brightens palette entries from acoustic fire events.

The safest rule for new visual code: read audio through `AudioFrame`, not from a
concrete source or output object.
