# Explicit Dependency Refactor Plan

## Goal

Make every runtime dependency explicit. Production code should receive the
state and services it needs through constructors, method parameters, or owned
context objects. Mutable process-wide globals should disappear from normal
application code. Temporary compatibility aliases are acceptable only when they
have a named removal phase and a test that prevents new call sites.

This review covers production `src/` code. The existing display refactor has
already moved much of screen composition toward explicit `IndexedFrame`,
`ScreenRenderContext`, `PresentationComposer`, and `DisplayRuntime` handoffs.
The remaining globals are concentrated in options/effect registries, scene and
display backend state, frame mutation buffers/renderer state, UI/input state,
and the diagnostics tail of process services. The old audio runtime/facade
cluster is now gone; remaining audio-adjacent notes are mostly Scene/Frame
Mutation boundary cleanup.
Startup configuration acquisition is no longer one of those ambient areas:
`ConfigurationBuilder` builds the startup `Config`, and `Application` passes
immutable slices to subsystem startup APIs.

## Progress Audit Since 73fd509

This status was cross-checked against commits `73fd509..20276a8` on
`modulerefactor` plus the current working tree, and a source scan of current
`src/`.

- Done: startup Configuration is explicit. Typed `Config` slices,
  `ConfigurationBuilder`, source strategies/patches/diagnostics, startup
  acquisition, automatic/continuation ini persistence, and most legacy startup
  parser globals have been replaced. Ini persistence now receives an explicit
  `LogSink` and uses a local writer object instead of a shared file cursor.
- Done for the current Runtime Reconfiguration boundary:
  `RuntimeCommand`, `RuntimeCommandSink`, `RuntimeChangeMediator`,
  `RuntimeDisplayControls`, `RuntimeAudioControls`,
  `RuntimeAutoChangeControls`, `RuntimeEffectControls`,
  `RuntimeCommandTargets`, `RuntimePersistence`, `RuntimeShutdown`, and
  `RuntimeConfigRegistry` route live changes without the mediator mutating
  subsystem globals directly. Generic option/effect routing now has explicit
  `RuntimeOptionTarget` and `RuntimeEffectControlTarget` ports. The legacy
  `Option&`/`EffectControl&` command factories and raw command payload fields
  have been removed. UI and X11 panel callers now use
  `RuntimeCommandTargetRouter`, which builds target-backed commands for a
  synchronous mediator call.
- Done for the current Audio module boundary. `AudioRuntime` and audio frame
  facade functions are gone; `AudioIngest`, `DecodedAudioHistory`,
  `AudioPassthrough`, owned `AudioFrame`, owned `AudioProcessor`/FFT state,
  owned audio processing selection, owned output config/dump state, owned OSS
  mixer session/controls, and instance-local audio diagnostics are in place.
- Done: Interface runtime state that belonged to Commands/Input has moved into
  Application-owned `InterfaceRuntime`: current interface selection, interface
  registration, runtime config/status adapters, save/status flags, and scoped
  option/effect command context.
- Done for Application Lifecycle ownership: Application now owns
  shutdown request state through
  `RuntimeCloseState`/`RuntimeShutdown`, and `Application` owns the major
  runtime roots. Auto-change policy now receives an Application-owned
  millisecond clock and random source instead of calling `gettime()`/`rand()`
  directly, and audio-processing random mode selection also uses the owned
  random source. Synthetic random PCM input is also built with that owned
  random source. Audio ingest pacing now receives an Application-owned
  seconds clock, and audio frame pipeline trace timing uses that same clock.
  X11 display/frame trace timing also receives that Application-owned seconds
  clock rather than calling `getTime()` directly.
  Interface error expiry and credits animation now use the
  Application-owned millisecond clock through `InterfaceRuntime`.
  `CStdRandomSource` now owns deterministic generator state, and direct
  Application seeding of process-global random state was removed after the
  migrated runtime paths stopped using legacy C random state.
  `CthughaDisplay` now owns its visual `FrameClock` and frame timestamp/delta
  values instead of exporting `now`/`deltaT`. The legacy public
  `gettime()`/`getTime()` adapters were removed. Logging verbosity now lives in
  Application-owned `LoggingRuntime`, with `CTH_*` macros kept as a temporary
  compatibility adapter. Quiet-message selection and video-director palette
  policy now use the Application-owned random source, as do randomized
  generated translation-table seeds, palette-transition strategy selection, and
  image-cue placement, random palette generation, and wave rendering.
  General-flame and generic EffectControl randomization now flow through
  `SceneCommands` plus runtime display/effect adapters and the
  Application-owned random source.
  QOTD prefetch
  deadline timing now uses Application-owned countdown timer factories that hand
  a per-fetch timer to the detached worker.
  Platform suspend signal state now lives in a `PlatformLifecycle`-owned
  `PlatformSuspendSignalState`; the previous file-scope request/installed/
  previous-handler globals and public `requestApplicationSuspend()` adapter were
  removed. The remaining file-scope pointer is only the POSIX `sigaction`
  bridge to the active owned state.
  The remaining tail belongs to Process Services/Diagnostics rather than
  lifecycle sequencing: the logging macro adapter remains as isolated
  compatibility while older modules are moved to explicit `LogSink&`.
- Done in the current working tree: the redundant audio-to-visual bridge class
  was deleted. `Application` now owns and calls `DefaultAudioFramePipeline`
  directly, then runs its Application-owned auto-change policy against the
  resulting frame metrics.
- Partial: Commands And Input is only halfway through. `InterfaceRuntime`
  removes the old interface current/edit globals, but `Action::head`,
  `Keymap::first`, `Keymap::current`, static `Keymap::action(...)`,
  `x11_key`, and `key_esc` remain.
- Partial: Scene owns more state through `Scene`/`SceneCommands`, owned
  auto-change settings, and runtime controls, but `EffectControl::first`,
  visual catalog/list globals, `videoDirector()`, and the scene command
  compatibility bridge remain. `AutoChanger` is no longer global or
  bridge-owned, but it still lives in `Application` rather than as Scene-owned
  policy.
- Partial: Frame Mutation and Display still have older display/frame globals
  and renderer-local statics. Audio and frame timing now pass through explicit
  frame contexts, but `CthughaBuffer::buffer`, display backend globals,
  overlay globals, and renderer state still need their own passes.

Audit conclusion:

- Configuration, Runtime Reconfiguration, and Audio can be marked complete for
  the goals of their current deglobalisation passes. Their remaining notes are
  boundary cleanup owned by Scene, Display, Frame Mutation, Commands/Input, or
  Process Services rather than evidence that those passes failed. Configuration
  persistence now also avoids the legacy logging macro adapter in
  `IniFiles.cc`.
- Application Lifecycle can be marked complete for lifecycle ownership:
  shutdown, suspend/resume polling, frame pacing, clocks for migrated runtime
  paths, Application-owned logging verbosity, and the Application-owned
  random-source adapter are explicit. The legacy logging macro adapter keeps
  Process Services/Diagnostics from being fully closed, but that is now a
  follow-up diagnostics pass rather than a lifecycle blocker. Audio
  runtime/source factory diagnostics now receive an explicit `LogSink` from
  Application, giving the logging migration its first non-macro module seam.
- Extra work completed beyond the original four-module focus includes
  Application-owned `InterfaceRuntime`, OSS mixer ownership, instance-local
  audio diagnostics, direct Application ownership of the audio frame pipeline,
  X11 timing injection, and removal of exported visual `now`/`deltaT`. The old
  `misc.cc` grab bag has also been deleted; the remaining C logging macro
  adapter is isolated in `LegacyLoggingAdapter.cc`.

Commit cross-reference highlights:

- Configuration: `744bd98`, `b6ab233`, `45e73a3`, `6186c8e`,
  `4f3c708`, `26cdcab`, `6cb168a`, `e49e8b2`, and `4ddcce2`
  moved defaults, audio/scene/keymap/path slices, source acquisition, removed
  stale startup capabilities, and split ini writing away from global config.
- Runtime Reconfiguration: `8e89861`, `6c4ca5f`, `7fdc454`,
  `89669f5`, `16f0a55`, `21f111d`, and `191e3dd` added the
  command boundary, mediator control ports, runtime config registry,
  owned shutdown/persistence, audio-processing routing, subsystem scaffolding,
  auto-change controls, mixer controls, runtime target ports, and runtime
  selection snapshots. Current working-tree follow-up removed raw
  `Option&`/`EffectControl&` command payloads and added
  `RuntimeCommandTargetRouter` for UI/panel target-backed dispatch.
- Audio: `850a51c`, `4c4acaf`, `2bcc8c4`, `51e9cbc`, `7c6334a`,
  `3e4a2ed`, `21f111d`, `218e5bd`, `107f100`, `191e3dd`, and
  `d280f83` removed `AudioRuntime`, audio facades, global metrics/context,
  `sound_minnoise`, global autochanger, global FFT/processing selection,
  startup/device/output option globals, OSS mixer globals, and shared audio
  diagnostic counters. Current working-tree follow-up removes the intermediate
  audio-to-visual bridge wrapper and has `Application` call
  `DefaultAudioFramePipeline` directly.
- Application Lifecycle and Process Services: `7fdc454` removed lifecycle
  dependence on `cthugha_close`; later committed and current working-tree
  changes move clocks, frame pacing, random source ownership, logging
  verbosity, display timing, interface timing, audio timing, QOTD deadlines,
  and several runtime diagnostics behind Application-owned collaborators.
- Interface runtime work beyond the original four-module focus: `d280f83`
  introduced Application-owned `InterfaceRuntime`, replacing old interface
  selection globals and scoped option/effect command globals while leaving
  concrete panel globals for a later UI/display pass.

## Next Module Choice Point

Three sensible next attacks remain. All three are valid; the choice is mostly
about whether to reduce command/input risk, scene/state risk, or rendering risk
first.

1. Commands And Input
   - How: introduce an explicit `InputQueue`, `KeymapRegistry`,
     `CommandRegistry`, `CommandDispatcher`, and per-dispatch `CommandContext`.
     Move `Action::head`, `Keymap::first/current`, static
     `Keymap::action(...)`, `x11_key`, and `key_esc` behind those objects.
   - Why: `InterfaceRuntime` already cleared the UI edit-state half of this
     boundary, so the remaining hidden state is well isolated. This would make
     key handling testable without scene/display/audio globals and would remove
     a lot of old static dispatch surface.

2. Scene
   - How: introduce `SceneState`, `SceneSnapshot`, `EffectRegistry`,
     `VisualCatalogs`, `SceneSerializer`, and a Scene-owned
     `SceneChangeScheduler` to absorb `AutoChanger` policy. Then make
     `SceneCommands` a real command target over owned scene state instead of a
     compatibility bridge.
   - Why: this directly attacks the biggest remaining runtime-domain globals:
     visual selections, `EffectControl::first`, preset traversal,
     `videoDirector()` scene policy, and `sceneCommandsForLegacyCallbacks()`.
     It also finishes the cleanup that Audio and Runtime Reconfiguration are
     currently waiting on.

3. Frame Mutation
   - How: extract `FrameStore` from `CthughaBuffer::buffer/current`, move
     `display.cc` renderer statics into renderer-state objects, make
     `VideoDirector` hand frame composition to a `FrameComposer`, and keep
     `ScreenRenderContext` as the explicit audio/scene/display input carrier.
   - Why: this is the next natural step before a full Display pass. It removes
     the oldest rendering globals while preserving display backend behavior,
     and it gives Scene and Display a clean frame handoff instead of sharing
     global buffers.

## Remaining Ambient Dependency Inventory

### Process Services

- Done: the legacy `cthugha_close` shutdown flag was removed; runtime shutdown
  requests use Application-owned `RuntimeShutdown`/`RuntimeCloseState`.
- Done: `AutoChanger` receives `MillisecondClock`, `RandomSource`, and
  `AutoChangeQuietObserver` collaborators from Application-owned adapters, so
  auto-change timing/random wait policy and quiet-message observation are
  deterministic in tests and no longer reach into process globals.
- Done: `AudioProcessingState` receives the same Application-owned
  `RandomSource`, so unspecified or invalid audio-processing mode selection is
  deterministic in tests and no longer calls `rand()` directly.
- Done: `RandomNoisePcmSource` receives `RandomSource` through
  `AudioIngest`/`RuntimeFactory`/`PcmSourceFactory`, so synthetic random audio
  no longer calls `rand()` directly.
- Done: randomized generated translation-table seeds receive `RandomSource`
  from Application-owned startup composition instead of reading process time.
- Done: image-cue placement receives `RandomSource` from `VideoDirector`, so
  `Image.cc` no longer uses the legacy `Random()` helper for placement.
- Done: general-flame randomization receives `RandomSource` through
  `SceneCommands`, so `flames.cc` no longer uses the legacy `Random()` helper.
- Done: generic EffectControl empty/invalid selection and change-one/change-all
  randomization receive `RandomSource` through `SceneCommands` and runtime
  display/effect adapters, so `EffectControl.cc` no longer uses `rand()` or the
  legacy `Random()` helper.
- Done: random palette generation is split into `PaletteRandomGenerator` and
  receives `RandomSource` through `SceneCommands`, so `palettes.cc` no longer
  uses the legacy `Random()` helper.
- Done: wave rendering receives `RandomSource` through `VideoDirector`,
  `WaveFilter`, and `WaveRuntime`, so `waves.cc` no longer uses `rand()` or the
  legacy `Random()` helper.
- Done: QOTD prefetches receive `CountdownTimerFactory` from Application-owned
  startup composition and move a per-fetch `CountdownTimer` into the detached
  worker, so socket deadlines no longer read `std::chrono` in
  `QotdMessagesProvider`.
- Done: production `AudioIngest` receives `SecondsClock` from Application and
  no longer constructs its own `SystemAudioIngestClock`/`getTime()` fallback.
- Done: `AudioFramePipeline` receives `SecondsClock` from Application for trace
  timing and no longer calls `getTime()` directly.
- Done: `AudioOutput` receives `SecondsClock` from Application through
  `AudioIngest`/`RuntimeFactory`, so output service trace timing no longer
  calls `getTime()` directly.
- Done: `Application` main-loop/frame tracing and frame-pacing calls use its
  owned `SecondsClock` instead of calling `getTime()` directly.
- Done: `FrameClock` uses the shared `SecondsClock` interface instead of a
  private `FrameTimeSource`/direct `getTime()` adapter.
- Done: `CthughaDisplay` owns its `FrameClock` plus current timestamp/delta;
  `Application` and `VideoFrameContext` consume those values explicitly, and
  the exported `now`/`deltaT` aliases were removed.
- Done: `InterfaceRuntime` receives the Application-owned `MillisecondClock`,
  and interface error expiry/credits animation no longer call `gettime()`
  directly.
- Done: X11 display/device frame tracing receives `SecondsClock` from
  Application through the display factories and no longer calls `getTime()`
  directly.
- Done: `CStdRandomSource` owns its own deterministic state and no longer wraps
  process-global `rand()`. Application startup no longer calls `srand(time(0))`,
  and the legacy `seedLegacyProcessRandom()` compatibility adapter plus
  `imath.cc::Random()` helper were removed after runtime users migrated.
