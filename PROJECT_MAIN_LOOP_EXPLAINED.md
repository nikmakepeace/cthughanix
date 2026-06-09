# Main Loop Explained

This is a guided walk through the active graphical main loop. Keep
`src/Application.cc`, `src/AudioIngest.cc`, `src/AudioFramePipeline.cc`,
`src/FrameGeneratorRuntime.cc`, `src/FrameGeneratorSceneBinding.cc`,
`src/FrameFilterchain.cc`, `src/CthughaDisplay.cc`, and
`src/CthughaDisplayX11.cc` open while reading.

## 1. Entry Point

`src/main.cc` is intentionally small:

```text
Application app(argc, argv)
if app.initialize()
  app.run()
return app.exitStatus()
```

The application object owns the runtime. There is no separate current server
startup path in the CMake target.

## 2. Startup In `Application::initialize()`

Startup begins by dropping elevated uid and building a typed startup config:

```text
buildStartupConfig(argc, argv)
loggingRuntimeValue.configure(...)
```

Help and version exits are handled immediately. That matters because the help
path can print usage without initializing X11.

After diagnostics and policy setup, the frame generator is configured with the
startup buffer size, palette smoothing settings, quiet-message duration, and
message provider config:

```text
FrameGeneratorRuntimeConfig config
frameGeneratorValue.configure(config)
```

Audio ingest starts before the display:

```text
audioIngestValue.reset(new AudioIngest(...))
audioIngestValue->start()
```

This builds source/output runtime objects through `RuntimeFactory`, creates
decoded history, and starts worker/callback paths where needed.

Visual catalogs are loaded next. The catalog loaders need final frame geometry,
because image loading, translation generation, and some visual policy depend on
logical indexed-frame dimensions.

`initSceneRuntime()` then creates the scene module and runtime command ports:

```text
SceneRuntime
RuntimeConfigRegistry
AudioProcessor / AudioProcessingState / AudioProcessingSelector
AutoChangeSettings / AutoChangeControls
RuntimePersistence
RuntimeShutdown
RuntimeDisplayControls
RuntimeAudioControls
RuntimeAutoChangeControls
RuntimeEffectControls
RuntimeChangeMediator
RoutedRuntimeCommandTargetRouter
```

Startup scene config is applied after those objects exist, and the interface
and keymaps are initialized before display creation so early display events can
route commands immediately.

Display startup creates a `DisplayDriverRegistry`, registers the X11 factory
when available, and opens `DisplaySystem`. The X11 factory constructs the
device, backend, display runtime, and presentation coordinator.

Finally the frame-generator pipeline and audio-frame pipeline are initialized,
then platform lifecycle hooks are installed.

## 3. Outer Loop In `Application::run()`

The outer loop has five active phases:

```text
DisplayRuntime::processEvents(inputQueue)
CommandsInputRuntime::runCurrent(commandContext)
runFrame(1)
CommandsInputRuntime::runCurrent(commandContext)
FramePacer::paceFrameEnd(...)
```

`CommandContext` contains the current `InterfaceRuntime`, the
`RuntimeChangeMediator`, and the routed command-target router. Interface and
keymap code should use those ports rather than reaching into subsystem state.

The loop exits when `RuntimeShutdown` reports a close request.

## 4. Frame Clock

`runFrame()` begins with:

```text
CthughaDisplay::nextFrame()
```

`nextFrame()` calls `FrameClock::beginFrame()`, stores the current frame time
and delta, and updates instantaneous and rolling FPS. Every later stage in this
visual frame receives that same display timestamp.

## 5. Audio Ingest Tick

The next call is:

```text
audioIngestValue->tick()
AudioFrame& audioFrame = audioIngestValue->currentFrame()
```

`AudioIngest::tick()` does three things:

1. If input is not worker-threaded, it decodes enough source data toward the
   current decode target.
2. If passthrough is not callback-driven, it services one output drain step.
3. It asks `AudioFrameBuilder` to build the current visual `AudioFrame` from
   `DecodedAudioHistory`.

The center sample comes from the passthrough presentation clock when a realtime
output provides one. Otherwise it comes from the ingest visual clock.

