# Seams And Risks

This file lists practical extension seams in the current codebase and the
couplings that still deserve caution.

## Low-Risk Extension Seams

### Add A Palette

Drop a `.map` file into `resources/map/`, the current directory, the installed
`CTH_LIBDIR/map/`, or `--path DIR/map/`.

The loader in `src/palettes.cc` accepts up to 256 RGB rows and optional
metadata before the color data:

```text
name: Human Name
set: classic bright
energy: low medium high extreme
```

`--palette-set SET` filters palettes whose metadata contains the requested set.
If no palette matches, the code warns and leaves palettes enabled.

### Add An Indexed Image

Drop uncompressed `.pcx` or indexed `.png` files into `resources/img/`, the
current directory, installed `CTH_LIBDIR/img/`, or `--path DIR/img/`.

Loaders:

- `src/pcx.cc`
- `src/png.cc`
- catalog path: `SceneImageCatalogLoader`

Images become scene visual choices. A scene image cue arms `ImageFilter` once;
the filter clips and writes the selected `IndexedImage` into active pixels and,
by default, mirrors it into passive pixels for immediate display.

### Add A Translation Effect

Add a `TranslateGenerator` and register it in `src/TranslateGenerator.cc`.
Translation catalogs are generated during visual startup, so the frame loop uses
ready `TranslationTable` data. `TranslateFilter` receives the selected table
from `FrameGeneratorSceneBinding`.

### Add A Line Object

External line objects live under `resources/obj/` and are loaded by
`src/waves.cc`.

Format:

```text
x1,y1,z1 - x2,y2,z2
```

### Add A Flame

Add the implementation in the flame domain and expose it through the flame
catalog/scene selection path. The executed stage is `FlameFilter`, which calls
the selected `Flame` against `FrameRenderTarget` and `FrameGeneratorContext`.

Contract:

- operate on active/passive indexed pixels through `FrameRenderTarget`;
- respect visible geometry, pitch, and hidden border rows;
- leave coordinate remapping to `TranslateFilter`;
- keep selection state in scene/catalog objects, not inside the filter.

### Add A Wave

Add the drawing function/catalog entry in the wave domain. `WaveFilter` receives
the selected `Wave`, `WaveConfig`, random source, and frame context from
`FrameGeneratorSceneBinding`.

Contract:

- read raw/processed audio from `FrameGeneratorContext`;
- use frame metrics and rolling acoustic values from the context;
- write active indexed pixels through `FrameRenderTarget`;
- keep per-wave persistent render state in `WaveState`/lookup tables owned by
  the filter.

### Add An Audio Processing Mode

Extend the audio-processing selector/processor path. The per-frame contract is
`AudioFrame.raw` in, `AudioFrame.processedWaveData` out, then metrics analysis
updates `AudioFrame.metrics` and `AcousticContext`.

Keep the output shape as the existing 1024 stereo sample-pair window expected
by visual code.

### Add A PCM Source

Implement a `PcmSource` subclass and extend `PcmSourceFactory`.

Contract:

- publish a valid `PcmFormat`;
- write raw PCM frames from `read()`;
- report finite input through `isFinished()` when applicable;
- leave output playback and visual timing to `AudioIngest`/`AudioPassthrough`.

### Add An Audio Output

Implement an `AudioOutput` subclass and extend `RuntimeFactory`.

Contract:

- implement `write()`, `isOpen()`, and realtime capability accurately;
- call into `AudioOutput::configureTiming()` before passthrough begins;
- provide a presentation delay/clock when the backend can support one;
- reject fake/null hardware backends for real-device smoke tests.

### Add A Frame Filter

Implement `FrameFilter`, add it through `FrameFilterchainFactory`, and decide
where `FrameGeneratorSceneBinding` should configure it.

Current concrete filters are:

- `ImageFilter`
- `BorderFilter`
- `FlameFilter`
- `TranslateFilter`
- `WaveFilter`
- `TextInjectionFilter`
- `FrameCommitFilter`
- `PaletteFilter`
- `FlashlightFilter`
- `IndexedFrameFilter`