- Done: legacy public `gettime()` and `getTime()` adapters were removed.
  Normal production runtime paths now consume owned clock interfaces where
  migrated, with `ProcessServices` owning the production steady-clock adapter.
- Frame timing state: remaining timing values flow through owned
  `CthughaDisplay`/`FrameClock` state and explicit `VideoFrameContext`
  snapshots. `FramePacer` is now owned by `Application`.
- Randomness: normal Scene/Display domains now receive the Application-owned
  `RandomSource`.
  Quiet-message selection, generated translation seed selection,
  video-director palette policy, palette-transition strategy selection, and
  image-cue placement now use the Application-owned random source. Random
  palette generation, wave rendering, general-flame randomization, and generic
  EffectControl randomization also use that source through `SceneCommands`,
  `WaveRuntime`, and runtime display/effect adapters. No normal runtime path
  now uses process-global `rand()`/`srand()` or the legacy `Random()` helper.
- Done for mutable logging state: logging verbosity moved into
  Application-owned `LoggingRuntime`. `CTH_*` macros still go through the
  legacy C logging adapter in `LegacyLoggingAdapter.cc` until the broader
  diagnostics pass replaces macro call sites with explicit sinks. Shared log
  level constants now live in `LogLevels.h`, so `ProcessServices` no longer
  depends on the legacy `cthugha.h` macro header.
- Done for the first explicit logging consumers: `Application` owns
  `ConsoleLogSink`, `AudioIngest` receives `LogSink&`, and
  `AudioIngest`/`RuntimeFactory`/`PcmSourceFactory` use that sink for
  startup/source-selection diagnostics instead of `CTH_DEBUG`/`CTH_TRACE`.
  `AudioFramePipeline` also receives `LogSink&` for per-frame audio/analysis
  diagnostics, so the current Audio ingest-to-pipeline path no longer reaches
  through the legacy logging macro adapter.
  Source acquisition (`PcmSource`, `AudioInput`, and `DspPcmSource`),
  `AudioOutput`, and its submitted-PCM debug reporter now receive that same
  sink for acquisition/output timing/service diagnostics; the Pulse and OSS/DSP
  output backends, `AudioPassthrough`, and `AudioOutputDump` also use it for
  backend/lifecycle diagnostics. `AudioSettings`, `DecodedAudioHistory`,
  `AudioFrameBuilder`, `AudioProcessingSelector`, `AcousticContext`,
  `MixerSession`/`MixerControls`, `AutoChangeControls`, and `AutoChanger` now
  also receive or retain explicit sinks for their runtime diagnostics, and the
  stale `cthugha.h` includes were removed from those target files.
- Done for Application Lifecycle diagnostics: `Application` startup/run-loop
  logging now uses its owned `ConsoleLogSink`, `PlatformLifecycle` receives
  `LogSink&` for suspend/resume diagnostics, and the legacy logging-runtime
  install/clear declarations moved from `cthugha.h` into `ProcessServices.h`.
- Done for Application Lifecycle suspend state: `PlatformLifecycle` owns
  suspend request, installation, and previous-handler state through
  `PlatformSuspendSignalState`; the process-wide `requestApplicationSuspend()`
  compatibility hook and old file-scope suspend request globals were removed.
- Done for Application display startup: `Application` now calls a
  `DisplayFrontendInitializer` port, with the raw legacy `cth_init` frontend
  symbol isolated in `LegacyDisplayFrontendInitializer`.
- Done for Configuration persistence diagnostics: automatic and continuation
  ini writes, plus continuation cleanup, receive `LogSink&` through
  `Application`/`IniRuntimePersistence`; `IniFiles.cc` no longer includes
  `cthugha.h`, owns a shared `FILE*` output cursor, or keeps a static home-path
  string buffer.
- Done for Audio FFT ownership: default-constructed `AudioProcessor` now owns
  its `FixedPointAudioFftProcessor`; the process-local
  `defaultAudioFftProcessor()` static fallback was removed.
- Done: stale `systemf()` shell execution helper was removed after the
  compressed-asset loader path that needed it disappeared.

### Option And Configuration Globals

- Core startup choices now live in `AppConfig` and `LoggingConfig`; the
  remaining options below are runtime/editable state or compatibility mirrors
  populated from config slices.
- Done since 73fd509: Audio startup/session configuration is no longer held in
  option globals. `AudioConfig`, `AudioSettings`, `PcmFormat`,
  `AudioOutputConfig`, and `AudioOutputDump` are passed through factories and
  owned runtimes. The old names such as `soundSampleRate`, `soundChannels`,
  `pulse_server`, and `audio_output_dump` may survive as config keys or owned
  fields, not process-wide mutable options.
- Display options/config: `zoom`, `maxFramesPerSecond`, `showFPS`,
  `text_size`, `fontSize`, `disp_size`, `bypp`, `bytes_per_line`, `draw_mode`,
  `screenSizes`, `bufferSizes`, X11 flags in `xcthugha.h`, and `xcth_font`.
- Done since 73fd509: Auto-change runtime values moved into owned
  `AutoChangeSettings`/`AutoChangeControls`; `sound_minnoise` moved into
  audio-analysis startup config consumed by `AudioFramePipeline`.
- Still remaining here: display options, message/palette smoothing values, and
  visual effect selection state.
- Visual effect selections: `screen`, `flame`, `flameGeneral`, `wave`,
  `waveScale`, `table`, `object`, `translation`, `palette`, `border`,
  `flashlight`, `use_translates`, `use_objects`.
- Done since 73fd509: `audioProcessing` is no longer a global option; it is
  owned by `AudioProcessingState`/`AudioProcessingSelector`.
- File/path startup config now lives in `PathConfig`/`InputConfig` slices.

### Registries And Catalogs

- `EffectControl::first` is the central hidden registry used by initial option
  resolution, random changes, save/restore, ini persistence, and presets.
- `effectPresetCatalog` is process-wide and walks the hidden effect registry.
- `Action::head`, `Keymap::first`, and `Keymap::current` are hidden
  registries/current selections.
- Done since 73fd509: `Interface::head`, `Interface::current`, and current
  option/effect edit globals moved into Application-owned `InterfaceRuntime`.
- Mutable catalog/list globals include `paletteEntries`, `screenEntries`,
  `_flames`, `_objects`, wave/object/table lists, and audio processor entries.
- Mostly immutable catalogs still have global exposure: `screenCatalog`,
  `flameCatalog`, `waveCatalog`, `keyAssoc`, generated/default keymap data.
- Math lookup tables `sine` and `sin360` are mutable process-wide tables filled
  by `init_imath()`.

### Runtime Object Aliases

- `CthughaBuffer::buffer` and `CthughaBuffer::current`.
- `cthughaDisplay`, `displayDevice`, `displayBackend`, `displayRuntime`.
- Done since 73fd509: the process-wide `autoChanger` pointer was removed.
  Current working-tree follow-up removed the intermediate bridge wrapper and
  keeps the current `AutoChanger` instance in `Application`.
- `videoDirector()` singleton.
- `sceneCommandsForLegacyCallbacks()` global bridge.

### Audio State

Done since 73fd509:

- `AudioRuntime.cc`/`.h` were deleted. `Application` owns `AudioIngest`, which
  owns acquisition, decoded history, visual pacing, and current `AudioFrame`.
- `AudioFrame` facade functions and silent fallback globals were removed.
- `audioAnalyzer`, `audioMetrics`, and global `acousticContext` were removed;
  `Application` owns `AcousticContext` and passes frame metrics through
  `VideoFrameContext`/`ScreenRenderContext`.
- `AudioProcessor` is an owned instance. FFT state is behind
  `AudioFftProcessor`, with the old lookup/static tables removed.
- Audio processing selection is owned by `AudioProcessingState` and changed
  through `AudioProcessingSelector`/`RuntimeAudioControls`; random selection
  uses an injected `RandomSource`.
- Synthetic random audio input receives `RandomSource` from Application-owned
  `AudioIngest` composition through `RuntimeFactory` and `PcmSourceFactory`.
- Audio ingest visual pacing receives `SecondsClock` from Application rather
  than owning a hidden `getTime()` adapter.
- Audio frame pipeline trace timing receives `SecondsClock` from Application
  rather than calling `getTime()` directly.
- Audio output service trace timing receives `SecondsClock` from Application
  through `RuntimeFactory` rather than calling `getTime()` directly.
- PCM history, passthrough, output config, output dumping, Pulse diagnostics,
  submitted-PCM diagnostics, and OSS mixer controls are instance-owned.

Remaining audio-adjacent work:

- Done in current working tree: per-frame audio processing/analysis moved into
  `AudioFramePipeline`/`DefaultAudioFramePipeline`; the redundant bridge
  wrapper was deleted. `Application` owns the remaining AutoChanger policy
  until Scene-owned auto-change scheduling replaces it.
- Audio still receives visual frame-window sizing from `Application`; keep that
  explicit until Frame Mutation owns frame geometry.
- Auto-change now receives Application-owned clock/random adapters directly;
  the remaining issue is module placement, not hidden clock/random ownership.
  Broader legacy logging macro cleanup still moves with Process
  Services/Diagnostics.

### Video And Screen Rendering State

- `VideoDirector.cc` is a singleton and still reads `cthughaDisplay`,
  `CthughaBuffer::buffer`, `maxFramesPerSecond`, `changeMsgTime`, and
  `paletteSmoothingChance`.
- `SceneCommands` is explicit as a facade, but it still reads every visual
  option singleton to build `SceneSettings`.
- Done since 73fd509: `display.cc` renderer audio inputs now arrive through
  `ScreenRenderContext` rather than global audio facades/metrics.
- `display.cc` also keeps renderer state in file-scope statics:
  `perm_lines`, `height_offset`, `s1`, `s2`, `p`, `rot`, `scaleFactor`,
  `scaleFactorPhase`, `splatSize`, `zicks`, and function-local animation
  state. These should be per-renderer state, not ambient state.
- `VideoFilters.cc` still has frame-commit diagnostic throttling as a
  function-local `static int debugReports`; that should become
  `FrameCommitFilter` state when Frame Mutation takes ownership of filterchain
  runtime state.

### Display Backend And Overlay State

- X11 platform globals: `xcth_display`, `xcth_app_con`, `DisplayDeviceX11::xcth_toplevel`.
- Backend frame layout globals: `disp_size`, `bypp`, `bytes_per_line`,
  `draw_mode`, `colormapped`, `bitmap_colors0..3`.
- Text/overlay globals: `text_size`, `fontSize`,
  static text colors, `errors`, and temporary `ScopedOverlayDisplayDevice`
  swapping the global `displayDevice`.
- X11 panel callbacks are static and depend on global options/catalogs even
  when the panel has `SceneCommands`.

### Input And UI State

- Key input: `x11_key`, `key_esc`, `keyAssoc`, `nKeyAssoc`.
- Done since 73fd509: UI current editing state moved into scoped
  `InterfaceRuntime` command context; key quit routes through runtime commands
  and `RuntimeShutdown`.
- Remaining: key actions/static keymaps still call into global registries or
  static dispatch paths (`screen`, `zoom`, `displayDevice`,
  `sceneCommandsForLegacyCallbacks()`, `Keymap::action(...)`), and raw X11 key
  input still uses globals.

### Ini Persistence And Platform State

- Done since 73fd509: `IniFiles.cc` no longer keeps process-wide writer state
  or a persisted startup `Config` snapshot for runtime save commands. Automatic
  and continuation writes build a local `IniWriter` around the opened file, and
  runtime saves read the current explicit `RuntimeConfigRegistry` snapshot
  through `IniRuntimePersistence`.
- `DisplayDeviceX11::cth_init()` still lets Xt consume toolkit command-line
  options/resources during X shell initialization.
- Done since 73fd509: `PlatformLifecycle` owns suspend request, handler
  installation, and previous-handler state. The remaining file-scope pointer is
  the POSIX signal bridge to active owned state, not application-visible
  lifecycle ownership.
- Done since 73fd509: OSS mixer state moved into `MixerSession`/`MixerDevice`
  and UI adapters. Current working-tree follow-up routes mixer diagnostics
  through an explicit `LogSink&` instead of the legacy `CTH_*` macro adapter,
  and mixer option label text is now stored per adapter instead of in a shared
  function-local static buffer.
  Submitted PCM dumping moved into `AudioOutputDump`, and submitted-PCM debug
  throttling is per `AudioOutput`.

### Stale Or Suspicious Compatibility Surface

These are declared or defined but appear unused in the current production path:
`screen_first`, `cth_main`, `cthugha_mode_text`, `cur_palette`,
`display_bitmap`, `display_text_time`, and the global `rev_byte_order`
declaration in `display.h` while X11 uses a member named `rev_byte_order`.
Each should be deleted or covered by a build target that proves it is still
needed.

Done in current working tree: `RuntimeCommand` no longer has raw `Option*` or
`EffectControl*` payload fields. Interface and X11 panel code route borrowed
legacy option/effect objects through `RuntimeCommandTargetRouter`, which keeps
the adapter lifetime scoped to the synchronous mediator call.

## Target Shape

Application should own one composition root, but the primary architecture should
be modules, not a bag of peer services. `ApplicationContext` is allowed to wire
module roots together; it must not become a service locator. Production code
should receive the narrow collaborator it needs through constructors or method
parameters. Only `Application` and module-root constructors should know about
the whole composition root.

The module boundaries are:

- Application Lifecycle
- Configuration
- Runtime Reconfiguration
- Commands And Input
- Scene
- Audio
- Frame Mutation
- Display

Subsystems should receive only the subset they need. For example,
the current `AutoChanger` behavior should become Scene-owned
`SceneChangeScheduler` policy that receives clock, random source,
auto-change options, audio analysis snapshots, and a scene command target. It
should not read global audio metrics, global time, global random, or
`videoDirector()`.

## Module Boundary Definitions

Services are candidate implementation details of modules. A service that fits
cleanly may be implemented under that module. A service that only probably fits
needs a boundary test before implementation. A service marked split-required
should not be implemented as a single object because doing so would preserve
implicit coupling under a new name.

### Application Lifecycle Module

- Boundary: Owns process startup, run-loop sequencing, shutdown, platform
  lifecycle hooks, and construction/destruction order for all other modules.
- Owns/replaces: shutdown requests, signal/lifecycle polling, process-level time
  and randomness sources where they are genuine infrastructure, and startup
  diagnostics.
- Inputs: command-line arguments, platform lifecycle notifications, module-root
  factories, and validated startup configuration.
- Outputs: explicit module roots, run-loop ticks, shutdown requests, and final
  teardown ordering.
- Allowed dependencies: configuration outputs during startup; module roots only
  through narrow lifecycle interfaces such as `start()`, `poll()`, `stop()`,
  and `shutdownRequested()`.
- Forbidden dependencies: visual catalogs, mutable scene state, display device
  globals, audio frame state, and whole-`ApplicationContext` access outside the
  composition root.
