# Runtime Map

This map describes the active graphical runtime in the current source tree.
The canonical source is `src/Application.cc`.

## Startup Flow

Graphical startup enters at `src/main.cc`:

```text
main(argc, argv)
  Application app(argc, argv)
  if app.initialize()
    app.run()
  return app.exitStatus()
```

`Application::initialize()` performs this work, in order:

```text
drop elevated uid
buildStartupConfig(argc, argv)
configure LoggingRuntime
handle --help / --version before display initialization
emit configuration diagnostics
configure input, translation, wave, and palette option policies
configure FrameGeneratorRuntime geometry/transition/message settings
remove continuation ini
initialize OSS mixer runtime when live DSP input requested
initialize silence-message providers
start AudioIngest
load visual catalogs and policy-enabled image files
initialize border and flashlight support
create SceneRuntime and runtime command/config/persistence ports
apply effect policy and startup SceneConfig
apply startup presentation and audio-processing choices
initialize interface, key actions, and keymaps
open DisplaySystem through registered display-driver factories
initialize FrameGeneratorRuntime filterchain pipeline
initialize DefaultAudioFramePipeline and SceneChangeScheduler
install PlatformLifecycle hooks
```

Important sequencing details:

- Help and version output return before any graphical display work.
- `AudioIngest` starts before display creation.
- Visual catalogs are loaded after frame geometry is configured, because image
  placement and generated translation tables depend on the logical frame size.
- `SceneRuntime` is created after visual catalogs exist, so startup scene
  selections can bind concrete catalog entries.
- `DefaultAudioFramePipeline` is initialized after display creation because
  quiet-message policy uses display timing and frame-budget information.

## Main Loop

`Application::run()` owns the outer loop:

```text
while !closeRequested()
  DisplayRuntime::processEvents(InputQueue)
  CommandsInputRuntime::runCurrent(CommandContext)
  Application::runFrame(1)
  CommandsInputRuntime::runCurrent(CommandContext)
  FramePacer::paceFrameEnd(...)
```

Display events update input/resize/expose state. Interface/keymap handling runs
both before and after the frame, so commands can react to input and also reflect
state changed by the frame.

The pacing step happens after display presentation. `FramePacer` uses the
visual frame start timestamp and `display.max_fps`; `0` means no limit.

## One Visual Frame

`Application::runFrame(int doDisplay)` is the per-frame scheduler:

```text
CthughaDisplay::nextFrame()
AudioIngest::tick()
DefaultAudioFramePipeline::processFrame(AudioFrame)
SceneChangeScheduler::operator()(AudioMetrics)
SceneRuntime::snapshot()
FrameGeneratorRuntime::render(FrameGeneratorContext)
CthughaDisplay::present(IndexedFrame, FrameRenderContext)
PlatformLifecycle::serviceFrameBoundary()
```

`CthughaDisplay::nextFrame()` samples the display clock, publishes `now` and
`deltaT`, and updates FPS accounting before audio analysis and visual filters
run. That makes audio processing, automatic scene changes, and frame generation
observe one stable timestamp for the visual frame.

`AudioIngest::tick()` advances decoded history, services passthrough when the
passthrough is not callback-driven, and builds the current `AudioFrame` around
the presentation sample position.

`DefaultAudioFramePipeline::processFrame()` applies the selected audio
processing mode, analyzes frame metrics, and updates rolling `AcousticContext`.

`SceneChangeScheduler` reads the current `AudioMetrics` and acoustic context.
It can request scene changes through `SceneCommandTarget` when quiet, elapsed
time, or cumulative fire policy triggers.

`FrameGeneratorRuntime::render()` creates a `FrameGeneratorContext`, asks
`FrameGeneratorSceneBinding` to push pending scene settings and cues into the
filterchain, runs the filterchain over `FrameStore` storage, and returns the
published `IndexedFrame`.

When `doDisplay` is nonzero and the generated frame is valid,
`CthughaDisplay` composes presentation screens, overlays, viewport, palette,
and backend transfer through the active display runtime.

## Audio Runtime

### Strategy Selection

Audio setup flows through:

```text
AudioSettings::fromConfig()
AudioOutputConfig::fromConfig()
Environment::detect()
RuntimeFactory
PcmSourceFactory
```

`Environment::detect()` records OSS DSP readability/writability and compile-time
availability of Pulse-compatible output and miniaudio capture/playback.

For live line-in, automatic input priority is:

- on Apple, Windows, Android, and Emscripten: miniaudio capture, then OSS when
  available;
- on other platforms: OSS when available, then miniaudio capture.

For playback, automatic output priority is:

- on Apple, Windows, Android, and Emscripten: miniaudio first, then Pulse when
  compiled/available;
- on other platforms: Pulse first, then miniaudio;
- OSS output is only considered by automatic selection when miniaudio support
  is not compiled in and OSS output is available.

Explicit `--audio-output-driver=` tries only the requested real driver, then
falls back to `AudioNullOutput` if it cannot open. `--silent` and
`--audio-output-driver=null` use `AudioNullOutput` directly.

