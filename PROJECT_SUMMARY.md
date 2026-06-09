# CthughaNix Project Summary

This repository is the current C/C++ CthughaNix visualizer. It keeps the
classic indexed-color Cthugha visual language, but the active runtime is now
organized around explicit application-owned modules for audio ingest, scene
state, frame generation, display presentation, runtime commands, and
configuration persistence.

The main executable target is `xcthugha`, the X11 frontend. CMake is the active
build system. SDL3 appears as a build option and runtime display id, but no SDL3
frontend is wired into `src/CMakeLists.txt` today.

The current one-frame mental model is:

```text
DisplayRuntime processes platform events
CommandsInputRuntime runs current interface/key actions
CthughaDisplay publishes the frame timestamp
AudioIngest advances decoded history and builds AudioFrame
DefaultAudioFramePipeline processes/analyzes audio
SceneChangeScheduler may mutate Scene through SceneCommandTarget
FrameGeneratorRuntime binds Scene settings into FrameFilterchain
FrameFilterchain publishes an IndexedFrame
CthughaDisplay composes overlays and presents through DisplayRuntime
FramePacer sleeps to honor the configured max FPS
```

## Root Document Set

- `PROJECT_STRUCTURE.md`: current repository layout and source subsystem map.
- `PROJECT_RUNTIME_MAP.md`: startup, frame, audio, visual, display, command,
  and configuration flow.
- `PROJECT_MAIN_LOOP_EXPLAINED.md`: guided source walk through
  `Application::initialize()`, `run()`, and `runFrame()`.
- `PROJECT_SEAMS_AND_RISKS.md`: extension seams and risky remaining couplings.
- `PROJECT_BUILD_AND_PORTING.md`: CMake options, dependencies, verification
  commands, and porting notes.
- `PROJECT_VERIFICATION.md`: checks used to keep these notes tied to current
  source.

## Current Architecture

`Application` is the lifecycle root. It owns process clocks, logging, random
source, input runtime, audio ingest, audio analysis, scene runtime, frame
generator, display system, runtime command mediator, persistence, platform
suspend/resume hooks, and shutdown state.

Audio is acquired by `AudioIngest`. It creates a `PcmSource` through
`RuntimeFactory`/`PcmSourceFactory`, writes decoded PCM into
`DecodedAudioHistory`, optionally passes the same PCM to an `AudioPassthrough`,
and builds the current visual `AudioFrame` through `AudioFrameBuilder`.

The supported PCM source families are live line-in, random noise, WAV, MP3 via
vendored minimp3, raw headerless PCM, and miniaudio capture when compiled.
Playback output can use PulseAudio-compatible output, OSS DSP output,
miniaudio playback, or `AudioNullOutput`.

Scene state lives in `SceneRuntime`. It owns the `Scene`, visual selection
catalogs, scene commands, the effect-selection registry, and scene
serialization. Runtime changes reach it through typed `RuntimeCommand` values
handled by `RuntimeChangeMediator`.

Visual rendering lives in `FrameGeneratorRuntime`. It owns `FrameGeometry`,
`FrameStore`, `FrameRenderTarget`, palette-transition policy, scene binding,
and the `FrameGeneratorPipeline`. The default filterchain order is image,
border, flame, translate, wave, text, frame commit, palette, flashlight, and
indexed-frame publication.

Display presentation lives in `DisplaySystem`. The currently wired factory is
X11, which creates `DisplayDeviceX11`, `DisplayBackendX11`, `DisplayRuntime`,
and `CthughaDisplayX11`. `CthughaDisplay` receives the generated
`IndexedFrame`, composes the selected presentation screen and overlays, applies
viewport policy, and asks the display runtime to present.

## Important Current Boundaries

- `RuntimeFactory` chooses audio input/output strategy from startup settings
  and compile-time availability.
- `AudioIngest` owns decoded history, passthrough, and visual audio frame
  timing.
- `DefaultAudioFramePipeline` owns the per-frame processing/analyzer step.
- `SceneRuntime` owns scene choices, scene commands, and scene persistence.
- `SceneChangeScheduler` owns automatic scene-change policy.
- `FrameGeneratorRuntime` owns indexed visual storage and filterchain state.
- `FrameGeneratorSceneBinding` translates `SceneSettings` and scene cues into
  concrete filter state.
- `DisplaySystem` owns display driver selection and display component lifetime.
- `RuntimeChangeMediator` is the live command hub for UI, keymap, panel,
  persistence, audio/display controls, scene commands, and shutdown.

## Portability State

The project is portable in a transitional sense. The active CMake build has
clear audio, scene, frame-generator, command, and display seams, and miniaudio
provides a cross-platform audio path. The only compiled graphical frontend in
the main target is still X11/Xt/Xaw, and several display-era globals remain in
the X11/presentation layer.

Treat `xcthugha` as the behavioral reference while porting. Add new frontends
by extending `DisplayDriverFactory`/`DisplaySystem` and preserving the
`IndexedFrame` plus palette presentation contract.