- Lifecycle: built first; installs platform lifecycle hooks before subsystem
  startup; sequences configuration, scene, display, audio, input, frame
  mutation, and display presentation; tears modules down in reverse order.
- Fits cleanly: `ShutdownController`, `PlatformLifecycle`.
- Probably belongs here: `Clock`, `RandomSource`, `Logger`. Each is
  cross-cutting infrastructure, so dependents must receive narrow interfaces
  and tests must prevent a service-locator style grab.
- Split-required: frame pacing belongs at the boundary between lifecycle and
  frame mutation/display, not inside a generic clock; overlay logging belongs
  to Display or Scene messages, not `Logger`.
- Does not fit: `ApplicationContext` itself. It is composition glue, not a
  service and not a dependency that ordinary production code may request.
- No shell/external-process runner is currently needed; add one only if future
  behavior intentionally reintroduces shell-out.

### Configuration Module

- Boundary: Owns defaults, command-line parsing, ini loading,
  validation, path resolution, startup configuration schemas/descriptors, and
  persistence formats.
- Owns/replaces: scalar/string startup option globals, path globals, parser
  state, legacy option-table coupling, ini reader globals, and save/restore
  wiring for configurable state.
- Inputs: command-line arguments, supported environment sources, ini files,
  build defaults, and persistence sources.
- Outputs: immutable startup configuration, option schemas/descriptors,
  validated path resolution, and serialization adapters for modules that own
  configurable domain state.
- Allowed dependencies: process logging, filesystem/path adapters, and
  serialization adapters supplied by owning modules.
- Forbidden dependencies: direct audio/display/scene runtime objects, hidden
  mutation of module state during parsing, or persistence code walking module
  globals.
- Lifecycle: defaults are built before parsing; ini files, environment, and
  command line are applied during startup; Runtime Reconfiguration consumes the
  resulting schemas/descriptors after Configuration; final persistence runs
  after runtime modules stop exposing mutable state.
- Fits cleanly: none of the current candidate services fits perfectly as-is.
- Probably belongs here: `PathConfig` if it is narrowed into a path declaration
  and resolver; `AppOptions` if split into startup config, runtime options, and
  scene selection state; `IniStore` if split into config loading and
  persistence.
- Split-required: `AppOptions` historically combined startup-only config, live
  runtime controls, scene selection, display/audio settings, and UI-editable
  option metadata. `IniStore` should remain split between startup config
  loading, runtime persistence, and module-owned serializers.
- Does not fit: `EffectRegistry` persistence should not be owned by
  Configuration; Configuration should call a Scene-provided serializer instead.
- Landed candidates since 73fd509: `ConfigurationBuilder`,
  source-strategy-style config acquisition, `ConfigPatch`, `ConfigSchema`,
  `DeferredLogBuffer`, typed `Config` slices, and explicit runtime
  persistence entry points.
- Still useful candidates: narrower `PathResolver`/`ConfigLoader` names if
  path lookup and source selection need more isolation, plus module-owned
  serializers for Scene and Display.

#### Configuration Module Findings

The Configuration module is now the first substantial module built by
`Application`. Its job is to acquire startup configuration needed by downstream
modules without constructing those modules and without mutating their state.

The implemented startup shape is:

```cpp
ConfigBuildResult startupConfig = buildStartupConfig(argc, argv);
Application handles diagnostics/help and then supplies slices:

loggingRuntimeValue.configure(startupConfig.config.logging);
configureAudioOptions(startupConfig.config.audio);
configureCthughaDisplay(startupConfig.config.display);
configureAutoChanger(startupConfig.config.autoChange);
sceneCommands().applyStartupConfig(startupConfig.config.scene);
Keymap::init(startupConfig.config.input);
```

Internally, `buildStartupConfig()` performs a small bootstrap pass for
`--path`/`--ini-file`, then builds the final value through:

```cpp
ConfigurationBuilder builder(bootstrapDiagnostics);
builder.addDefaults()
    .addIniFile(...)
    .addEnvironmentVariables(...)
    .addCommandLine(argc, argv)
    .build();
```

The exact source list should be configurable and test-owned, but the precedence
rule should be fixed: every source produces a `ConfigPatch`; later patches
override earlier patches for the same key. Hardcoded defaults are the lowest
precedence. Command-line options are the highest precedence. A command-line
`--ini-file` override is source selection, not an ordinary runtime option: it
replaces the default ini-file source list with the requested file.

The completed replacement deliberately ends mutation during parsing.
Configuration building has exactly one product: a `Config` value, or a failed
`ConfigBuildResult` with diagnostics. It may preserve useful semantics such as
source precedence, aliases, validation rules, and defaults, but it must not
initialize subsystems, write runtime globals, stage callbacks, or perform
downstream work.

- `ConfigurationBuilder` owns source order and precedence. It does not know
  about audio, scene, display, or command objects.
- `ConfigAcquisitionStrategy` objects know how to read one source type:
  defaults, ini file, supported environment variables, and command line.
- Each strategy returns a `ConfigPatch` plus diagnostics. It must not mutate
  globals, load scene files, open audio devices, resize buffers, or touch
  display objects.
- `ConfigSchema` owns option names, aliases, types, validation, clamping, and
  deprecation/canonical-name warnings. This is the startup source of truth.
- `DeferredLogBuffer` records warnings/errors/debug traces produced while
  building config. After `LoggingConfig` is known, `Application` constructs the
  real logger and flushes buffered diagnostics according to the final level.
- Fatal parse errors should be returned as structured `ConfigDiagnostic`
  entries in a failed `ConfigBuildResult`, not printed directly from the parser.
- After config is built, `Application` decides initialization order and supplies
  only the required config slice to each module root. Audio receives
  `AudioConfig`; Display receives `DisplayConfig`; Scene receives `SceneConfig`
  plus `EffectPolicy`; Runtime Reconfiguration receives option descriptors and
  explicit module reconfiguration targets; Commands And Input receives
  `InputConfig` and command descriptors.

The produced `Config` should be a composite value, but consumers should receive
only their slice:

- `AppConfig`: application-level startup choices such as save-on-exit,
  continuation behavior, and process-level toggles.
- `LoggingConfig`: verbosity and diagnostic sink policy.
- `PathConfig` or `PathResolverConfig`: library paths, ini override, keymap
  path, audio file paths, and resource roots.
- `AudioConfig`: source mode, generation/file/live input settings, sample
  format/rate/channels, output/passthrough settings, mixer/device settings,
  dump path, and acoustic-analysis thresholds such as minimum noise.
- `DisplayConfig`: display mode, window size/position, X11 flags, font,
  colormap/shared-memory/fullscreen/panel policy, zoom, and FPS display.
- `SceneConfig`: initial screen/flame/wave/table/object/translation/palette/
  border/flashlight/audio-processing selections. After startup this becomes
  Scene-owned mutable state, not Configuration-owned state.
- `EffectPolicy`, `SceneTransitionPolicy`, and `MessagesConfig`: allowed effect
  catalogs/presets, transition behavior, and quiet-message/QOTD policy.
- `RuntimeOptionSchema`: descriptors for settings that can be edited at
  runtime and persisted. Configuration may define schemas and persistence keys,
  but Runtime Reconfiguration coordinates live edits and module owners keep the
  live values.
- `KeymapConfig`: selected keymap file and keymap-loading policy for Commands
  And Input.

Important pushback/constraints:

- `Config` should be read-only after `build()`. Mutable scene selection is not
  "the config changing"; it is Scene state changing from initial configuration
  and commands. Persisting that state should use Scene serializers.
- Runtime-editable options need an explicit owner. Process-level toggles belong
  to Application Lifecycle or Logging; audio tuning belongs to Audio; scene
  selections belong to Scene; display choices belong to Display. The UI should
  edit through Runtime Reconfiguration and typed command targets, not by
  writing `Config`.
- `ConfigurationBuilder` must not emit downstream commands. Current
  startup side effects such as loading quiet-message files, setting image
  loading, resizing `CthughaBuffer::buffer`, changing `audioProcessing`, or
  mutating mixer initial volumes are config values consumed by the owning
  module during startup.
- Source discovery may require a bootstrap pass, but that pass should also be
  explicit. `--path`, `--ini-file`, `--help`, and early logging/env controls
  can be parsed by a `BootstrapCommandLineSource` that returns
  `BootstrapConfig`, then the full builder runs with the selected sources.
- Xt toolkit options/resources remain outside Cthugha startup configuration.
  `Application` still passes a display argument vector to X11 shell
  initialization; if that becomes Cthugha config in the future, it should be
  captured as an explicit X11 slice.

Test targets for Configuration:

- Builder precedence: defaults < library ini < home/auto ini < local/extra ini
  < continuation < environment < command line.
- `--ini-file` override uses only the requested ini file plus defaults and
  command-line/env sources.
- Deferred diagnostics are buffered before `LoggingConfig` and flushed at the
  final verbosity level.
- Aliases/deprecated names produce diagnostics but canonical names win.
- Invalid values fail or clamp according to schema rules with source/line
  attribution.
- Consumers can be constructed with only `AudioConfig`, `DisplayConfig`,
  `SceneConfig`, etc.; tests should fail if a consumer needs the whole
  `Config`.
- No config source or parser writes to option globals, display globals, audio
  globals, scene globals, or keymap globals.

Current startup configuration status:

- Done: `ConfigDiagnostic`, `DeferredLogBuffer`, `ConfigPatch`,
  `ConfigSchema`, typed `Config` slices, source strategy interfaces, and tests
  for defaults, ini text, environment, command-line, and dependency boundaries.
- Done: `Application` builds typed startup `Config`, acts on diagnostics/help,
  and passes immutable slices to logging, input, audio, display/X11,
  autochanger, audio analysis, effect policy, scene transition, messages,
  scene, keymap, catalog loading, and ini persistence.
- Done: old parser files and globals for startup config acquisition were
  deleted or replaced with typed config/persistence entry points.
- Done since 73fd509: runtime persistence now reads explicit runtime
  contributors and owned settings instead of a global startup config snapshot.
- Remaining: Configuration still defines some schemas/keys for runtime-owned
  domains, and Scene/Display still need module-owned serializers before
  Configuration can be considered fully isolated from live runtime domains.

### Runtime Reconfiguration Module

- Boundary: Owns runtime setting changes after startup. It turns UI edits,
  key actions, command-line-style runtime commands, panel changes, automatic
  changes, and scripted changes into typed reconfiguration requests applied to
  the module that owns the live state.
- Why this follows Configuration: the startup `Config` refactor makes initial
  dependencies explicit, but it does not by itself stop `Option` from owning
  live values, parsing, validation, UI text, side effects, and persistence.
  This module must be addressed immediately after Configuration so the refactor
  does not merely replace startup globals while leaving runtime mutation hidden.
- Owns/replaces: the live-value responsibilities currently concentrated in
  `Option`, `OptionInt`, `OptionOnOff`, `OptionTime`, `EffectControl` as an
  `Option`, and UI option editing globals.
- Inputs: option schemas/descriptors from Configuration, command intents from
  Commands And Input, current snapshots from owning modules, and explicit
  module reconfiguration targets.
- Outputs: typed `RuntimeChangeRequest` values, validation diagnostics,
  applied-change results, updated module snapshots, and persistence snapshots.
- Initial scope to cover: audio source and audio tuning changes; scene/effect
  selections such as screen, flame, wave, object, translation, palette, border,
  flashlight, and audio-processing choices; display screen selection, zoom,
  FPS display, and backend presentation toggles.
- Ownership rule: Runtime Reconfiguration coordinates live edits, but it does
  not own all live state. Audio owns audio runtime state, Scene owns effect and
  scene selection state, Display owns display presentation state, and
  Application owns lifecycle-only toggles. The reconfiguration layer calls
  explicit module APIs instead of writing globals or mutating `Config`.
- Lifecycle: constructed after Configuration has produced startup config and
  option descriptors; attached to module roots during Application startup;
  applies changes only at safe boundaries, normally between frames or at
  explicit restart/reopen points; provides snapshots to persistence during
  shutdown.
Current runtime reconfiguration status:

- Done since 73fd509: `RuntimeCommand` and `RuntimeCommandSink` define the
  command boundary for key/UI/panel/auto-change requests.
- Done since 73fd509: `RuntimeChangeMediator` no longer mediates by mutating
  the old globals directly. It delegates to explicit control ports:
  display, audio, auto-change, effects, persistence, and shutdown.
- Done since 73fd509: persistence and shutdown were extracted from the
  mediator into `RuntimePersistence`/`IniRuntimePersistence` and
  `RuntimeShutdown`/`RuntimeCloseState`.
- Done since 73fd509: `RuntimeConfigRegistry` and runtime config
  contributors provide current-selection snapshots for UI display and
  persistence.
- Done since 73fd509: live audio-processing and auto-change edits now mutate
  owned state through runtime controls; OSS mixer edits route through
  `RuntimeAudioControls`/`MixerControls`.
- Done in current working tree: generic raw `Option&`/`EffectControl&`
  command factories and raw `RuntimeCommand` payload fields are gone.
  `RuntimeCommandTargetRouter` now gives Interface/X11 panel code an explicit
  target-backed dispatch port.
- Remaining: `Option`/`EffectControl` still carry UI text/parsing and many
  scene/display live values. Scene and Display need their own live-state
  owners before `Option` becomes metadata/adapters rather than domain state.

### Commands And Input Module

- Boundary: Owns input event normalization, keymap lookup, command intent
  creation, command dispatch, and option-editing interaction state.
- Owns/replaces: `x11_key`, `key_esc`, `Action::head`,
  `Keymap::first`, `Keymap::current`, static keymap dispatch, key association
  globals, `Interface::head`, `Interface::current`, `currentOption`,
  `currentEffectControl`, and `currentOptionInterfaceElement`.
- Inputs: raw input events from Display/platform, keymap configuration,
  available command definitions, and editable option descriptors from
  Configuration and Scene.
- Outputs: typed command intents and explicit command execution through a
  narrow `CommandContext`.
- Allowed dependencies: `InputQueue`, `KeymapRegistry`, command registry,
  option editor model, and narrow command targets supplied per dispatch.
- Forbidden dependencies: direct pointers to scene globals, display globals,
  audio globals, shutdown globals, or the whole application context.
- Lifecycle: default command/key bindings are registered during startup; keymap
  files are loaded after paths are resolved; events are translated after each
  display/platform event pump; commands execute before frame mutation for that
  tick; UI edit state is saved during configuration persistence.
- Fits cleanly: none of the current candidate services fits cleanly without a
  split because both UI and keymap code currently combine registry and
  execution responsibilities.
- Probably belongs here: `KeymapRegistry` if reduced to input-to-command
  mapping; `InterfaceManager` if reduced to option-editing state and selection.
- Split-required: `KeymapRegistry::dispatch(...)` should become a separate
  `CommandDispatcher`; `InterfaceManager::renderOverlay(...)` crosses into
  Display; action definitions belong in command registration, not keymap data.
