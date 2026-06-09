# Verification Notes

This file records how to verify that the root project notes describe the
current source tree rather than an older architecture.

## Documentation Inventory

The root notes currently expected are:

```text
PROJECT_SUMMARY.md
PROJECT_STRUCTURE.md
PROJECT_RUNTIME_MAP.md
PROJECT_MAIN_LOOP_EXPLAINED.md
PROJECT_SEAMS_AND_RISKS.md
PROJECT_BUILD_AND_PORTING.md
PROJECT_VERIFICATION.md
```

Inventory command:

```sh
find . -maxdepth 1 -name 'PROJECT_*.md' -print | sort
```

There may be compatibility or older notes below subdirectories; they are not
part of this root-doc inventory.

## Source Evidence Used

The most important source-of-truth files for these notes are:

```text
CMakeLists.txt
src/CMakeLists.txt
src/Application.cc
src/Application.h
src/AudioIngest.cc
src/AudioFramePipeline.cc
src/RuntimeFactory.cc
src/PcmSourceFactory.cc
src/AudioOutput.cc
src/AudioMiniAudioOutput.cc
src/MiniAudioCapturePcmSource.cc
src/SceneRuntime.cc
src/SceneChangeScheduler.cc
src/FrameGeneratorRuntime.cc
src/FrameGeneratorSceneBinding.cc
src/FrameGeneratorPipeline.cc
src/FrameFilterchain.cc
src/FrameFilters.cc
src/FrameStore.h
src/FrameRenderTarget.h
src/DisplaySystem.cc
src/DisplayDeviceSDL3.cc
src/Sdl3Presentation.cc
src/DisplayDeviceX11.cc
src/CthughaDisplay.cc
src/CthughaDisplayX11.cc
src/RuntimeChangeMediator.cc
tests/CMakeLists.txt
```

The docs should use current class names from those files. In particular, the
active architecture is centered on `AudioIngest`, `SceneRuntime`,
`SceneChangeScheduler`, `FrameGeneratorRuntime`, `FrameFilterchain`, and
`DisplaySystem`.

## Staleness Checks

Search root project docs for removed or superseded subsystem names before
calling the refresh complete:

```sh
stale='Audio''Runtime|Audio''VisualBridge|Video''Filterchain|Video''Director|Video''Frame|Cthugha''Buffer|Auto''Changer|Audio''Buffer'
rg -n "$stale" PROJECT_*.md
```

The shell concatenation keeps the removed names out of the docs as live prose.
If the search reports hits, inspect whether each hit is intentionally
historical. These notes should normally avoid historical names altogether and
describe only the current system.

Also check that the current names appear where expected:

```sh
rg -n 'AudioIngest|SceneRuntime|SceneChangeScheduler|FrameGeneratorRuntime|FrameFilterchain|DisplaySystem' PROJECT_*.md
```

Check that frontend docs describe the current dual-frontend state rather than
the older single-frontend state:

```sh
stale_frontend='only.*X''11|SDL''3.*placeholder|not wir''ed|xcthugha.*ref''erence'
rg -n "$stale_frontend" PROJECT_*.md
rg -n 'SDL3|cthugha|xcthugha|DisplayDeviceSDL3|DisplayDeviceX11' PROJECT_*.md
```

## Build And Test Checks

A documentation-only change should not require a full rebuild, but build/test
commands are still useful when documentation claims a target or test exists.

Current focused checks for this worktree:

```sh
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCTH_BUILD_X11=OFF \
  -DCTH_BUILD_SDL3=ON \
  -DCTH_ENABLE_MINIAUDIO=ON \
  -DCTH_ENABLE_PULSE=OFF \
  -DCTH_BUILD_TESTS=ON \
  -DCTH_BUILD_BENCHMARKS=OFF
cmake --build build --target cthugha scene_script_test frame_filterchain_diagnostics_test --parallel
ctest --test-dir build -R '^(scene_script_test|frame_filterchain_diagnostics_test)$' --output-on-failure
```

General project checks:

```sh
cmake -S . -B build -DCTH_BUILD_X11=OFF -DCTH_BUILD_SDL3=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

Real-device audio smoke tests are opt-in:

```sh
cmake -S . -B build-miniaudio-device-tests -G Ninja \
  -DCTH_BUILD_X11=OFF \
  -DCTH_BUILD_SDL3=OFF \
  -DCTH_ENABLE_MINIAUDIO=ON \
  -DCTH_RUN_AUDIO_DEVICE_TESTS=ON \
  -DCTH_BUILD_TESTS=ON
cmake --build build-miniaudio-device-tests
ctest --test-dir build-miniaudio-device-tests \
  -R 'miniaudio_.*_smoke' --output-on-failure
```

## Documentation Completion Criteria

The root docs are current when:

- every root `PROJECT_*.md` file has been reviewed or replaced;
- source names and build options match the current tree;
- frame flow matches `Application::run()` and `Application::runFrame()`;
- audio flow matches `AudioIngest`, `RuntimeFactory`, and `PcmSourceFactory`;
- visual flow matches `FrameGeneratorRuntime`, `FrameGeneratorSceneBinding`,
  and `FrameFilterchain`;
- display flow matches `DisplaySystem` and the SDL3/X11 factories;
- runtime command flow matches `RuntimeChangeMediator`;
- stale subsystem names do not remain as active architecture;
- verification searches and relevant focused build/test commands have been run
  or explicitly deferred.
