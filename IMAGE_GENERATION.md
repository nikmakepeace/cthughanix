# Image Generation Notes

This note describes how a visual frame is produced today. It replaces an older
sound-backend-oriented walkthrough that no longer matches the source tree.

## Per-Frame Shape

```text
CthughaDisplay::nextFrame()
audioFrameTick()
AudioVisualBridge::runFrame()
VisualPipeline::run()
CthughaDisplay::operator()()
CDPlayer::operator()()
```

Audio is exposed to the visual engine through `AudioFrame`:

- `audioFrameData()` returns the current raw 1024-sample stereo window.
- `audioFrameProcessedData()` returns the selected `sound-processing` output.
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

`VisualPipeline::run()` executes the current visual modules in order. The
default pipeline stages are:

```text
BufferFrameBeginStage
ImageStage
FlashlightStage
BorderStage
FlameStage
TranslateStage
WaveStage
BufferFrameEndStage
PaletteStage
```

In module form, this is currently:

```text
BufferFrameBeginModule
ImageStageModule
FlashlightVisualModule
BorderVisualModule
FlameStageModule
TranslateStageModule
WaveStageModule
BufferFrameEndModule
PaletteStageModule
```

Image, flame, translate, and wave are real object-executing stages now. The
image stage overlays the selected PCX when `VisualDirector` arms it once.
Before each frame, `VisualDirector` updates the stage bindings for the selected
PCX, per-buffer flames, translate providers, waves, and palette state. The
modules still bind the legacy `CthughaBuffer` globals before calling classic
effect code, but they no longer choose their own entries from `CoreOption`.

## Where Audio Affects Pixels

- `src/AudioProcessor.cc` creates processed sample data for waves/effects.
- `src/AudioAnalyzer.cc` computes amplitude, channel balance, and noisy/silent
  state.
- `src/AudioVisualBridge.cc` updates `AcousticContext`, which drives fire-level
  effects and `AutoChanger`.
- `src/Border.cc` can copy raw audio into the hidden border rows used by flame
  diffusion.
- `src/waves.cc` draws waveform and object effects from
  `audioFrameProcessedData()`.
- `src/Flashlight.cc` brightens palette entries from acoustic fire events.

The safest rule for new visual code: read audio through `AudioFrame`, not from a
concrete source or output object.