- Does not fit: scene changes, display commands, audio commands, and shutdown
  requests. Those are command targets supplied through `CommandContext`, not
  state owned by Commands And Input.
- Missing candidates: `InputQueue`, `CommandRegistry`, `CommandDispatcher`, and
  `CommandContext`.

Current Commands/Input status:

- Done since 73fd509: `InterfaceRuntime` is Application-owned and now holds the
  current interface, interface registry, runtime adapters, save/status flags,
  and scoped option/effect command context.
- Done since 73fd509: `Interface::head`, `Interface::current`,
  `currentOption`, `currentEffectControl`, and
  `currentOptionInterfaceElement` are gone from production code.
- Done in the current working tree: raw input now flows through
  Application-owned `InputQueue`, X11/panel input publishes through
  `InputEventSink`, key-name lookup is file-local immutable data, and
  `Keymap::setInterfaceRuntime(...)`/`Keymap::interfaceRuntime()` have been
  deleted. Key actions and interface rendering now receive
  `InterfaceRuntime&` explicitly through dispatch/display calls.
- Done in the current working tree: `ErrorMessages errors` has moved from a
  public Interface global to Application-owned overlay state passed explicitly
  into the X11 display coordinator.
- Done in the current working tree: Application no longer imports the public
  `interfaceMixer` global; mixer controls get the mixer panel through
  `InterfaceRuntime::find("Mixer")`.
- Done in the current working tree: `interfaceMain.extraKeymap` has moved into
  `InterfaceRuntime`; `setExtraKeymap` now mutates runtime-owned state.
- Done in the current working tree: `InterfaceHelp::pos`/`scrolling` has moved
  into `InterfaceRuntime`; help scroll actions and display now operate on
  runtime-owned state.
- Done in the current working tree: `InterfaceCredits::pos`/`firstTime` has
  moved into `InterfaceRuntime`; credits animation now updates runtime-owned
  state.
- Done in the current working tree: Help and Credits panel objects are created
  through `InterfaceRuntime::registerOwnedInterface(...)` instead of static
  process-lifetime panel instances.
- Done in the current working tree: Main and Mixer panel objects are also
  created through `InterfaceRuntime::registerOwnedInterface(...)`; there are no
  remaining `interfaceMain` or `interfaceMixer` panel globals.
- Done in the current working tree: EffectControls, Options, playlist, and
  effect-choice list panels are also created through
  `InterfaceRuntime::registerOwnedInterface(...)`; the named static interface
  panel objects have been removed.
- Done in the current working tree: default keymap loading creates named
  keymaps from `#keymap` sections, so `Keymap::init(...)` no longer relies on
  static `Keymap` objects and interface element/list code refers to keymap
  names instead of static `Keymap` members.
- Done in the current working tree: dead `Keymap::current` selected-keymap
  state and `Keymap::set(...)` have been deleted.
- Done in the current working tree: `Application` owns `KeymapRegistry` as
  concrete state; `Keymap::first`, static `Keymap::find(...)`, static
  `Keymap::readFile(...)`, static `Keymap::addList(...)`, static
  `Keymap::init(...)`, and static keymap-name dispatch have been removed from
  production. Interface input routing now receives `KeymapRegistry&`
  explicitly.
- Remaining: `Action::head`, raw `Action*` binding storage, and global action
  objects still need an explicit `CommandRegistry`, dispatcher, and command
  context. The stop-and-continue action no longer builds a continuation
  snapshot inside keymap; persistence now snapshots the current runtime config
  through `RuntimeConfigRegistry`.

#### Commands/Input Globals Removal Plan

Do not introduce a broad Commands/Input service layer during this pass.
Application can own the concrete input queue, keymap registry, command registry,
dispatcher, and interface runtime directly until the globals are gone. A module
facade can be added later over already-owned state. This pass should focus on
turning hidden process state into explicit objects and deleting the compatibility
escape hatches that would otherwise become permanent public API.

The global inventory tracked for this module is now:

- Removed in the current working tree: raw key state/configuration
  (`x11_key`, `key_esc`, `configureKeys(...)`, `keys_x11(...)`,
  `getkey_x11()`, and `getkey()`), and exported key-name lookup state
  (`keyAssoc`/`nKeyAssoc`).
- Removed in the current working tree: keymap registry state
  (`Keymap::first`, static `Keymap::find(...)`, static
  `Keymap::readFile(...)`, static `Keymap::addList(...)`, static
  `Keymap::init(...)`, and static keymap-name dispatch). `Application` now owns
  a concrete `KeymapRegistry`, keymap sections create named keymaps inside that
  registry, and interface input routing receives `KeymapRegistry&` explicitly.
- Command/action registry state still remaining: `Action::head`, raw `Action*`
  binding storage, and global action objects. Static `Keymap` instances from
  `Keymap::init(...)`, `InterfaceElementOption::keymap`,
  `InterfaceElementEffectControl::effectControlKeymap`, and
  `InterfaceList::listOptionKeymap` have been removed; keymap sections now
  create named keymaps during loading.
- Removed in the current working tree: runtime target bridges hidden behind
  keymap statics (`keymapRuntimeCommandSink`, `keymapInterfaceRuntime`,
  `Keymap::setRuntimeCommandSink(...)`, `Keymap::runtimeCommandSink()`,
  `Keymap::setInterfaceRuntime(...)`, and `Keymap::interfaceRuntime()`).
- Global action objects produced by the `ACTION(...)` macro in `keymap.cc`,
  `Interface.cc`, `InterfaceList.cc`, and `InterfaceHelp.cc`. These are
  registered by static construction. They now receive `InterfaceRuntime&`
  explicitly during dispatch, but they still need to become owned command
  definitions in `CommandRegistry`.
- Removed in the current working tree: static interface panel objects and their
  panel-owned state. The named panel instances now register through
  `InterfaceRuntime::registerOwnedInterface(...)`, while `ErrorMessages`,
  extra-keymap state, help scrolling state, and credits animation state are
  owned by Application/`InterfaceRuntime`.

The target call shape before adding a module facade is:

```c++
DisplayEventStats stats = displayRuntime.processEvents(inputEvents);
interfaceRuntime.runCurrent(inputEvents, keymaps, dispatcher, commandContext);
```

`inputEvents`, `keymaps`, `dispatcher`, and `commandContext` are
Application-owned collaborators. Display/platform code may publish raw input
events into a narrow input sink, but it must not know command targets. Keymaps
may translate keys into action invocations, but they must not execute commands.
Actions may execute only through an explicit `CommandContext` supplied for that
dispatch.

Migration sequence:

1. Add boundary tests and characterization before moving code.
   - Add focused tests for X11 key text translation, disabled/enabled ESC
     handling, shifted-number mapping, named-key lookup, keymap parsing, default
     binding loading, multi-action bindings, interface option editing, help/list
     actions, and quit/stop/preset command dispatch.
   - Extend `ConfigurationDependencyBoundaryTest` with temporary owner lists for
     the known globals. Each migration step should flip one owner from
     `assertSourceContains(...)` to `assertSourceDoesNotContain(...)`.
   - Record current allowed consumers for `Keymap::interfaceRuntime()` and
     `Keymap::runtimeCommandSink()` so new call sites are blocked immediately.

2. Introduce explicit input event storage without changing dispatch behavior.
   - Add an `InputEvent`/`InputQueue` pair that can store raw platform key
     events and return normalized Cthugha key codes in FIFO order.
   - Move `key_esc` into a `KeyTranslator` or `InputDecoder` instance built
     from `InputConfig`. `configureKeys(...)` becomes construction/configuration
     of that object rather than a write to process state.
   - Keep the existing key constants in `keys.h`, but remove extern mutable
     state from that header. The key-name table should become file-local
     immutable data behind a `KeyNameTable`/lookup helper.
   - Add an adapter only if needed to keep intermediate builds green:
     `getkey()` may temporarily drain the Application-owned queue through an
     explicitly installed queue pointer, but that adapter must have a boundary
     test and a deletion condition: delete when `Interface::run(...)` receives
     the queue directly.

3. Route display/platform input into the queue instead of `x11_key`.
   - Change the display event pump from `processEvents()` to
     `processEvents(InputEventSink&)` or equivalent. X11 should publish a raw
     key event containing the `XLookupString`/`XKeysymToString` text and modifier
     state; it should not write `x11_key`.
   - Delete `keys_x11(...)`, `getkey_x11()`, and the single-key `x11_key`
     mailbox once X11 and panel callbacks push into the explicit sink.
   - Preserve the existing run-loop order: pump display events, run the active
     interface once before frame generation, run/present a frame, then run the
     active interface again.

4. Split keymap storage from command execution.
   - Replace `Action::head` with an owned `CommandRegistry` containing named
     action definitions. Registration order must be explicit, deterministic, and
     driven by startup functions such as `registerDefaultKeyActions(...)`,
     `registerInterfaceKeyActions(...)`, `registerListKeyActions(...)`,
     `registerHelpKeyActions(...)`, and `registerCreditsKeyActions(...)`.
   - Replace the `ACTION(...)` macro's static self-registration with action
     objects or function adapters owned by `CommandRegistry`. Action
     implementations receive `(ActionInvocation, CommandContext&)`; they must
     not call `Keymap::runtimeCommandSink()` or
     `Keymap::interfaceRuntime()`.
   - Parse bindings into action names/parameters or action ids owned by
     `KeymapRegistry`; do not store raw `Action*` from static objects.
   - Keep keymap conditional parsing as pure parser support. It may remain in
     `keymap.cc` initially, but it should depend only on explicit feature flags
     and the supplied source text.

5. Make `KeymapRegistry` instance-owned.
   - Move `Keymap::first`, `Keymap::current`, static `find(...)`,
     static `readFile(...)`, static `addList(...)`, static `set(...)`, and
     static `action(...)` into an Application-owned `KeymapRegistry`.
   - Current working tree status: this ownership move is done for keymap
     storage/loading/name dispatch. `Application` owns `KeymapRegistry`, default
     keymaps and optional keymap files load through that instance, and
     `InterfaceRuntime`/interface panels receive the registry explicitly while
     handling input.
   - Default keymap definitions should be loaded by an explicit startup call:
     `keymaps.registerDefaults(defaultKeymapLines, commandRegistry)`, followed
     by `keymaps.loadFile(config.keymapFile, commandRegistry)`.
   - Static keymap objects such as `InterfaceElementOption::keymap`,
     `InterfaceElementEffectControl::effectControlKeymap`, and
     `InterfaceList::listOptionKeymap` become keymap names or registry handles
     resolved during interface setup.
   - Source-boundary target for this step: no `Keymap* Keymap::first`, no
     `Keymap* Keymap::current`, no static keymap loading/lookup methods, and
     no production `Keymap::action(const char*, ...)` call sites. `Action::head`
     remains tracked by the command-registry split in steps 4 and 6.

6. Add a `CommandDispatcher` and per-dispatch `CommandContext`.
   - `KeymapRegistry` should return action invocations for a key; it should not
     execute them.
   - `CommandDispatcher` executes those invocations with a `CommandContext`
     containing only the explicit targets needed by commands:
     `RuntimeCommandSink&`, `InterfaceRuntime&`, and scoped option/effect target
     data when an interface row is editing a value.
   - Runtime-changing actions (`quit`, `screenChg`, `soundProcess`,
     `writeIni`, `save`, `restore`, palette actions, etc.) build
     `RuntimeCommand` values and apply them through the context sink.
   - Interface-only actions (`up`, `down`, `home`, `end`, `toggleStatus`,
     `toggleSave`, `setInterface`, `nextInterface`, `prevInterface`,
     `setExtraKeymap`) mutate only `InterfaceRuntime` through context methods.
   - Scoped option/effect actions (`chgValue1`, `chgValue2`, `chgValue3`,
     `setValue`, `lockElement`, `toggleUse`, `activate`) use the same scoped
     command data currently held by `InterfaceRuntime`, but passed as part of
     the dispatch context instead of fetched through a global keymap pointer.

7. Make interface input explicit.
   - Change `InterfaceRuntime::runCurrent()` to receive `InputQueue&`,
     `KeymapRegistry&`, `CommandDispatcher&`, and a base `CommandContext&`, or
     add a narrow `InterfaceInputRunner` helper owned by Application that calls
     into `InterfaceRuntime`. This is still concrete wiring, not a service
     layer.
   - Change `Interface::run(...)` so it drains the supplied queue instead of
     calling `getkey()`.
   - Change `Interface::doKey(...)`, `InterfaceMain::doKey(...)`,
     `InterfaceEffectControl::doKey(...)`, `InterfaceList::doKey(...)`,
     `InterfaceElementOption::doKey(...)`, and
     `InterfaceElementEffectControl::doKey(...)` to dispatch through explicit
     registry/dispatcher arguments or through `InterfaceRuntime` methods that
     receive those collaborators.
   - Delete `Keymap::setInterfaceRuntime(...)` and
     `Keymap::setRuntimeCommandSink(...)` once no action or interface code uses
     them.

8. Move concrete interface panel state out of globals.
   - Convert `registerDefaultInterfaces(...)`, `registerListInterfaces(...)`,
     `registerHelpInterface(...)`, `registerCreditsInterface(...)`, and
     `registerAudioInterfaces(...)` from borrowing static objects to creating
     `InterfaceRuntime`-owned panel instances.
   - Replace the public `interfaceMixer` global with a lookup result from
     `InterfaceRuntime`, so `MixerControls::installInto(...)` and
     `clearInterface(...)` receive the mixer panel through explicit Application
     wiring.
   - Move `interfaceMain.extraKeymap` into `InterfaceRuntime` or an owned main
     panel instance. The `setExtraKeymap` action should mutate that owned state
     through `CommandContext`.
   - Move help scrolling state into the owned help panel instance and register
     help actions against that instance during setup.
   - Move `ErrorMessages errors` into `InterfaceRuntime` or an owned interface
     overlay model. Display overlay production should receive an explicit
     `OverlayProducer`/interface overlay source from Application instead of
     reading `errors` or `Keymap::interfaceRuntime()` globally.

9. Tighten boundaries and delete compatibility code.
   - `keys.h` should expose key constants and small value types only; no
     mutable externs, no `getkey()`, and no X11-specific translation function.
   - `keymap.h` should expose the owned registry/definition types only; no
     static dispatch, static selected keymap, or runtime target accessors.
   - `Interface.h` should expose interface definitions and runtime-owned
     registration helpers only; no public panel globals.
   - Boundary tests should assert that `x11_key`, `key_esc`, `Action::head`,
     `Keymap::first`, `Keymap::current`, `Keymap::interfaceRuntime`,
     `Keymap::runtimeCommandSink`, `getkey()`, `keys_x11(...)`,
     `interfaceMixer`, `interfaceHelp`, `interfaceCredits`, `interfaceMain`,
     and `ErrorMessages errors` do not appear in production headers or source
     outside deleted-history comments.