If file input is complete and output/visual drain is complete, the application
requests shutdown.

## 6. Audio Processing And Analysis

The audio side of the visual frame is:

```text
runAudioFramePipeline(audioFrame)
```

`DefaultAudioFramePipeline::processFrame()` applies the selected
`AudioProcessingSelector` mode to populate `processedWaveData`, analyzes the
frame with `AudioProcessor`, and updates `AcousticContext`.

The same `AudioFrame` now holds raw samples, processed samples, frame metrics,
sample count, and center sample.

## 7. Automatic Scene Changes

After audio analysis, `runAudioFramePipeline()` calls
`SceneChangeScheduler::operator()(audioFrame.metrics)`.

The scheduler can:

- observe long quiet periods and ask the frame generator to emit a text cue;
- change scene choices after an interrupted quiet period;
- change scene choices when cumulative fire crosses the configured threshold;
- change scene choices after the configured wait interval.

Scene changes go through `SceneCommandTarget`, so the scene model remains the
owner of selection state.

## 8. Frame Generator Context

`Application::runFrame()` snapshots the scene and calls:

```text
runFrameGenerator(audioFrame, sceneSnapshot)
```

The application builds:

```text
AudioAnalysisSnapshot
FrameGeneratorContext
```

The context carries borrowed pointers to raw audio, processed audio, metrics,
rolling acoustic values, scene snapshot, frame time, frame delta, and the
current frame-budget estimate.

## 9. Scene Binding

`FrameGeneratorRuntime::render()` ensures the filterchain exists, then calls:

```text
FrameGeneratorSceneBinding::configureFilterchain(...)
```

The binding watches scene changes and cues. When needed, it pushes the current
scene settings into typed filters:

- `FlameFilter`: flame implementation and general-flame flag.
- `TranslateFilter`: translation table.
- `WaveFilter`: wave renderer, wave config, and random source.
- `BorderFilter`: hidden-border mode.
- `FrameCommitFilter`: scene names for diagnostics.
- `PaletteFilter`: target palette and transition strategy.
- `FlashlightFilter`: enabled/disabled mode.
- `ImageFilter`: one-shot image cue and placement.
- `TextInjectionFilter`: text cue content, duration, and ink color.

## 10. Filterchain Execution

`FrameGeneratorPipeline::render()` calls:

```text
FrameFilterchain::run(FrameRenderTarget, FrameGeneratorContext)
```

One `FrameFilterFrame` wraps the active/passive render target, frame context,
optional frame palette, published-indexed-frame slot, and log sink.

The default stages run in this order:

```text
Image -> Border -> Flame -> Translate -> Wave -> Text
      -> FrameCommit -> Palette -> Flashlight -> IndexedFrame
```

Disabled stages are skipped. Armed-once stages run once and then disarm. The
final `IndexedFrameFilter` publishes passive pixels, geometry, pitch, and
palette for display.

## 11. Presentation

After frame generation, the application builds a `FrameRenderContext` for
display overlays and calls:

```text
CthughaDisplay::present(indexedFrame, presentationContext)
```

`CthughaDisplay` records the source frame, applies the frame palette to the
display device, and dispatches to the frontend coordinator.

`CthughaDisplayX11::operator()()` performs the current X11 presentation:

```text
setGlobalPalette()
composePresentationFrame()
preDraw()
checkZoom()
clearBorder()
collect interface/error/FPS overlays
DisplayRuntime::present(...)
postDraw()
```

The presentation composer applies the selected screen effect and fills an
`IndexedDisplayFrame`. The display runtime then transfers indexed pixels and
overlays through the backend.

## 12. Frame Boundary

At the end of `runFrame()`:

```text
display.observeVisualLatency(...)
platformLifecycle.serviceFrameBoundary()
```

Visual latency is smoothed for diagnostics. Platform lifecycle service is
deferred to this boundary so suspend/resume work does not interrupt audio,
frame generation, or display transfer midway through a frame.

`Application::run()` then runs the current interface a second time and finally
paces the frame end with `FramePacer`.