### Sources

`PcmSourceFactory` selects:

- line-in for `AIM_DSPIn`;
- random noise for `AIM_Random`;
- WAV for `--play` names ending in `.wav`;
- MP3 for `--play` names ending in `.mp3` when minimp3 is compiled in;
- raw headerless PCM for other non-empty `--play` names;
- no source for `AIM_None` or unsupported modes.

Raw PCM uses the configured sample format, channel count, and sample rate.

### Ingest And Passthrough

`AudioIngest` owns:

- active `AudioInput`;
- optional `AudioOutputDump`;
- `DecodedAudioHistory`;
- optional `AudioPassthrough`;
- current `AudioFrame`;
- input worker thread when enabled;
- visual clock fallback when no realtime passthrough clock exists.

The input chunk size is roughly one twentieth of the sample rate, clamped to
1024..8192 samples. The decode-ahead window starts at four chunks and expands
by output target delay when passthrough is active.

Realtime output backends can provide a presentation clock. miniaudio playback
uses callback drain and reports a presentation delay derived from the device
callback period. Pulse and OSS use their output timing paths. `AudioNullOutput`
is non-realtime, so a null passthrough does not pace playback by wall-clock
audio hardware.

File playback reports completion when input is finished and either passthrough
has drained or the visual clock has reached the decoded end. `Application`
turns completion into a close request through `RuntimeShutdown`.

## Scene Runtime

`SceneRuntime` owns:

- `Scene`;
- `SceneCommands` and `SceneCommandsTarget`;
- scene selection registry and preset catalog;
- visual catalog factory result;
- scene serializer.

Startup creates typed catalogs for wave objects, palettes, translations, and
images, then creates a visual catalog service over those catalogs. The scene
runtime registers visual selections so keymap, interface, config, and
automatic changes all mutate the same scene model.

`SceneRuntime::snapshot()` produces the per-frame immutable scene view used by
frame generation and display presentation.

## Frame Generation

`FrameGeneratorRuntime` owns the visual engine:

```text
FrameGeometry
FrameStore
FrameTransitionController
FrameGeneratorSceneBinding
FrameGeneratorPipeline
```

`FrameStore` owns active and passive indexed pixel allocations. Its
`FrameRenderTarget` exposes only the render-target operations needed by
filters: dimensions, pitch, active/passive pixels, hidden border rows, swaps,
and clears.

The default filterchain order from `FrameGeneratorSceneBinding` is:

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

`FrameGeneratorSceneBinding` observes `Scene` changes and pending scene cues.
Before each render it updates filter objects with current flame, translation,
wave, border, palette, flashlight, image-cue, and text-cue state.

`FrameCommitFilter` swaps active/passive buffers. `IndexedFrameFilter`
publishes the passive pixels, pitch, dimensions, and `FramePalette` into an
`IndexedFrame` for display.

## Display Runtime

`DisplaySystem` selects a `DisplayDriverFactory` from the registry. The
application registers the X11 factory when `CTH_XWIN` is compiled and the SDL3
factory when `CTH_SDL3` is compiled. A typical development build registers SDL3
alone; a compatibility build registers X11 alone. If both are compiled and the
runtime driver is `auto`, registration order currently prefers X11.

The SDL3 factory creates:

```text
DisplayDeviceSDL3
DisplayBackendSDL3
DisplayRuntime
SDL3 presentation coordinator
```

The X11 factory creates:

```text
DisplayDeviceX11
DisplayBackendX11
DisplayRuntime
CthughaDisplayX11
```

`DisplayRuntime::processEvents()` drains platform events into the input queue.
For SDL3 this means SDL events, close requests, key translation, and resize
tracking. For X11 this means Xt/X event dispatch, key-release translation,
expose/resize tracking, and optional panel callbacks.

`CthughaDisplay::present()` receives the generated `IndexedFrame`, applies the
frame palette to the display device, and calls the frontend-specific
coordinator. The coordinator then:

```text
set global/frame palette
compose presentation frame
choose viewport and border-clearing needs
collect interface/error/FPS overlays
transfer pixels through DisplayRuntime
```

X11 also prepares mapped draw memory and runs `postDraw()` around the runtime
transfer. SDL3 presents through renderer texture upload/copy/present in its
backend.

## Runtime Commands And Configuration

Live changes flow through `RuntimeCommand` values. Producers include keymap
actions, interface actions, frontend panel callbacks where available,
credits/input screens, automatic scene changes, file-playback completion, and
persistence commands.

`RuntimeChangeMediator` handles the command and delegates to:

- `SceneCommandTarget`;
- `RuntimeDisplayControls`;
- `RuntimeAudioControls`;
- `RuntimeAutoChangeControls`;
- `RuntimeEffectControls`;
- `RuntimePersistence`;
- `RuntimeShutdown`;
- optional palette metadata target.

`RuntimeConfigRegistry` gathers current config from contributors owned by the
scene serializer, display settings, audio-processing state, auto-change
settings, quiet-message option, and legacy config contributor. Persistence
uses this registry for normal save and stop-and-continue writes.