10. Wrap the cleaned concrete parts behind one coherent module API.
    - Add this only after steps 2-9 have removed the mutable globals and static
      compatibility accessors. The API should consolidate already-owned
      concrete parts; it must not become a new home for legacy adapter plumbing
      or delayed cleanup.
    - Treat this as a boundary-polishing step over working concrete objects,
      not as a rescue layer. By the time this starts, raw input should already
      flow into `InputQueue`, key decoding should already be instance-owned,
      keymaps should already produce action invocations, command definitions
      should already live in `CommandRegistry`, and interface panels/overlay
      state should already be owned by `InterfaceRuntime` or another explicit
      collaborator.
    - Introduce a Commands/Input root, for example `CommandsInputRuntime` or
      `InputCommandRuntime`, owned by `Application`. It should own the
      `InputQueue`, `KeyTranslator`, `CommandRegistry`, `KeymapRegistry`,
      `CommandDispatcher`, `CommandContext` factory/state, and
      `InterfaceRuntime`/owned panels.
    - Define the public surface around module workflows and ports, not around
      exposing the pieces. Public callers should see input ingress, startup
      registration/loading, per-tick input execution, interface selection, and
      overlay production. The concrete registries, dispatcher, panels, and edit
      state should become private implementation details of the module root.
    - Keep the public surface verb-shaped and module-shaped. External code
      should be able to configure input, load keymaps, register the built-in
      command/interface definitions, accept platform input, run pending input,
      run/select the active interface, and provide overlay text or overlay draw
      commands. It should not fetch registries and panels to assemble those
      workflows itself.
    - Expose narrow ports rather than exposing the object graph. Display should
      receive an `InputEventSink&` or `PlatformInputPort&`. Runtime command
      execution should enter through a supplied `RuntimeCommandSink&` or a
      dispatch call that receives one. Overlay rendering should see an
      `InterfaceOverlaySource&`/`OverlayProducer&`, not `ErrorMessages` or an
      interface panel global. Tests can use explicit fixture builders instead of
      singleton back doors.
    - A plausible first public surface is:

      ```c++
      class CommandsInputRuntime {
      public:
        void configureInput(const InputConfig&);
        void registerBuiltins(const CommandsInputBuiltins&);
        void loadKeymaps(const PathConfig&);
        InputEventSink& inputSink();
        void runPendingInput(RuntimeCommandSink&);
        void runCurrentInterface(RuntimeCommandSink&);
        void selectInterface(const char* name);
        InterfaceOverlaySource& overlaySource();
      };
      ```

      Temporary accessors such as `mixerInterface()` are acceptable only if the
      next migration still needs a concrete panel during setup. They should have
      a named deletion condition in the plan or in the boundary test.
    - Make dependency direction visible in the signatures. Platform/display
      code can push input and ask for overlay output; runtime code can supply a
      `RuntimeCommandSink&` for command effects; configuration code can supply
      `InputConfig`, keymap paths, and built-in definition bundles. None of
      those customers should reach inward for `KeymapRegistry`,
      `CommandRegistry`, `InterfaceRuntime`, or panel-specific objects.
    - Move to this root in small mechanical slices: first move the concrete
      Application fields into the root without changing behavior, then move
      registration/setup functions behind `registerBuiltins(...)`, then replace
      event-loop call sites with `inputSink()`, `runPendingInput(...)`, and
      `runCurrentInterface(...)`, and finally delete production includes of the
      concrete registry/panel headers outside the module.
    - Keep internal services private. Callers should not fetch
      `KeymapRegistry&`, `CommandRegistry&`, `CommandDispatcher&`, concrete
      panel objects, or mutable interface internals unless a test fixture or
      short-lived migration adapter explicitly requires it.
    - Boundary tests should flip from checking individual concrete ownership in
      `Application` to checking that production customers depend on the module
      root or one of its narrow ports. `Application` may construct the root, but
      display, interface panels, key actions, and runtime command code should
      not know the root's internal object graph.

Completion criteria for this module:

- Display/platform input reaches Commands/Input only through an explicit queue
  or sink.
- Key decoding is instance-owned and configured by `InputConfig`.
- Keymaps are owned by an explicit registry and produce action invocations
  without command side effects.
- Commands execute through a supplied `CommandContext`; no command reaches
  scene, display, audio, shutdown, runtime persistence, or interface state
  through process globals.
- Interface panels and UI overlay/error state are owned by `InterfaceRuntime`
  or another explicit Application-owned object.
- The source-boundary test has no remaining allowlist for Commands/Input
  mutable globals.

### Scene Module

- Boundary: Owns scene configuration, effect/control registration, visual
  catalogs, current scene selection, preset state, scene-change policy, and
  message policy that is semantic rather than display-specific.
- Owns/replaces: visual effect selection globals, effect-control registry
  globals, preset catalog globals, visual catalog/list globals, scene command
  bridges, and the policy portion of automatic scene changes.
- Inputs: validated configuration, command intents, random source, clock/timing
  source, and audio analysis snapshots.
- Outputs: immutable or snapshotted `SceneSettings`, scene-change requests,
  effect/control updates, and render-ready scene descriptions for Frame
  Mutation.
- Allowed dependencies: Configuration option views, process clock/random
  interfaces, Audio analysis snapshots, and command targets.
- Forbidden dependencies: direct buffer mutation, display presentation, audio
  runtime frame storage, or display backend geometry globals.
- Lifecycle: catalogs/effects register after configuration defaults exist;
  startup configuration selects the initial scene; commands and auto-change
  policy may update scene state between frames; a scene snapshot is handed to
  Frame Mutation before rendering.
- Fits cleanly: `EffectRegistry`, `VisualCatalogs`.
- Probably belongs here: `AutoChanger` if renamed/reframed as
  `SceneChangeScheduler`; `SceneCommands` if it becomes a narrow command target
  over scene state rather than a compatibility bridge.
- Split-required: `VideoDirector` currently spans scene control, frame
  composition, display presentation, message overlays, timing, and palette
  smoothing. Only scene state and scene transitions belong here. Visual
  selections currently inside `AppOptions` should move into explicit scene
  state or a scene configuration view.
- Does not fit: audio module wrappers; audio should publish analysis snapshots
  and Scene should consume them through policy.
- Missing candidates: `SceneController`, `SceneState`, `SceneSnapshot`,
  `SceneChangeScheduler`, and `SceneSerializer`.

### Audio Module

- Boundary: Owns audio input/output runtime, audio frame acquisition, audio
  processing, analysis, audio device/mixer state, and audio dump writing.
- Owns/replaces: `AudioRuntime.cc` file-scope state, audio frame facade
  functions, silent fallback frame globals, audio analyzer/metrics/acoustic
  globals, audio processor globals, mixer globals, and audio dump globals.
- Inputs: startup audio configuration, resolved audio paths/devices, clock for
  synchronization, and lifecycle/shutdown requests.
- Outputs: `AudioFrame` snapshots, processed audio products, and
  `AudioAnalysisSnapshot` values for Scene and Frame Mutation.
- Allowed dependencies: audio options, path resolver, clock, logger, shutdown
  requester, and platform audio/file adapters.
- Forbidden dependencies: scene commands, display presentation, render buffers,
  or auto-change policy.
- Lifecycle: configured after Configuration; started before first visual frame;
  polled or advanced once per application tick; analysis snapshot is produced
  before scene policy and frame mutation; stopped before display teardown.
- Fits cleanly: low-level `PcmSource` implementations, `AudioBuffer`, and
  `AudioFrameBuilder` as Audio-owned implementation details.
- Probably belongs here: raw-only `AudioFrameProvider`,
  `AudioProcessingPipeline`, `AcousticContextTracker`, `AudioDumpWriter`,
  mixer setup, and silent frame provider.
- Split-required: `AudioFrameProvider` should expose raw/current frames; the
  processed wave belongs to an audio processing pipeline or frame product.
  `AudioAnalysisState::minimumNoiseSatisfied(...)` is Scene policy and should
  leave Audio. `AudioSystem` and `AudioRuntime` should split into acquisition,
  passthrough, processing, analysis, output dump, and mixer/device adapters.
- Does not fit: `AutoChanger` as an audio policy.
- Missing candidates: `AudioAcquisitionRuntime`, `AudioPassthrough`,
  `AudioProcessingPipeline`, `AudioAnalysisSnapshot`, `AudioFrameProducts`,
  `AudioDeviceConfig`, and `AudioDumpWriter`.

#### Audio Module Findings

The Audio module should be treated as a pipeline with five separate
responsibilities. They are related, but they should not collapse into one
runtime object:

1. Audio generation
   - Purpose: Produce PCM without depending on external live input. This covers
     random audio and decoded file-backed audio.
   - Current code: `RandomNoisePcmSource`, `WavPcmSource`, `Minimp3PcmSource`,
     `RawPcmSource`, `PcmSourceFactory`.
   - Output: interleaved PCM plus `PcmFormat`.
   - Dependencies to inject: audio source config, resolved file paths,
     `RandomSource&` for random generation, and `Logger&`.
   - Must not know: display geometry, visual effect state, auto-change policy,
     or output/playback devices.

2. Audio acquisition
   - Purpose: Turn one selected PCM source, whether generated/file/live, into
     the raw audio frame for the current visual tick.
   - Current code: `AudioIngest`, `DecodedAudioHistory`,
     `AudioFrameBuilder`, and low-level `PcmSource` implementations.
     Formerly this was concentrated in `AudioRuntime.cc`,
     `AudioInputProcessor`, and `AudioBuffer`.
   - Output: `AudioFrame` or future `RawAudioFrameSnapshot` containing signed
     8-bit stereo samples, sample count, center sample, format metadata, and
     silence/completion status.
   - Dependencies to inject: selected `PcmSource`, clock or sample-position
     source, audio format/window config, logger, and shutdown requester for
     fatal/completion handoff.
   - Must not know: selected visual processor, acoustic policy, scene commands,
     display presentation, or global frame facades.

3. Audio passthrough
   - Purpose: Optionally play acquired/generated PCM through an output backend.
   - Current code: `AudioPassthrough`, `AudioOutput`, `AudioNullOutput`,
     `AudioPulseOutput`, `AudioDSPOutput`, `AudioOutputDump`, and submitted-PCM
     debug reporting owned by output instances.
   - Output: submitted PCM and completion/drain status.
   - Dependencies to inject: output config, output backend factory, clock for
     diagnostics/pacing, logger, optional `AudioDumpWriter`, and the PCM buffer
     it drains.
   - Must not know: visual frame processing, acoustic analysis, or scene
     changes.
   - Note: generated/file PCM can be passed through; live-input passthrough
     should be an explicit loopback mode, not a side effect of acquisition.

4. Audio processing for visuals
   - Purpose: Convert one raw per-frame audio snapshot into the data the visual
     domain needs for rendering.
   - Current code: `AudioFramePipeline`, `AudioProcessingSelector`,
     `AudioProcessingState`, owned `AudioProcessor`, and
     `AudioFftProcessor`, writing explicitly into `AudioFrame::processedWaveData`.
     Formerly this mixed `AudioProcessingOption`, the global `audioProcessing`,
     global FFT tables, and the audio-frame facade.
   - Output: future `VisualAudioFrame` or `ProcessedAudioFrame` containing the
     processed wave data used by waves, zick, borders, and other renderers.
   - Dependencies to inject: processing mode/config, owned FFT tables or
     immutable lookup tables, and the raw audio frame.
   - Must not know: acquisition runtime, output passthrough, global option
     registry, display backend, or scene auto-change policy.

5. Acoustic context
   - Purpose: Maintain a stateful view of what the music is doing over time.
     This is slower semantic audio state, not renderer sample data.
   - Current code: `AudioFrame` owns per-frame `AudioMetrics`,
     `DefaultAudioFramePipeline` analyzes each frame, and `Application` owns the
     rolling `AcousticContext`. Formerly this mixed `AudioAnalyzer`,
     `sound_minnoise`, global `audioAnalyzer`, global `audioMetrics`, and
     global `acousticContext`.
   - Output: future `AudioAnalysisSnapshot` containing frame metrics,
     smoothed intensity, attack/fire, cumulative fire, and noisy/quiet status.
   - Dependencies to inject: raw audio frame, acoustic-analysis config such as
     minimum noise threshold, and logger for bounded diagnostics.
   - Must not know: `AutoChanger`, scene-change thresholds, visual filterchain
     refresh, or display rendering.

The intended per-frame data flow is:

```text
PcmSource
  -> AudioAcquisitionRuntime
  -> RawAudioFrameSnapshot
      -> AudioProcessingPipeline -> ProcessedAudioFrame
      -> AcousticContextTracker  -> AudioAnalysisSnapshot

AudioAcquisitionRuntime -> optional AudioPassthrough -> AudioOutput/AudioDump

Scene receives AudioAnalysisSnapshot.
Frame Mutation receives RawAudioFrameSnapshot, ProcessedAudioFrame, and
AudioAnalysisSnapshot through its frame context.
Display receives pixels only.
```

Original audio smells and current status:

- Done: `AudioRuntime.cc` file-scope ownership was replaced by
  `AudioIngest`, `DecodedAudioHistory`, `AudioPassthrough`, and owned output
  objects.
- Done: the `AudioFrame.cc` global facade was removed; `Application` now
  advances ingest and hands an owned `AudioFrame` into the visual frame path.
- Done: `AudioProcessor.cc` no longer owns option/catalog selection or global
  FFT tables; sound-processing mode is owned by `AudioProcessingState`, and FFT
  work is behind `AudioFftProcessor`.
- Done: `AudioAnalyzer.cc` no longer publishes global metrics/context or owns
  `sound_minnoise`; per-frame metrics live on `AudioFrame`, and
  `AcousticContext` is Application-owned.
- Done in current working tree: per-frame audio processing/analysis moved into
  `AudioFramePipeline`/`DefaultAudioFramePipeline`, which processes the frame,
  publishes frame metrics, and updates `AcousticContext`.
- Done in current working tree: the intermediate audio-to-visual wrapper was
  deleted. Remaining follow-up is to move AutoChanger behavior from
  `Application` into a Scene-owned `SceneChangeScheduler`.
- Remaining: audio frame-window sizing is explicit at the Application
  composition root, but it still depends on display/frame geometry until Frame
  Mutation owns that boundary.

Audio module objects should be split this way:

- `PcmSource`: existing interface, kept as the low-level PCM producer. It
  should be constructed through an injected source factory using explicit
  config and paths.
- `AudioAcquisitionRuntime`: owns selected source, buffering, sample-position
  alignment, frame building, live/file strategy differences, and completion
  state.
- `AudioPassthrough`: owns optional output draining, output backend state, and
  dump writer integration.
- `AudioFrameProvider`: raw-frame-only view over acquisition output. It should
  not expose processed wave data.
- `AudioProcessingPipeline`: pure-ish per-frame processor from raw frame plus
  processing mode to processed visual audio.
- `AcousticContextTracker`: stateful analyzer from raw frame plus acoustic
  config to `AudioAnalysisSnapshot`.
- `AudioModule`: module root owned by `Application`; it wires the above
  objects and exposes narrow interfaces, not global facades.

The audio refactors added tests around these seams; keep extending that style
for the remaining Scene policy cleanup:

- Service tests: deterministic `PcmSource` generation, raw frame building from
  a fixture PCM stream, passthrough drain/completion, each visual processing
  mode, acoustic metrics/intensity/fire/cumulative-fire behavior, and silent
  fallback behavior.
- Dependent tests: `Application` advances audio through an injected module;
  frame context creation receives explicit raw/processed/acoustic snapshots;
  `AutoChanger` or its replacement consumes only `AudioAnalysisSnapshot`;
  `display.cc`, `Border.cc`, and waves consume audio only through frame context.
- Boundary tests: forbid new production uses of `audioFrameCurrent()`,
  `audioFrameRawData()`, `audioFrameProcessedWaveData()`, `audioFrameTick()`,
  `audioFrameChange()`, `audioAnalyzer`, `audioMetrics`, `acousticContext`,
  `audioProcessing`, `sound_minnoise`, and `audioRuntime*` outside named
  compatibility adapters while the migration is active.

Current Audio status since 73fd509:

- Done: `AudioIngest` replaced the public `AudioRuntime` facade and owns
  acquisition, decoded history, visual pacing, current frame, completion, and
  suspend/resume lifecycle.
- Done: `DecodedAudioHistory` and `AudioOutputStream` split acquisition
  history from passthrough output cursors.
- Done: `AudioPassthrough` and output backends drain PCM without deciding the
  current visual audio frame.
- Done: `AudioFrame` now owns raw/processed per-frame data and per-frame
  metrics; global audio-frame facade functions are gone.
- Done: `AudioProcessor` absorbed per-frame analysis, delegates FFT through
  `AudioFftProcessor`, and no longer owns global FFT tables.
- Done: `AudioProcessingState`/`AudioProcessingSelector` own sound-processing
  mode; runtime audio controls mutate the selector.
- Done: `AudioFramePipeline`/`DefaultAudioFramePipeline` own per-frame audio
  processing, analysis, acoustic-context updates, and bounded debug reporting.
- Done: `AcousticContext` is Application-owned and passed explicitly through
  frame contexts.
- Done: startup/device audio configuration and output configuration are
  explicit (`AudioConfig`, `AudioSettings`, `PcmFormat`,
  `AudioOutputConfig`, `AudioOutputDump`).
- Done: OSS mixer state, controls, and diagnostics are owned by
  `MixerSession`, `MixerDevice`, `MixerControls`, and an explicit `LogSink`;
  mixer option label text is instance-owned rather than static shared state.
- Done: `AutoChanger` timing/random inputs, quiet-message observation,
  diagnostics, and status text storage are explicit collaborators/state
  supplied by `Application`.
- Done: submitted-PCM and Pulse underflow debug throttles are instance-local.
- Remaining: move the remaining Application-owned AutoChanger policy into
  Scene-owned auto-change scheduling; keep driving display/frame consumers from
  explicit `VideoFrameContext` until Frame Mutation owns that boundary.

Historical removal order inside Audio and its status:

1. Done in current shape: introduce explicit data products:
   `RawAudioFrameSnapshot`,
   `ProcessedAudioFrame`, `AudioAnalysisSnapshot`, and an aggregate
   `AudioFrameProducts` for one visual tick. The names changed: the current
   product is `AudioFrame` plus `VideoFrameContext`/`ScreenRenderContext`
   analysis pointers rather than separate snapshot classes.
2. Done in current shape: extract `AudioProcessingPipeline` from
   `AudioProcessor.cc` so processing
   works on explicit raw/processed buffers without touching the frame facade or
   option globals. The name used in code is `AudioProcessingSelector` plus the
   owned `AudioProcessor` instance.
3. Done in current shape: extract `AcousticContextTracker` from
   `AudioAnalyzer.cc` so analysis owns no
   globals and receives minimum-noise config explicitly.
4. Done in current shape: introduce `AudioFramePipeline` as the audio-only
   per-frame pipeline. Current working-tree follow-up deleted the intermediate
   wrapper; remaining follow-up is moving automatic scene-change ownership from
   `Application` into Scene.
5. Done in current shape: convert `AudioFrameProvider` to raw-frame-only and pass raw/processed/audio
   analysis products into `VideoFrameContext`.
6. Done: split `AudioRuntime.cc` into acquisition and passthrough owners, preserving
   the current file/generator and live-input strategies.
7. Done for production audio facades: remove audio facade functions and global audio state once all consumers take
   injected audio products or module interfaces.

### Frame Mutation Module

- Boundary: Owns mutation of indexed frame buffers and renderer-local state. It
  turns scene snapshots plus audio/frame inputs into pixels.
- Owns/replaces: `CthughaBuffer::buffer`, `CthughaBuffer::current`,
  `display.cc` renderer statics, screen renderer state, translation/image
  working buffers, filterchain diagnostic/runtime counters such as
  `FrameCommitFilter` debug throttling, and mutable math/render lookup tables
  where they are render implementation details.
- Inputs: `SceneSnapshot`, `AudioFrame` or `AudioAnalysisSnapshot`, display
  dimensions, frame timing, and renderer configuration.
- Outputs: `IndexedFrame` or frame view ready for Display presentation.
- Allowed dependencies: Scene snapshots, Audio snapshots, display dimensions,
  clock/frame delta, and renderer-local random source where needed.
- Forbidden dependencies: display backend/device state, key/action state,
  global audio facades, scene option globals, or persistence.
- Lifecycle: buffer storage is sized after display/config resolution; renderer
  state is initialized for the selected scene; each tick mutates the next frame;
  the completed frame is handed to Display; renderer state is reset on scene or
  size changes.
- Fits cleanly: `CthughaBuffer` as owned state, not really as a service.
- Probably belongs here: `PresentationComposer`, `ScreenRenderContext`,
  renderer state factories, `MathTables` if tables are render-owned.
- Split-required: `VideoDirector` composition belongs here, but scene
  transition policy and display presentation do not. `AudioProcessor` FFT state
  belongs in Audio unless it is purely a render lookup table.
- Does not fit: `SceneCommands`, `DisplayRuntimeOwnership`, `AudioSystem`, or
  `KeymapRegistry`.
- Missing candidates: `FrameStore`, `FrameComposer`, `RendererRegistry`,
  `RendererStateFactory`, and `FrameMutationContext`.

### Display Module

- Boundary: Owns platform/window backends, display devices, geometry, native
  pixel transfer, overlays as rendered output, and raw event collection.
- Owns/replaces: `cthughaDisplay`, `displayDevice`, `displayBackend`,
  `displayRuntime`, X11 globals, display geometry globals, native pixel tables,
  overlay text/error globals, and global overlay device swapping.
- Inputs: display configuration, frame views from Frame Mutation, overlay
  messages, and platform events.
- Outputs: presented frames, raw input events for Commands And Input, display
  geometry snapshots, and backend lifecycle status.
- Allowed dependencies: display options, path resolver for fonts, logger
  diagnostics, platform/X11 adapters, and an input event sink.
- Forbidden dependencies: scene catalogs, audio runtime, key command dispatch,
  option mutation, or direct shutdown globals.
- Lifecycle: configured after startup configuration; opened before audio/video
  frame flow starts; event-pumped once per tick before command dispatch;
  presents after Frame Mutation; closes after audio/frame flow stops.
- Fits cleanly: none of `DisplayRuntimeOwnership` fits cleanly as currently
  drawn because it combines too many display concerns.
- Probably belongs here: display facade/device/backend ownership, geometry,
  native pixel transfer, and overlay sink after splitting.
- Split-required: `DisplayRuntimeOwnership` should become `DisplaySystem`,
  `DisplayBackend`, `DisplayGeometry`, and `OverlaySink`. `Logger` must not
  point at display overlays; overlays should subscribe to messages or receive
  explicit message output.
- Does not fit: input command dispatch, scene message policy, or frame
  composition.
- Missing candidates: `DisplaySystem`, `DisplayBackend`, `DisplayGeometry`,
  `OverlayMessageQueue`, and `OverlaySink`.

### Service Placement Audit

- Fits cleanly: `ShutdownController` -> Application Lifecycle;
  `PlatformLifecycle` -> Application Lifecycle; `EffectRegistry` -> Scene;
  `VisualCatalogs` -> Scene; low-level `PcmSource` implementations,
  `AudioBuffer`, and `AudioFrameBuilder` -> Audio; `CthughaBuffer` -> Frame
  Mutation as state.
- Probably belongs: `Clock` -> Application Lifecycle; `RandomSource` ->
  Application Lifecycle; `Logger` -> Application Lifecycle/Diagnostics;
  `PathConfig` -> Configuration; `AutoChanger` -> Scene as
  `SceneChangeScheduler`; `SceneCommands` -> Scene command target; raw-only
  `AudioFrameProvider`, `AudioProcessingPipeline`, and
  `AcousticContextTracker` -> Audio; display backend/device/geometry pieces ->
  Display.
- Split across modules: `AppOptions`, `IniStore`, `VideoDirector`,
  `DisplayRuntimeOwnership`, `InterfaceManager`, `KeymapRegistry`,
  `AudioAnalysisState` as currently written, `AudioFrameProvider` as currently
  written, `AudioSystem`, and `AudioRuntime`.
- Does not fit as a service: `ApplicationContext`, one-off bridge wrappers,
  `CthughaBuffer` if treated as a peer service rather than frame storage, and
  any logger-to-overlay coupling.
- Missing from the original service list: `InputQueue`, `CommandRegistry`,
  `CommandDispatcher`, `CommandContext`, `SceneController`, `FrameComposer`,
  `DisplaySystem`, `PathResolver`, and `ConfigLoader`.

## ApplicationContext Service Contracts

This section is a service-contract audit, not the final module design. Any
service marked split-required above should be decomposed before implementation;
any service marked "probably belongs" needs an explicit boundary test before it
is allowed to settle in that module.

Status note since 73fd509: some contracts below landed under narrower names.
`ShutdownController` landed as `RuntimeShutdown`/`RuntimeCloseState`;
`InterfaceManager` partially landed as `InterfaceRuntime`; `AudioSystem`
split into `AudioIngest`, `AudioPassthrough`, output/config/dump classes,
`AudioProcessingState`/`AudioProcessingSelector`, and mixer owners rather than
one service. Keep the contracts below as shape guidance, not proof that the
old names still need to be implemented.


### `ShutdownController`

- Purpose: Own the application shutdown request state and the reason for
  termination.
- Globals replaced: `cthugha_close` and direct increments from key actions,
  audio completion, X11 quit, credits, and resume failure paths.
- Used by: `Application::run()`, key/action dispatch, X11 event handling,
  audio-runtime completion, credits/help sequences, and platform lifecycle
  polling.
- Provided to customers: Constructed by `ApplicationContext`; passed as
  `ShutdownController&` to coordinators and as a narrower
  `ShutdownRequester&` to code that may only request shutdown.
- Lifecycle timing: Created before option parsing so command-line errors can
  request an orderly exit; read before each frame wait, after each event pump,
  after audio advancement, after display presentation, and during final teardown.
- References contained: No external service references; owns an atomic or plain
  request flag plus optional enum/string reason.
- API surface: `request(ShutdownReason)`, `requested() const`,
  `reason() const`, `clearForTest()`, and optionally
  `throwIfRequested()` for startup paths that cannot continue.

### `Clock`

- Purpose: Provide monotonic application time, frame delta calculation, and
  testable timers without direct wall-clock calls.
- Globals replaced: `gettime()`, `getTime()`, `now`, `deltaT`,
  `CthughaDisplay.cc` static `FrameClock`, `Application.cc` static
  `FramePacer`, and ad hoc `time(0)` seeding.
- Used by: `Application::run()`, frame pacing, `CthughaDisplay`, audio runtime,
  display tracing, message/error expiry, credits/help animation, auto-change
  policy, and tests that need deterministic time.
- Provided to customers: `ApplicationContext` owns a concrete
  `SystemClock`; dependents receive `Clock&` or a `FrameTimer&` facade.
- Lifecycle timing: Created at process startup before random seeding; sampled
  once at startup, before each frame is composed, after event/audio work, before
  frame pacing sleep, and during shutdown diagnostics.
- References contained: No application references; may contain a steady-clock
  epoch, last-frame timestamp, and injected sleeper for tests.
- API surface: `now()`, `secondsSinceStart()`, `beginFrame()`,
  `frameDelta() const`, `sleepUntilFrameDeadline(FrameRateLimit)`, and
  `makeTimer(Duration)`.

### `RandomSource`

- Purpose: Centralize all nondeterminism and make random-dependent behavior
  reproducible in tests.
- Globals replaced: `srand(time(0))`, `rand()`, and `Random()`.
- Used by: option randomization, `AutoChanger`, palette generation, wave and
  flame setup, message choice, random audio, and any effect that currently
  reaches for process random state.
- Provided to customers: `ApplicationContext` owns the generator; dependents
  receive `RandomSource&`, while pure functions receive generated values where
  practical.
- Lifecycle timing: Seeded after `Clock` and before option/catalog
  initialization; used during initial random option resolution, scene changes,
  every frame that needs random visual/audio data, and tests with fixed seeds.
- References contained: Optional `Clock&` only for production seed creation;
  otherwise owns generator state and seed metadata.
- API surface: `seed(uint64_t)`, `seedFromClock(Clock&)`, `nextUInt()`,
  `uniformInt(min, max)`, `uniformFloat(min, max)`, `chance(probability)`, and
  `shuffle(range)`.

### `Logger`

- Purpose: Own diagnostics, verbosity, and structured application messages.
- Globals replaced: the process-wide logging verbosity switch and direct
  `CTH_*` macro dependency on global state.
- Used by: application startup, option/ini loading, display backend, audio
  runtime, resource loading, effect registration, and tests that assert warning
  behavior.
- Provided to customers: `ApplicationContext` owns a `Logger`; dependents
  receive `Logger&` or `LogSink&`. Macros become thin adapters during migration.
- Lifecycle timing: Created first, before option parsing; verbosity is updated
  after command-line and ini parsing; used throughout startup, per-frame
  diagnostics, error overlays, and teardown.
- References contained: Output sink, verbosity level, optional overlay message
  sink reference once display exists.
- API surface: `setVerbosity(int)`, `verbosity() const`, `trace(category,msg)`,
  `info(msg)`, `warn(msg)`, `error(msg)`, and `withSink(LogSink&)`.

### `PathConfig`

- Purpose: Own resolved filesystem paths and path-search policy separate from
  the option parser.
- Globals replaced: `extra_lib_path`, `ini_file_override`,
  `Keymap::keymapFile`, audio input/output filenames, and scattered device/path
  strings currently living as globals.
- Used by: ini loading/saving, resource loaders, keymap loading,
  audio input/output setup, and tests that need temporary resource roots.
- Provided to customers: Built by option parsing into `ApplicationContext`;
  passed as `const PathConfig&` to loaders and mutable `PathConfig&` only to
  parsers.
- Lifecycle timing: Defaults exist before command-line parsing; resolved after
  command line and before ini/resource/keymap/audio setup; read during startup
  and on explicit save/export actions.
- References contained: No service references; owns normalized path strings,
  search roots, and resolved file policy.
