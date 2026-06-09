# Project Structure Map

## Top Level

```text
.
|-- src/                 application, runtime, visual engine, and display frontends
|-- tests/               focused unit, smoke, fixture, and benchmark sources
|-- resources/           runtime palettes, indexed images, and line objects
|-- external/miniaudio/  vendored miniaudio single-header dependency
|-- external/minimp3/    vendored MP3 decoder
|-- external/cthugha-js/ JavaScript port/reference tree
|-- cmake/               generated config and keymap helper scripts
|-- doc/                 older user-facing manual/manpage material
|-- tools/               small asset/helper scripts
|-- CMakeLists.txt       root CMake entry point
|-- src/CMakeLists.txt   application target source list
`-- PROJECT_*.md         current repository notes
```

The root source tree currently contains 159 `.cc` files and 162 headers. The
main application source list is `CTHUGHA_COMMON_SOURCES` in
`src/CMakeLists.txt`; use that list, not build-directory object files, as the
authoritative answer for what is compiled.

## Entrypoints And Lifecycle

- `src/main.cc`: constructs `Application`, calls `initialize()`, then `run()`.
- `src/Application.*`: startup/shutdown orchestration, main loop, frame
  scheduler, platform suspend/resume callbacks, and subsystem ownership.
- `src/Configuration.*`, `src/StartupConfiguration.cc`: defaults, ini,
  environment, command-line parsing, diagnostics, and startup config slices.
- `src/ProcessServices.*`: logging runtime, log sinks, clocks, countdown
  timers, sleepers, and random-source adapters.
- `src/PlatformLifecycle.*`: frame-boundary platform suspend/resume service.

## Audio

- `src/AudioTypes.h`: shared PCM format, sample format, and driver ids.
- `src/AudioSettings.*`: runtime-friendly view of startup audio settings.
- `src/AudioOutputConfig.*`: output-driver latency/device/dump settings.
- `src/RuntimeFactory.*`: startup-time selection of audio input and output.
- `src/PcmSourceFactory.*`: source strategy selection for line-in, random, WAV,
  MP3, raw PCM, and miniaudio capture.
- `src/PcmSource.*`: base source plus WAV, raw PCM, random noise, and MP3
  source implementations.
- `src/DspPcmSource.cc`: OSS DSP capture source.
- `src/MiniAudioCapturePcmSource.cc`: miniaudio capture source when enabled.
- `src/AudioOutput.*`: output base class and non-realtime null backend.
- `src/AudioPulseOutput.cc`: PulseAudio-compatible playback output.
- `src/AudioDSPOutput.cc`: OSS DSP playback output.
- `src/AudioMiniAudioOutput.cc`: miniaudio callback playback output.
- `src/DecodedAudioHistory.*`: decoded PCM ring/history used by visuals and
  passthrough.
- `src/AudioPassthrough.*`: optional output drain and presentation clock.
- `src/AudioIngest.*`: application-owned acquisition, history, passthrough, and
  visual frame construction.
- `src/AudioFrame.*`: raw/processed 1024-sample visual audio frame.
- `src/AudioProcessing*`, `src/AudioProcessor.*`: selectable processing modes.
- `src/AudioAnalyzer.*`, `src/AudioAnalysisSnapshot.*`: frame metrics and
  rolling acoustic state.
- `src/AudioFramePipeline.*`: per-frame processing and analysis port.
- `src/Mixer.*`, `src/MixerControls.*`, `src/MixerInterfaceControls.cc`: OSS
  mixer integration for live DSP input.

## Scene And Runtime Commands

- `src/Scene.*`: scene settings, commands, cues, snapshots, and selection APIs.
- `src/SceneRuntime.*`: owner for `Scene`, command target, serializer, visual
  catalog factory result, effect-selection registry, and selection state.
- `src/Scene*Catalog*`: typed image, palette, translation, wave-object, and
  built-in choice catalogs/loaders.
- `src/SceneVisual*`: visual selection sets and catalog-service factories.
- `src/SceneChangeScheduler.*`: automatic scene-change policy driven by audio
  metrics and `AcousticContext`.
- `src/AutoChangeSettings.*`, `src/AutoChangeControls.*`: runtime settings and
  controls for automatic scene changes.
- `src/RuntimeCommand.*`, `src/RuntimeCommandSink.h`: typed live commands.
- `src/RuntimeChangeMediator.*`: command dispatcher to scene, display, audio,
  auto-change, persistence, palette metadata, and shutdown owners.
- `src/RuntimeCommandTargets.*`: adapters for UI-originated option and
  effect-control changes.
- `src/RuntimeConfig*`, `src/RuntimePersistence.*`,
  `src/LegacyRuntimeConfigContributor.cc`: runtime config snapshots and ini
  persistence.
- `src/RuntimeDisplayControls.*`, `src/RuntimeAudioControls.*`,
  `src/RuntimeAutoChangeControls.*`, `src/RuntimeEffectControls.*`,
  `src/RuntimeShutdown.*`: subsystem control ports used by the mediator.

## Frame Generation And Visual Effects

- `src/FrameGeneratorRuntime.*`: owner for geometry, storage, scene binding,
  transition policy, and filterchain pipeline.
- `src/FrameGeometry.*`: visible indexed-frame dimensions and scene-geometry
  port.
- `src/FrameStorageLayout.h`, `src/FrameStore.*`,
  `src/FrameRenderTarget.*`: active/passive indexed storage, pitch, hidden
  border rows, and render-target access.
- `src/FrameGeneratorContext.*`, `src/FrameRenderContext.*`: per-frame audio,
  scene, timing, and presentation context objects.
- `src/FrameGeneratorPipeline.*`: filterchain ownership.
- `src/FrameGeneratorSceneBinding.*`: observes `Scene` changes and binds
  current scene settings/cues into concrete filter objects.
- `src/FrameFilterchain.*`, `src/FrameFilterchainSequence.*`,
  `src/FrameFilterchainFactory.*`, `src/FrameFilters.*`: filterchain runtime,
  stage order, factory, and concrete visual filters.
- `src/IndexedFrame.h`, `src/IndexedDisplayFrame.*`: frame descriptors used
  between generator, composer, and display.
- `src/FramePalette.*`, `src/ColorPalette.*`, `src/PaletteTransition.*`,
  `src/PaletteRandomGenerator.*`, `src/palettes.cc`: palette data, palette
  transitions, random palettes, map loading, and metadata.
- `src/Image.*`, `src/pcx.*`, `src/png.*`: indexed image model and loaders.
- `src/Flame.*`, `src/flames.*`: flame catalog and feedback implementations.
- `src/Translate.*`, `src/TranslateGenerator.*`,
  `src/TranslationOptions.*`: translation tables and generators.
- `src/Wave.*`, `src/WaveObject.*`, `src/waves.*`, `src/sound_tables.cc`:
  wave catalogs, optional line objects, wave drawing, and lookup tables.
- `src/Border.*`, `src/Flashlight.*`, `src/DosVga9x14Font.cc`: supporting
  visual stages.

## Display, Input, And Interface

- `src/DisplaySystem.*`: display-driver registry, active driver components,
  and display presentation settings.
- `src/DisplayDevice.*`: abstract display device and text drawing base.
- `src/DisplayBackend.h`, `src/DisplayPresentation.h`: backend presentation
  contract.
- `src/DisplayRuntime.*`: event processing and presentation wrapper over a
  display backend.
- `src/DisplayPresentationOptions.*`, `src/PresentationComposer.*`,
  `src/ViewportPolicy.*`, `src/ViewportPresentation.h`,
  `src/PixelTransfer.*`, `src/Screen*`: presentation-screen, viewport, overlay,
  and pixel-transfer helpers.
- `src/CthughaDisplay.*`: frame timing, composition, viewport management, and
  frontend presentation-coordinator base.
- `src/DisplayOverlay.*`: shared interface/error/FPS overlay collection used
  by display coordinators.
- `src/CthughaDisplayX11.cc`: X11 presentation coordinator.
- `src/DisplayDeviceX11.cc`, `src/DisplayDeviceX11-Panel.cc`: X11/Xt/Xaw
  display device, MIT-SHM path, X events, palette handling, optional panel, and
  palette metadata editing.
- `src/DisplayDeviceSDL3.*`, `src/Sdl3Presentation.*`: SDL3 display device,
  SDL event handling, renderer texture presentation, optional frame dumps, and
  SDL3 presentation helpers.
- `src/display.cc`, `src/display.h`: classic presentation screen functions.
- `src/CommandsInputRuntime.*`, `src/InputQueue.*`, `src/keymap.*`,
  `src/default.keymap`, `src/keys.*`, `src/xwin_keys.cc`: input queue, command
  registry, keymap loading, SDL-style key handling, and the X11 key wrapper.
- `src/Interface.*`, `src/InterfaceRuntime.*`, `src/InterfaceHelp.cc`,
  `src/InterfaceCredits.cc`, `src/InterfaceList.cc`, `src/OverlaySource.*`,
  `src/FpsOverlay.*`: on-screen interface and overlays.

## Assets

- `resources/map/`: 101 `.map` palette files. Loader: `src/palettes.cc`.
- `resources/img/`: 7 indexed image assets (`.pcx` and indexed `.png`).
  Loaders: `src/pcx.cc` and `src/png.cc`.
- `resources/obj/`: 23 text `.obj` line-object files loaded by `src/waves.cc`.

Runtime asset lookup checks the current directory, repository resource
directories, installed `CTH_LIBDIR` resource directories, and `--path DIR`
subdirectories as implemented by the loaders.

## Build And Tests

The root CMake build defines common sources, optional miniaudio sources, the
`cthugha` SDL3 target, the `xcthugha` X11 compatibility target, optional
focused tests, and optional benchmarks. Tests are registered in
`tests/CMakeLists.txt`; benchmark targets live under `tests/benchmarks/`.

Generated build files belong in out-of-tree CMake build directories. CMake also
generates `default.keymap.str` under the build tree from `src/default.keymap`.