Stage execution receives one `FrameFilterFrame` carrying the render target,
frame context, optional frame palette, indexed-frame publication slot, and log
sink. Filters should not discover global scene state on their own.

### Add A Display Frontend

Add a `DisplayDriverFactory` and register it with `DisplayDriverRegistry` during
application display startup.

The active driver must create:

```text
DisplayDevice
DisplayBackend
DisplayRuntime
CthughaDisplay or compatible coordinator
```

The display contract is an `IndexedFrame` plus palette, presented through
`DisplayRuntime`. A new frontend should preserve that contract and avoid
reintroducing direct visual-engine dependencies into event handling.

## High-Risk Areas

### X11 Is Still The Only Wired Graphical Frontend

The main target only registers the X11 display factory. `CTH_BUILD_SDL3` exists
as a CMake option and display-driver enum support exists in config parsing, but
there is no SDL3 source path in `src/CMakeLists.txt`.

### Display Globals Remain

The display and presentation layer still has process-wide values such as
`disp_size`, `bypp`, `bytes_per_line`, `draw_mode`, `text_size`, `fontSize`, and
the global `screen` option. These are most visible in X11, classic presentation
screens, and text rendering.

### Runtime Selection Is Explicit But Fallbacks Matter

`RuntimeFactory` makes audio strategy selection explicit, but output fallback
currently returns `AudioNullOutput` for failed explicit or automatic playback
selection. `AudioNullOutput` is non-realtime; that is correct for deliberate
silent/null output, but it is not a substitute for an opened real playback
device when diagnosing sync.

### Audio Timing Depends On Backend Semantics

Realtime backends can provide presentation clocks and delay estimates. Null
output does not. miniaudio callback playback, Pulse writes, and OSS writes have
different queue semantics, so latency fixes should be tested against trace logs
and at least one real backend.

### Scene Binding Owns Much Of The Visual Policy

`FrameGeneratorSceneBinding` is the current policy bridge from `Scene` to
filters. It handles pending scene changes, image cues, text cues, palette
transitions, and stage enablement. New visual policy should usually live there
or in scene/runtime command objects, not inside individual filters.

### Filter Order Is Semantically Significant

The default order is:

```text
Image, Border, Flame, Translate, Wave, Text,
FrameCommit, Palette, Flashlight, IndexedFrame
```

Moving stages changes visible behavior. For example, frame commit determines
which buffer becomes display-facing, palette and flashlight happen after pixel
commit, and indexed-frame publication must remain last.

### Startup Order Is A Contract

Several modules assume startup has happened in the order used by
`Application::initialize()`:

- frame geometry before visual catalog loading;
- visual catalogs before `SceneRuntime`;
- scene/runtime command ports before display opening;
- display opening before audio-frame pipeline quiet observer creation;
- platform lifecycle hooks after audio/display state exists.

### Asset Loaders Are Still Format-Specific

Palette, PCX, indexed PNG, translation, and line-object loading have narrow
format expectations. Validate new assets with focused tests or a trace-enabled
run before assuming the renderer is wrong.

### OSS Paths Are Legacy But Active

OSS DSP and mixer support still compile when headers are available and options
are enabled. The mixer path is only initialized for live DSP input. Be careful
when changing audio config, because the old Unix device assumptions are still
observable in runtime behavior.

## Practical Development Guidance

- Prefer adding tests around the seam you touch before changing artistic
  behavior.
- Use `RuntimeCommand` for live UI/key/panel mutations.
- Use `SceneCommandTarget` for scene selection changes.
- Use `FrameGeneratorContext` for frame-local audio/time/scene data.
- Use `FrameRenderTarget` instead of process-wide pixel aliases.
- Use `DisplaySystem` and `DisplayDriverFactory` for frontend work.
- Keep `xcthugha` green while adding portable paths.