- API surface: `libraryPaths() const`, `iniOverride() const`,
  `displayDumpPath() const`, `keymapPath() const`, `audioInputFile() const`,
  `audioOutputDump() const`, `resolveResource(name)`, and setters used only by
  parsers/builders.

### `AppOptions`

- Purpose: Own scalar and string configuration as explicit state, grouped by
  subsystem.
- Globals replaced: core options, audio options, display options,
  auto-change/message options, and feature toggles listed in the option
  inventory.
- Used by: command-line parser, ini store, display runtime setup, audio system
  setup, video director, auto changer, visual catalog defaults, key actions,
  and UI option editors.
- Provided to customers: `ApplicationContext` owns one `AppOptions`; most
  dependents receive narrower views such as `AudioOptions&`, `DisplayOptions&`,
  `AutoChangeOptions&`, or `const AppOptions&`.
- Lifecycle timing: Defaults are constructed before parsing; command line and
  ini mutate it during startup; runtime UI/key actions mutate allowed live
  options between frames; teardown reads save/persistence settings.
- References contained: Owns value groups and may contain `PathConfig` by value
  or reference; should not reference runtime services.
- API surface: typed accessors for `core()`, `audio()`, `display()`,
  `autoChange()`, `messages()`, `paths()`, `visualSelections()`,
  `setFromOption(name,value)`, `snapshot()`, and validation helpers.

### `EffectRegistry`

- Purpose: Own the registered effect controls and their lifecycle operations.
- Globals replaced: `EffectControl::first`, `effectPresetCatalog` traversal of
  hidden registration state, and effect save/restore/random iteration through
  process globals.
- Used by: `Application` startup, option parsing, ini persistence,
  `SceneCommands`, preset loading/saving, random change actions, and UI option
  editors.
- Provided to customers: `ApplicationContext` constructs the registry before
  effect/control definitions are registered; constructors or registration
  functions receive `EffectRegistry&`.
- Lifecycle timing: Created before visual catalog initialization; populated
  during startup registration; read and mutated during option parsing, scene
  changes, randomization, per-frame option updates, and ini save on teardown.
- References contained: Owns or references registered `EffectControl` objects;
  may reference `RandomSource&` only for operations that randomize.
- API surface: `registerControl(EffectControl&)`, `find(name)`, `all()`,
  `changeOne(name,delta)`, `changeAll(ChangeRequest)`, `randomize(RandomSource&)`,
  `save(IniWriter&)`, `restore(IniReader&)`, and `presets()`.

### `VisualCatalogs`

- Purpose: Own named visual/audio-effect catalogs and resolve selected scene
  entries without global lists.
- Globals replaced: `paletteEntries`, `screenEntries`, `_flames`, `_objects`,
  wave/object/table lists, audio processor entries, `screenCatalog`,
  `flameCatalog`, `waveCatalog`, and related exposed catalogs.
- Used by: `SceneCommands`, option parsing, UI option panels, key actions,
  preset loading, image/table/translation setup, and tests that build small
  catalog fixtures.
- Provided to customers: `ApplicationContext` owns a `VisualCatalogs`;
  consumers receive `const VisualCatalogs&` for lookup and a builder/mutable
  reference only during startup registration.
- Lifecycle timing: Constructed after `EffectRegistry` and `AppOptions`
  defaults; populated during startup; queried during parsing and every scene
  change; stable during frame rendering except for explicit runtime catalog
  reload features if kept.
- References contained: References to registered effect controls and immutable
  catalog entries; no direct display/audio runtime references.
- API surface: `screens()`, `flames()`, `waves()`, `palettes()`, `borders()`,
  `flashlights()`, `audioProcessors()`, `findScreen(name)`,
  `resolve(SceneSelection)`, `registerEntry(...)`, and validation methods.

### `AudioSystem`

- Purpose: Own audio device/file runtime, threading, lifecycle, and the current
  frame source.
- Globals replaced: `AudioRuntime.cc` file-scope pointers, threads, atomics,
  chunk buffers, completion flags, visual clock state, mixer setup state where
  runtime-owned, and audio dump state when it belongs to output.
- Used by: `Application` startup/run/teardown, `AudioFramePipeline`, display
  frame context creation, and tests with fake input.
- Provided to customers: `ApplicationContext` owns `AudioSystem`; dependents
  receive `AudioSystem&` for lifecycle or `AudioFrameProvider&` for frame reads.
- Lifecycle timing: Configured after `AppOptions` and `PathConfig`; started
  before the first display frame; advanced or polled once per application frame;
  stopped before display teardown and before final ini save.
- References contained: `const AudioOptions&`, `PathConfig&` or resolved paths,
  `Clock&`, `Logger&`, optional `ShutdownRequester&`, device/file backends, and
  owned `AudioFrameProvider` implementation.
- API surface: `configure(AudioOptions, PathConfig)`, `start()`, `poll()`,
  `stop()`, `finished() const`, `frameProvider()`, `metricsSource()` if needed,
  and `setDumpWriter(AudioDumpWriter)`.

### `AudioFrameProvider`

- Purpose: Provide explicit access to raw and processed audio frames without
  global facade functions.
- Globals replaced: `audioFrameRawData()`, `audioFrameProcessedWaveData()`,
  silent fallback buffers in `AudioFrame.cc`, and current-frame file-scope
  storage in `AudioRuntime.cc`.
- Used by: `AudioProcessor`, `AudioAnalyzer`, `VideoFrameContext`,
  `ScreenRenderContext`, `BorderFilter`, `display.cc` renderers, and tests.
- Provided to customers: Exposed by `AudioSystem::frameProvider()`; passed as
  `AudioFrameProvider&` or `const AudioFrameProvider&` to frame builders and
  processors.
- Lifecycle timing: Available after `AudioSystem::configure()`; returns silent
  frames before audio starts, latest frames during each application frame, and a
  final stable/silent frame during shutdown.
- References contained: May reference `AudioRuntime` current frame storage or
  own a small silent-frame fallback; should not reference visual systems.
- API surface: `currentFrame() const`, `rawSamples() const`,
  `processedWave() const`, `sampleRate() const`, `channels() const`,
  `isSilent() const`, and `silentFrame()`.

### `AudioAnalysisState`

- Purpose: Own derived audio analysis data used by visuals and automatic scene
  changes.
- Globals replaced: `audioAnalyzer`, `audioMetrics`, `acousticContext`, and
  analysis reads in `display.cc` and `AutoChanger`.
  The global `autoChanger` pointer is removed by moving automatic scene-change
  policy into the Scene module as `SceneChangeScheduler`.
- Used by: `AudioFramePipeline`, `AutoChanger`,
  `VideoFrameContext`, `ScreenRenderContext`, `display.cc` 3-D/bent/zick
  renderers, and border/flashlight effects that depend on audio.
- Provided to customers: `ApplicationContext` owns `AudioAnalysisState`;
  analyzers receive mutable access, visual consumers receive const views or
  per-frame snapshots.
- Lifecycle timing: Created before audio starts; updated after each audio poll
  and before video composition; snapshotted into frame contexts before screen
  rendering; reset during audio restart and teardown.
- References contained: Owns `AudioMetrics`, `AcousticContext`, analysis
  history, and maybe a reference to `AudioFrameProvider` during update calls.
- API surface: `update(const AudioFrameProvider&)`, `metrics() const`,
  `acoustic() const`, `snapshot() const`, `reset()`, and
  no scene-policy methods. The current
  `minimumNoiseSatisfied(AutoChangeOptions)` idea belongs in Scene policy, not
  Audio analysis.

### `VideoDirector`

- Purpose: Coordinate scene state, frame composition policy, messages,
  transitions, and display presentation decisions.
- Globals replaced: `videoDirector()` singleton, reads of `cthughaDisplay`,
  `CthughaBuffer::buffer`, `maxFramesPerSecond`, `changeMsgTime`, and
  `paletteSmoothingChance`.
- Used by: `Application::run()`, `SceneCommands`, key actions,
  auto-change, message/overlay code, and tests for scene
  transitions.
- Provided to customers: Owned by `ApplicationContext` after display, buffer,
  options, catalogs, and analysis services are available; passed explicitly as
  `VideoDirector&` or narrower `ScenePresenter&`.
- Lifecycle timing: Constructed after `CthughaBuffer` and display runtime
  setup; initialized before first frame; called once per frame after audio
  analysis and input handling; shut down before display backend teardown.
- References contained: `CthughaBuffer&`, display/presentation sink,
  `const DisplayOptions&`, `const MessageOptions&`, `RandomSource&`, `Clock&`,
  optional `AudioAnalysisState&`, and renderer/composer owned state.
- API surface: `initialize(SceneSettings)`, `composeFrame(VideoFrameContext)`,
  `present()`, `setScene(SceneSettings)`, `postMessage(Message)`,
  `setPaletteSmoothing(...)`, `frameBudget() const`, and `shutdown()`.

### `CthughaBuffer`

- Purpose: Own the indexed pixel buffers and expose buffer access explicitly to
  renderers, loaders, and display handoff code.
- Globals replaced: `CthughaBuffer::buffer` and `CthughaBuffer::current`.
- Used by: `Application`, `SceneCommands`, `VideoDirector`,
  `PresentationComposer`, image/translation generation, display fallback paths,
  and tests that build deterministic frame buffers.
- Provided to customers: `ApplicationContext` owns the instance; consumers
  receive `CthughaBuffer&`, `IndexedFrame&`, or immutable frame views.
- Lifecycle timing: Constructed after display/buffer size options are resolved;
  resized during display-mode changes; written during each render pass; read
  during presentation; destroyed after display presentation stops.
- References contained: Owns frame storage, dimensions, active buffer index,
  and possibly palette/translation working buffers; no references to services.
- API surface: `resize(Size)`, `currentFrame()`, `nextFrame()`,
  `swapFrames()`, `clear(index)`, `dimensions() const`, and `indexedView()`.

### `InterfaceManager`

- Purpose: Own UI interfaces, current selection/edit state, and option editing
  flow.
- Globals replaced: `Interface::head`, `Interface::current`, `currentOption`,
  `currentEffectControl`, and `currentOptionInterfaceElement`.
- Used by: key/action dispatch, option panels, X11 controls, text overlays,
  ini save/edit actions, and tests that exercise option navigation.
- Provided to customers: during the globals-removal pass, `Application` owns
  the concrete `InterfaceRuntime` and any owned panel instances directly;
  actions receive `InterfaceRuntime&` or narrower option-editing context through
  `CommandContext`.
- Lifecycle timing: Created after `AppOptions`, `EffectRegistry`, and
  `VisualCatalogs`; populated during startup UI registration; updated after
  input events and before frame composition; flushed/saved during teardown if
  required.
- References contained: References to editable option views, effect registry,
  visual catalogs, display text sink, and optional logger.
- API surface: `registerInterface(InterfaceDefinition)`, `current() const`,
  `selectNext()`, `selectPrevious()`, `beginEdit(OptionId)`,
  `adjustCurrent(delta)`, `renderOverlay(TextSink&)`, and `saveState(IniWriter&)`.

### `KeymapRegistry`

- Purpose: Own keymaps, action bindings, and dispatch context explicitly.
- Globals replaced: `Action::head`, `Keymap::first`, `Keymap::current`,
  `Keymap::keymapFile`, `keyAssoc`, `nKeyAssoc`, and static
  `Keymap::action(...)` dispatch.
- Used by: command-line/ini keymap loading, X11 input handling,
  key-action execution, UI tests, and help/credits displays.
- Provided to customers: during the globals-removal pass, `Application` owns
  the concrete registry directly; input systems receive `KeymapRegistry&` for
  translation/dispatch and actions receive an explicit `CommandContext`. A
  higher-level module facade can wrap this later after the registry no longer
  hides legacy state.
- Lifecycle timing: Defaults registered before keymap files are read; command
  line/ini can select or override keymaps during startup; dispatch occurs after
  each event pump and before audio/video composition for that frame.
- References contained: Registered action definitions, keymap definitions,
  selected keymap, `PathConfig` for loading, and maybe `Logger&`; no hidden
  references to scene/display/audio globals.
- API surface: `registerKeymap(KeymapDefinition)`, `load(Path)`,
  `select(name)`, `translate(InputEvent)`, `actionsFor(Key)`, and
  `bindings() const`. Command execution belongs to `CommandDispatcher`, not to
  the registry.

### `DisplayRuntimeOwnership`

- Purpose: Own the display facade, backend, device, overlay sink, and backend
  configuration state as explicit objects.
- Globals replaced: `cthughaDisplay`, `displayDevice`, `displayBackend`,
  `displayRuntime`, `xcth_display`, `xcth_app_con`,
  `DisplayDeviceX11::xcth_toplevel`, `disp_size`, `bypp`, `bytes_per_line`,
  `draw_mode`, `colormapped`, `bitmap_colors0..3`, `text_size`, `fontSize`,
  `errors`, and `ScopedOverlayDisplayDevice`.
- Used by: `Application`, `CthughaDisplay`, `VideoDirector`, X11 callbacks,
  overlay/message rendering, key/display commands, and tests with fake display
  sinks.
- Provided to customers: `ApplicationContext` owns `DisplayRuntimeOwnership`;
  consumers receive narrow references such as `DisplaySink&`,
  `OverlaySink&`, `DisplayBackend&`, or `DisplayGeometryProvider&`.
- Lifecycle timing: Configured after options and path/resource setup; opened
  before audio/video starts; pumped for input and presented once per frame;
  overlays are flushed during composition; closed after audio/video shutdown.
- References contained: `DisplayOptions&`, `PathConfig&`, `Logger&`,
  platform/X11 state, backend/device/display instances, geometry, native pixel
  lookup tables, and overlay/error text state.
- API surface: `open(DisplayOptions)`, `pumpEvents(InputQueue&)`,
  `display()`, `backend()`, `device()`, `geometry() const`,
  `present(const IndexedFrame&)`, `overlaySink()`, `resize(Size)`, and
  `close()`.

### `ConfigPersistence`

- Purpose: Own ini writing and persistence coordination separate from startup
  config acquisition.
- Globals replaced: `IniFiles.cc` writer state, the persisted startup `Config`
  snapshot, and direct runtime save commands that call a global no-arg writer.
- Used by: application save-on-exit, UI save commands, stop-and-continue state,
  and module serializers for runtime-owned state.
- Provided to customers: `ApplicationContext` owns `ConfigPersistence`;
  services receive `ConfigWriter&` or a narrow persistence command target.
- Lifecycle timing: Created after startup config is built; serves explicit save
  commands during runtime; writes final state during teardown when enabled.
- References contained: `PathConfig&`, `Logger&`, current output file state, and
  serialization adapters supplied by module owners.
- API surface: `saveStartupSnapshot(Config)`, `saveContinuation(SceneSnapshot)`,
  `writer()`, and `diagnostics()`.

### `PlatformLifecycle`

- Purpose: Own platform signals and lifecycle integration without leaking
  process statics into application logic.
- Globals replaced: public dependency on `PlatformLifecycle.cc` signal statics
  and direct platform shutdown checks outside the lifecycle service.
- Used by: `Application` startup/run/teardown, signal handlers, X11 lifecycle,
  audio interruption handling, and tests that simulate termination requests.
- Provided to customers: `ApplicationContext` owns `PlatformLifecycle`;
  `Application` calls it directly, while signal handlers write only to its
  internal platform-safe flag.
- Lifecycle timing: Installed before subsystems start; polled before and after
  event/audio/frame work; uninstalled after display/audio teardown; reset
  between tests.
- References contained: Internal signal-safe flags, previous handlers, optional
  `ShutdownRequester&`, and `Logger&` for non-signal diagnostics.
- API surface: `install(ShutdownRequester&)`, `poll()`,
  `shutdownRequested() const`, `restore()`, `resetForTest()`, and
  `lastSignal() const`.

## Outside-In Refactor Plan

Every module/component migration follows the same outside-in loop. The API is
agreed and tested first, the dependent behavior is pinned next, and only then
does the implementation move global state behind the module boundary.

1. Identify the service API, including the narrow read/write interfaces its
   customers should receive.
2. Write service tests that fail because the service contract does not exist
   yet. These are red 1 tests.
3. Write dependent tests that fail because current dependents still use globals
   or lack injection points. These are red 2 tests.
4. Implement the service and its fakes/adapters until the service tests pass.
   This is green 1.
5. Update the dependents to receive the service explicitly until the dependent
   tests pass. This is green 2.
6. Recheck source for remaining implicit dependencies or globals that belong to
   that service. Add a boundary test for every remaining compatibility alias.
7. If the recheck finds more hidden dependencies, revise the API and repeat
   from step 1 for that service.

### Cross-Cutting Guard Rails

- Add a source-boundary test harness before the first service migration. It
  should be able to assert that a token is allowed only in named owner files and
  compatibility adapters.
- Keep compatibility aliases short-lived. Each alias must have an owner file, a
  removal condition, and a source-boundary test that blocks new consumers.
- Preserve current smoke coverage after each module/component migration:
  `cmake --build build`, `ctest --test-dir build --output-on-failure`,
  `tests/headers/check-headers.sh`, and `git diff --check`.
- Record manual smoke commands for X11, MIT-SHM, audio playback, screen modes,
  overlays, keymaps, and ini persistence before deleting the old globals.
- Delete or prove the stale declarations listed above as soon as the first
  boundary test harness exists.

### Module Migration Order

This order is outside-in at the module boundary. Within each module, apply the
seven-step loop to every clean service or split-out component before moving on.
Do not implement split-required services as single classes.

1. Application Lifecycle - complete for lifecycle ownership, Process Services diagnostics tail remaining
   - API first: module-root construction, shutdown requests, lifecycle polling,
     clock/random/logging interfaces, and run-loop sequencing.
   - Red 1 tests: lifecycle services can be faked independently and do not need
     any scene/audio/display globals.
   - Red 2 tests: `Application::run()`, key quit, X11 quit, audio completion,
     frame timing, random seeding, and diagnostics use injected lifecycle
     collaborators.
   - Green targets: `ShutdownController`, `PlatformLifecycle`, narrow `Clock`,
     narrow `RandomSource`, policy-free `Logger`, and deletion or ownership of
     unused shell-execution helpers.
   - Done since 73fd509: shutdown requests are Application-owned through
     `RuntimeShutdown`/`RuntimeCloseState`; direct `cthugha_close` mutation is
     gone from normal runtime paths, and the stale declaration/definition were
     removed. `Application` now owns its `FramePacer`, and owns the clock and
     random-source adapters used by `AutoChanger` and audio-processing mode
     selection, plus the quiet-observer adapter used by `AutoChanger` and
     synthetic random PCM input. Audio ingest visual pacing
     and audio frame pipeline trace timing receive an Application-owned
     `SecondsClock`. Application run-loop/frame tracing and frame-pacing
     samples use the same owned `SecondsClock`. `FrameClock` now consumes the
     shared `SecondsClock` interface instead of its private time-source adapter.
     `CthughaDisplay` owns the visual frame clock/timestamp/delta values instead
     of exporting `now`/`deltaT`.
     Interface error expiry and credits animation use the Application-owned
     `MillisecondClock` through `InterfaceRuntime`. X11 display/frame trace
     timing also receives the Application-owned `SecondsClock`.
     `CStdRandomSource` owns deterministic generator state, and direct
     Application seeding of legacy C random state was removed after migrated
     runtime paths stopped using it. Generic EffectControl randomization,
     random palette generation, and wave rendering now consume that source
     through `SceneCommands`/`WaveRuntime`. Application now also owns
     `ConsoleLogSink`; `AudioIngest`, `RuntimeFactory`, and `PcmSourceFactory`
     receive/use an explicit `LogSink` for Audio startup diagnostics, and
     `AudioFramePipeline` receives it for per-frame audio diagnostics.
     `PcmSource`/`AudioInput`/`DspPcmSource` receive it for acquisition
     diagnostics. `AudioOutput` receives it for shared output service
     diagnostics, and Pulse/OSS-DSP output backends, `AudioPassthrough`, and
     `AudioOutputDump` now emit through that sink too. `Application` itself
     now emits startup/run-loop lifecycle diagnostics through its owned
     `ConsoleLogSink`, and `PlatformLifecycle` receives `LogSink&` for
     suspend/resume diagnostics instead of using `CTH_*` macros.
   - Remaining: continue replacing `CTH_*` macro call sites with explicit
     logging sinks and then delete `LegacyLoggingAdapter.cc`.
   - Recheck target: no reads/writes of `cthugha_close`, `gettime()`,
     `getTime()`, `rand()`, `srand()`, `Random()`, lifecycle statics, or
     logging verbosity outside owner/adapters.

2. Configuration - complete for startup acquisition/provisioning and current persistence boundary
   - API first: `ConfigurationBuilder`, `ConfigSource` strategies,
     `ConfigPatch`, `ConfigSchema`, `DeferredLogBuffer`, immutable `Config`
     slices, `RuntimeOptionSchema`, `PathResolver`, `ConfigPersistence`, and
     serializers supplied by modules.
   - Red 1 tests: command-line, ini, environment, path resolution, validation,
     deferred diagnostics, source precedence, and save behavior operate on
     explicit models.
   - Red 2 tests: audio setup, display setup, scene defaults, UI editing, and
     save-on-exit consume only their configuration slices rather than globals or
     whole-application config.
   - Green targets: split runtime `Option` state from startup config, narrow
     `PathConfig`, keep source strategies testable, keep config slices
     read-only, and move persistence behind explicit module serializers.
   - Done since 73fd509: startup config acquisition/provisioning is typed,
     tested, and slice-based; legacy parser globals and no-arg ini write paths
     were removed.
   - Remaining: finish module-owned serializers for Scene/Display and reduce
     Configuration knowledge of runtime-owned domains. Runtime persistence
     already reads explicit current-state contributors rather than startup
     globals.
   - Recheck target: no scalar/string startup option globals, path globals, or
     parser-owned globals outside Configuration and persistence adapters.

3. Runtime Reconfiguration - complete for current command-routing boundary
   - Done since 73fd509: runtime commands, mediator, subsystem control ports,
     runtime config registry, runtime persistence, shutdown, audio-processing,
     auto-change, effect-control, display, and mixer routing are in place.
     Current working-tree follow-up removed raw `Option&`/`EffectControl&`
     command payloads and added `RuntimeCommandTargetRouter` for synchronous
     target-backed UI/panel dispatch.
   - Remaining: `Option`/`EffectControl` still own too much domain state for
     Scene and Display; those modules need owned live state before this module
     can become mostly metadata plus typed dispatch.
   - Recheck target: `Option` no longer owns parsing, validation, live state,
     UI text, side effects, and persistence as one object; live changes are not
     applied by writing globals or mutating immutable startup `Config`.

4. Commands And Input - interface runtime partial, keymap/input registries remaining
   - API first: `InputQueue`, `CommandRegistry`, `CommandDispatcher`,
     `CommandContext`, input-to-command keymaps, and option-editing state.
   - Red 1 tests: keymaps translate events into command intents without owning
     command targets; dispatcher executes against explicit command context.
   - Red 2 tests: key actions, option editing, help/credits, and X11 events no
     longer touch hidden current state or runtime globals.
   - Green targets: split `KeymapRegistry`, split `InterfaceManager`, explicit
     command registration, and UI edit state independent of display overlays.
   - Done since 73fd509: `InterfaceRuntime` owns interface selection and
     scoped option/effect command context.
   - Done in the current working tree: raw X11 key globals are replaced by
     `InputQueue`, and keymap storage/name dispatch is owned by
     Application-owned `KeymapRegistry`.
   - Remaining: split `Action::head`, raw `Action*` bindings, and global
     `ACTION(...)` objects into explicit command registry/dispatcher objects.
   - Recheck target: no `x11_key`, `key_esc`, `Action::head`,
     `Keymap::first`, `Keymap::current`, `Interface::head`,
     `Interface::current`, `currentOption`, `currentEffectControl`, or static
     keymap dispatch consumers.

5. Scene - partially complete, still a major remaining module
   - API first: `SceneController`, `SceneState`, `SceneSnapshot`,
     `SceneChangeScheduler`, `SceneSerializer`, `EffectRegistry`, and
     `VisualCatalogs`.
   - Red 1 tests: scene state, catalog resolution, effect changes, presets, and
     auto-change policy run against fixture catalogs/options/audio snapshots.
   - Red 2 tests: `SceneCommands`, key actions, UI lists, preset loading, and
     auto-change no longer read visual option globals or `videoDirector()`.
   - Green targets: `EffectRegistry`, `VisualCatalogs`, scene selection state,
     scene serializer, and `AutoChanger` as Scene-owned
     `SceneChangeScheduler`.
   - Done since 73fd509: startup scene config is slice-based; `Scene` and
     `SceneCommands` exist; auto-change settings are owned; `autoChanger` is no
     longer a process-wide pointer.
   - Remaining: own visual selections, effect registry/catalogs, presets,
     scene serialization, and move AutoChanger policy out of `Application`.
   - Recheck target: no visual selection globals, `EffectControl::first`,
     hidden preset traversal, `sceneCommandsForLegacyCallbacks()`, or global
     `autoChanger`.

6. Audio - complete for runtime ownership, with Scene-owned policy cleanup remaining
   - API first: `AudioAcquisitionRuntime`, `AudioPassthrough`, raw
     `AudioFrameProvider`, `AudioProcessingPipeline`, `AudioFrameProducts`,
     `AudioAnalysisSnapshot`, mixer/device adapters, and `AudioDumpWriter`.
   - Red 1 tests: audio runtime, frame provider, processor, analyzer, silent
     fallback, mixer/device config, and dump writer are independently fakeable.
   - Red 2 tests: `AudioFramePipeline`, screen contexts, and border effects
     consume explicit audio products.
   - Green targets: split `AudioSystem`, raw-only `AudioFrameProvider`,
     policy-free `AudioAnalysisState`, owned audio processor state, mixer state,
     and dump writer.
   - Done since 73fd509: `AudioRuntime` and audio frame facades were removed;
     acquisition, passthrough, processing selection, processor/FFT state,
     analysis metrics, output config/dump, Pulse diagnostics, and OSS mixer are
     owned/tested units. Current working-tree follow-up also removed the OSS
     mixer dependency on the legacy logging macro adapter, extracted
     `AudioFramePipeline`, removed `AutoChanger`'s direct `videoDirector()`
     dependency, moved `AutoChanger` ownership to `Application`, and deleted
     the intermediate bridge wrapper.
   - Remaining: move Application-owned auto-change policy into Scene. The
     clock/random/logging dependencies for the current audio runtime path are
     already explicit; broader legacy `CTH_*` cleanup belongs to the Process
     Services diagnostics tail.
   - Recheck target: no audio facade functions, `audioAnalyzer`,
     `audioMetrics`, `acousticContext`, `AudioRuntime.cc` file-scope runtime
     state, mixer globals, or audio dump globals outside owners.

7. Frame Mutation - partly advanced by display/audio context work, still open
   - API first: `FrameStore`, `FrameComposer`, `FrameMutationContext`,
     renderer state factories, render math tables, and explicit frame handoff.
   - Red 1 tests: renderers and composers mutate fixture frames from explicit
     scene/audio/display-dimension snapshots.
   - Red 2 tests: `display.cc`, border, translation/image generation, and
     screen render paths no longer read global buffers, audio facades, or scene
     options.
   - Green targets: `CthughaBuffer` as `FrameStore`, renderer-local state,
     `ScreenRenderContext` audio/display inputs, and split-out composer pieces
     from `VideoDirector`.
   - Recheck target: no `CthughaBuffer::buffer`, `CthughaBuffer::current`,
     `display.cc` renderer statics, direct audio globals, or mutable math table
     globals outside explicit renderer/math owners.

8. Display - still open
   - API first: `DisplaySystem`, `DisplayBackend`, `DisplayGeometry`,
     native-pixel transfer, `OverlayMessageQueue`, `OverlaySink`, and raw event
     output.
   - Red 1 tests: fake display systems can open, report geometry, collect raw
     events, accept frames, and render overlays.
   - Red 2 tests: `Application`, frame presenter, overlays, X11 callbacks, and
     display commands use explicit display collaborators.
   - Green targets: split `DisplayRuntimeOwnership`, remove overlay device
     swapping, move native pixel tables/geometry/text state into display-owned
     objects, and route raw events to Commands And Input.
   - Recheck target: no `cthughaDisplay`, `displayDevice`, `displayBackend`,
     `displayRuntime`, X11 globals, global display geometry, global overlay
     text/error state, or logger-to-overlay back references.

9. Final compatibility removal
   - API first: none; this is a deletion pass backed by source-boundary tests.
   - Red tests: forbidden-token tests cover every removed global and adapter.
   - Green targets: delete compatibility aliases, stale declarations, and
     hidden singleton helpers.
   - Recheck target: every remaining global is immutable, file-local, or owned
     behind a named module boundary with contract and dependent tests.

### Completion Criteria

- Production code has no direct references to mutable compatibility globals:
  `cthugha_close`, `cthughaDisplay`, `displayDevice`, `displayBackend`,
  `displayRuntime`, `CthughaBuffer::buffer`, `CthughaBuffer::current`,
  `videoDirector()`, `autoChanger`, `EffectControl::firstRegistered()`,
  `Interface::current`, `Keymap::action(...)`, global audio facade functions,
  global audio metrics, global display geometry, and mutable option globals.
- Remaining global data is immutable, file-local, or hidden inside a named
  module boundary that has contract tests and dependent injection tests.
- `ApplicationContext` construction order is documented in `Application` tests:
  process services, options/paths/ini, registries/catalogs, buffer/display,
  audio/analysis, video/UI/input, lifecycle teardown.

Each module/component migration should preserve the existing tests:

```sh
cmake --build build
ctest --test-dir build --output-on-failure
tests/headers/check-headers.sh
git diff --check
```
