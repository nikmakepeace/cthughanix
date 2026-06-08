
## SDL3 macOS Buildout And Test Plan

This section describes how to build the SDL3 Display driver and prove it works
on macOS without weakening the explicit-dependency goal. It is intentionally
more operational than the module plan above. The expected outcome is a display
driver that can coexist with X11 in the same source tree, can be compiled on
macOS without X11/XQuartz, can be selected at runtime when compiled, and can be
tested with fast headless unit tests plus a small number of explicit windowed
macOS smoke tests.

Reference assumptions to re-check at implementation time:

- SDL3 headers are included as `<SDL3/SDL.h>`.
- SDL3 CMake packages provide an imported target named `SDL3::SDL3`.
- The SDL3 video/render flow is based on `SDL_Init(SDL_INIT_VIDEO)`,
  `SDL_CreateWindow`, `SDL_CreateRenderer`, `SDL_CreateTexture`,
  `SDL_LockTexture`/`SDL_UnlockTexture`, `SDL_RenderTexture`,
  `SDL_RenderPresent`, and `SDL_PollEvent`.
- SDL3 event names use the `SDL_EVENT_*` form, including key, quit, window
  resize, window pixel-size, expose, and close-request events.
- macOS windowed tests must run from a GUI login session and on the main thread.
  They should not be part of the default noninteractive test run.

### macOS Prerequisites

Use Homebrew for the first macOS path because it supplies an SDL3 package with a
CMake config file and keeps the developer workflow short. The initial
verification matrix should cover both Apple Silicon and Intel macOS if possible,
but one architecture is enough for the first local proof.

Install tools:

```sh
xcode-select --install
brew update
brew install cmake ninja sdl3
```

Use a dedicated build directory so X11 and SDL3 experiments never overwrite the
known X11 build:

```sh
cmake -S . -B build-sdl3-macos -G Ninja \
  -DCTH_BUILD_X11=OFF \
  -DCTH_BUILD_SDL3=ON \
  -DCTH_BUILD_TESTS=ON \
  -DCTH_BUILD_BENCHMARKS=OFF \
  -DCMAKE_PREFIX_PATH="$(brew --prefix sdl3)"
cmake --build build-sdl3-macos
ctest --test-dir build-sdl3-macos --output-on-failure
```

If CMake cannot locate SDL3 through `brew --prefix sdl3`, retry with the broader
Homebrew prefix:

```sh
cmake -S . -B build-sdl3-macos -G Ninja \
  -DCTH_BUILD_X11=OFF \
  -DCTH_BUILD_SDL3=ON \
  -DCTH_BUILD_TESTS=ON \
  -DCMAKE_PREFIX_PATH="$(brew --prefix)"
```

Do not require an app bundle for the first driver. A command-line executable run
from Terminal is sufficient for development. App bundle, Info.plist, signing,
and notarization concerns belong to a later packaging pass after the driver
contract is stable.

### Build-System Shape

The build must support three useful display configurations:

1. `CTH_BUILD_X11=ON`, `CTH_BUILD_SDL3=OFF`: current Linux/X11 behavior.
2. `CTH_BUILD_X11=OFF`, `CTH_BUILD_SDL3=ON`: macOS SDL3 behavior with no X11
   requirement.
3. `CTH_BUILD_X11=ON`, `CTH_BUILD_SDL3=ON`: one executable with runtime driver
   selection, useful on systems where both frontends are installed.

Add these CMake controls:

```cmake
option(CTH_BUILD_SDL3 "Build the SDL3 display driver" OFF)
```

When `CTH_BUILD_SDL3` is enabled:

- call `find_package(SDL3 CONFIG REQUIRED)`;
- add SDL3 driver sources to the display executable;
- add a compile definition such as `CTH_SDL3`;
- link `SDL3::SDL3`;
- add the SDL3 include path only through the imported target, not manually;
- keep SDL3 source files out of X11-only builds.

The eventual target should be a display-neutral executable such as `cthugha`
rather than an X11-named target. During migration, keep `xcthugha` working until
the runtime registry exists. Once both drivers share the same display contract,
rename or add the neutral target and optionally keep `xcthugha` as a compatibility
alias only if the packaging path needs it.

The build should not make X11 mandatory on macOS. `CTH_BUILD_X11=OFF` must skip
all `find_package(X11 ...)`, X11 include directories, X11 libraries, and
`CTH_XWIN` compile definitions. This is the key portability gate.

### Configuration Shape

Add a startup display-driver selection that is independent from driver-specific
settings:

```c++
enum DisplayDriverId {
  DisplayDriverAuto,
  DisplayDriverX11,
  DisplayDriverSDL3
};

struct DisplayConfig {
  DisplayDriverId driver;
  ...
};
```

Parse:

- ini key: `display.driver`;
- command-line option: `--display-driver DRIVER`;
- allowed values: `auto`, `x11`, `sdl3`.

Selection rules:

- `auto` chooses from compiled and registered factories using deterministic
  priority. Preserve existing behavior by preferring X11 when X11 is compiled,
  otherwise choose SDL3.
- `x11` requires an X11 factory to be registered.
- `sdl3` requires an SDL3 factory to be registered.
- an unavailable explicit driver is a configuration error, not a silent fallback.
- the chosen driver id should be visible in debug/startup logs.

Keep driver-specific config typed:

- `X11Config` is only consumed by the X11 factory.
- Add `SDL3Config` only for settings SDL3 genuinely owns.
- Shared geometry, zoom, FPS overlay, presentation choice, and frame-size values
  stay in `DisplayConfig`.

Initial `SDL3Config` should be small:

```c++
struct SDL3Config {
  int highPixelDensityEnabled;
  int resizableWindowEnabled;
  std::string rendererName;
  std::string frameDumpDirectory;
  int frameDumpLimit;
  int frameDumpEvery;
};
```

Do not add SDL3 settings for X11-only concepts such as root-window drawing,
private colormaps, MIT-SHM, X11 fonts, override-redirect, or Athena panel
widgets.

### Driver Contracts

Introduce the driver seam before writing SDL3-specific code. The seam should be
testable without opening a real SDL window.

Core construction types:

```c++
class DisplayOpenRequest {
public:
  DisplayConfig display;
  X11Config* x11;       // null unless X11 is compiled and selected
  SDL3Config* sdl3;     // null unless SDL3 is compiled and selected
  PixelSize initialFrameSize;
  LogSink& log;
};

class DisplayDriver {
public:
  virtual ~DisplayDriver() { }
  virtual int open(const DisplayOpenRequest& request) = 0;
  virtual DisplayEventStats processEvents(InputEventSink& input,
                                          DisplayLifecycleEventSink& lifecycle) = 0;
  virtual PixelSize outputSize() const = 0;
  virtual void present(const DisplayPresentation& presentation) = 0;
  virtual void close() = 0;
};

class DisplayDriverFactory {
public:
  virtual ~DisplayDriverFactory() { }
  virtual DisplayDriverId id() const = 0;
  virtual const char* name() const = 0;
  virtual std::unique_ptr<DisplayDriver> create() = 0;
};
```

`DisplayLifecycleEventSink` is important. Window close and app quit events are
lifecycle events, not keyboard input. SDL3 should not translate
`SDL_EVENT_QUIT` into a fake key and should not dispatch `RuntimeCommand`
objects from inside the driver. It should publish a close request through a
narrow lifecycle sink or through a typed field in `DisplayEventStats`, and
`Application` should route that to `RuntimeShutdown`.

The driver contract must be one-way:

- Display receives raw input and lifecycle events from the platform.
- Display receives completed indexed frames from Frame Generator.
- Display receives overlay commands from Interface.
- Display presents to the platform.
- Display never pulls scene state, frame storage, runtime commands, audio state,
  or interface state through globals.

### SDL3 Driver Object Model

Implement SDL3 through small owned objects rather than one large file-scope
blob. The first pass can keep them in one `.cc` file if needed, but the
ownership model should be clear.

Recommended objects:

- `Sdl3VideoContext`: owns SDL video initialization and final shutdown.
- `Sdl3Window`: owns `SDL_Window*`, window flags, logical window size, and
  high-DPI/pixel-size tracking.
- `Sdl3Renderer`: owns `SDL_Renderer*`, render output size queries, clear,
  texture draw, overlay draw, and present.
- `Sdl3StreamingTexture`: owns `SDL_Texture*`, texture size, pixel format, pitch
  negotiation, lock/unlock, and resize-on-frame-size-change behavior.
- `Sdl3PaletteExpander`: owns the 256-entry palette expansion table and converts
  indexed pixels to the texture pixel format.
- `Sdl3OverlayRenderer`: uses the existing `BitmapFont`/`dosVga9x14Font()` to
  draw overlay commands into the SDL renderer or into the streaming texture.
- `Sdl3EventPump`: maps SDL events into `InputEventSink`,
  `DisplayLifecycleEventSink`, and `DisplayEventStats`.
- `Sdl3FrameDumper`: optional diagnostic that writes the expanded frame or final
  pre-overlay frame to a portable image format for tests.

No SDL3 file should define mutable process-wide state. Every pointer returned by
SDL is owned by exactly one object and destroyed in that object's destructor or
`close()` method. A failed open should release already-created SDL resources
before returning failure.

### SDL3 Open And Close Lifecycle

Open order:

1. `DisplaySystem::open(...)` selects `DisplayDriverSDL3Factory`.
2. `DisplayDriverSDL3` constructs empty owned members.
3. `Sdl3VideoContext::open()` initializes SDL video.
4. `Sdl3Window::open()` creates the window from common `DisplayConfig`.
5. `Sdl3Renderer::open()` creates the renderer from the window.
6. `Sdl3Renderer` records the initial render output size.
7. `Sdl3OverlayRenderer` receives `DisplayTextMetrics` and the shared bitmap
   font.
8. The driver reports open success only after window, renderer, and initial
   output-size query all succeed.

Close order:

1. Stop using the current presentation frame.
2. Destroy the streaming texture.
3. Destroy renderer-owned overlay resources.
4. Destroy the renderer.
5. Destroy the window.
6. Quit the SDL video subsystem if this driver initialized it.
7. Clear all owned pointers and make `close()` idempotent.

Main-thread rule:

- SDL initialization, event polling, window operations, renderer operations, and
  texture updates should all happen on the main application thread.
- Do not add a display thread for SDL3.
- Do not write tests that open SDL windows on worker threads.

### SDL3 Window And Output Geometry

Use common display geometry for window creation:

- `DisplayConfig.displayWidth/displayHeight` when custom display size is set;
- otherwise the selected display mode size;
- initial frame size from Frame Generator only as a minimum texture/frame
  reference, not as a mutable global.

Window flags:

- use a resizable window by default unless configuration says otherwise;
- use high-pixel-density mode when enabled;
- use fullscreen only when the common display config requests fullscreen through
  a shared setting added for both drivers;
- do not implement X11 root-window behavior in SDL3.

Separate these sizes:

- logical window size: what the user asked for and what window events report;
- render output size: actual drawable pixels, especially important on Retina;
- source frame size: the indexed frame from Frame Generator;
- composed indexed display frame size: output of `PresentationComposer`;
- viewport destination: destination rectangle for rendering into output pixels.

`DisplayDriverSDL3::outputSize()` should return render output pixels, not
logical window points. On macOS Retina displays this distinction prevents blurry
or incorrectly scaled output.

On resize and pixel-size events:

- increment `DisplayEventStats.resizeEvents`;
- update cached render output size;
- mark the next presentation as needing a border/background clear;
- do not resize Frame Generator storage;
- do not mutate DisplayConfig.

### SDL3 Texture And Pixel Transfer

The first SDL3 renderer should use a streaming texture because it maps cleanly
onto Cthugha's "indexed frame plus palette" model.

Texture policy:

- create one streaming texture whose size matches the composed
  `IndexedDisplayFrame`;
- recreate it when composed frame width or height changes;
- use a predictable 32-bit format such as RGBA8888 or ARGB8888;
- set nearest-neighbor scaling so indexed artwork stays crisp;
- keep all palette expansion state in `Sdl3PaletteExpander` or the common
  `NativePixelTransfer`, not in globals.

Present path:

1. Receive `DisplayPresentation` from `DisplaySystem`.
2. Validate the composed indexed frame.
3. Ensure streaming texture size matches the frame.
4. Update the native palette table when `FramePalette` is dirty or when text
   overlay palette rules require it.
5. Lock the streaming texture.
6. Expand indexed source pixels into the locked texture using the locked pitch.
7. Unlock the texture.
8. Clear the renderer to black.
9. Draw the texture into the viewport destination rectangle.
10. Draw overlay commands.
11. Present the renderer.

Do not use X11's `needsFullCopy` semantics as an SDL3 control path. SDL3 can
clear the renderer every frame and render the current texture into the current
viewport. Keep the flags in `DisplayPresentation` for compatibility and tests,
but treat them as hints rather than a requirement for partial native copies.

### SDL3 Palette Behavior

The current X11 display has special palette behavior when text is on screen.
SDL3 should preserve visible behavior without preserving X11's colormap
mechanics.

Rules:

- the indexed visual frame remains unchanged;
- palette expansion applies text-darkening or text-reserved colors only in the
  presentation step;
- text overlay colors use Display-owned constants or `DisplayTextPalette`;
- `FramePalette::paletteDirty()` is acknowledged only by the display backend
  that consumed it;
- SDL3 should not write `bitmap_colors*` or rely on `draw_mode`.

For true-color SDL3 output, implement text dimming by choosing overlay/text
colors and optional frame darkening in the expanded RGBA pixels. Do not simulate
X11 PseudoColor cell allocation.

### SDL3 Overlay Rendering

The first SDL3 overlay renderer should not use SDL_ttf. Use the existing
bitmap font to avoid adding another dependency.

Overlay pipeline:

1. Interface produces `OverlayCommands` through `InterfaceOverlaySource`.
2. Display computes `OverlayLayout` from `DisplayTextMetrics` and render output
   size.
3. SDL3 draws the frame texture.
4. SDL3 draws each overlay text command using the bitmap font.
5. SDL3 presents the renderer.

Text coordinates:

- `OverlayTextCommand.y` keeps the existing line-based behavior;
- positive `y` counts down from the top;
- negative `y` counts up from the bottom;
- justification `l`, `c`, and `r` are applied using `OverlayLayout.columns`;
- clipping happens inside `Sdl3OverlayRenderer`.

Text colors:

- `TEXT_COLOR_NORMAL`, `TEXT_COLOR_ERROR`, and `TEXT_COLOR_HIGHLIGHT` should
  move to a display-owned enum or value object;
- SDL3 maps those values to RGBA colors;
- palette darkening should be a display presentation effect, not an Interface
  side effect.

### SDL3 Event Mapping

Create pure mapping helpers before touching the real event loop. The helpers
make event behavior testable without opening a window.

Event categories:

- quit/close request: publish to `DisplayLifecycleEventSink`;
- key up: publish raw key text plus shift state to `InputEventSink`;
- window resize/pixel-size changed: update output size and stats;
- expose: mark a redraw/full-clear request and stats;
- all other events: ignore unless a future SDL3 feature needs them.

Keyboard mapping must match the current `KeyTranslator` expectations:

- letters and digits should produce their one-character text;
- shifted digits should preserve the existing shifted-number behavior through
  the `shifted` flag;
- function keys should produce `F1` through `F24`;
- arrow keys should produce `Up`, `Down`, `Left`, `Right`;
- page up/down should produce `Prior` and `Next` to match the current X11 names;
- return should produce `Return`;
- backspace should produce `BackSpace`;
- delete should produce `Delete`;
- home/end should produce `Home` and `End`;
- keypad digits should produce `KP_0` through `KP_9` or direct digits, whichever
  the existing translator expects after tests confirm behavior;
- escape should produce the same raw text currently accepted by `InputQueue`.

Do not let SDL3-specific key names leak throughout the application. Keep them in
`Sdl3KeyMapper`, and test that mapper directly.

### macOS Test Layers

Use four layers of tests. The first three run in normal `ctest`; the fourth is
explicitly opt-in because it opens real windows.

1. **Source-boundary tests.**
   These check that SDL3 is behind the driver seam and that Display remains
   explicit:
   - `Application.cc` does not include SDL headers;
   - only SDL3 driver files include `<SDL3/...>`;
   - SDL3 files do not include X11 headers;
   - X11 files do not include SDL3 headers;
   - Interface files do not include display backend headers;
   - Display driver files do not include Frame Generator implementation headers.

2. **Pure unit tests.**
   These do not call SDL:
   - driver registry selection for `auto`, explicit `x11`, explicit `sdl3`,
     unavailable explicit driver, and no available drivers;
   - `DisplayConfig` parsing for `display.driver`;
   - `SDL3Config` parsing for any SDL3-specific keys;
   - SDL3 key-name mapper;
   - viewport selection for Retina render-output sizes;
   - RGBA palette expansion for representative palette entries;
   - texture resize decision logic;
   - overlay layout and clipping;
   - lifecycle close-request routing.

3. **SDL API adapter tests with fakes.**
   Wrap SDL calls behind a tiny `Sdl3Api` interface only where it helps
   lifecycle testing. Use a fake implementation to test:
   - open creates video, window, renderer, and texture in order;
   - open failure at each stage releases earlier resources;
   - `close()` is idempotent;
   - texture recreation happens only on geometry changes;
   - present locks, writes, unlocks, clears, renders, overlays, and presents in
     order;
   - event pumping updates stats and sinks correctly.

4. **Windowed macOS smoke tests.**
   These run only when explicitly enabled, for example with
   `CTH_RUN_WINDOW_TESTS=ON` or an environment variable. They require a logged-in
   macOS GUI session. They should not run in default CI unless that CI has a
   real window server.

Windowed smoke targets:

- `sdl3_window_open_smoke`: opens a small SDL3 window, clears it, presents once,
  and exits.
- `sdl3_texture_present_smoke`: presents a deterministic indexed gradient
  through the SDL3 texture path for a fixed number of frames, then exits.
- `sdl3_resize_smoke`: opens a resizable window, programmatically requests or
  simulates resize where SDL allows it, and verifies output-size bookkeeping.
- `sdl3_event_smoke`: opens a window and exits when the close event is observed.

The smoke binaries should be tiny and isolated from audio and full scene setup
where possible. If the full application is used for a smoke run, pass options
that avoid macOS audio-device dependency:

```sh
./build-sdl3-macos/src/cthugha \
  --display-driver=sdl3 \
  --no-sound \
  --silent \
  --disp-mode=320x200 \
  --max-fps=30 \
  --show-fps \
  --no-save
```

If the executable is still named `xcthugha` during migration, use that name
temporarily but do not encode it into new tests as the permanent SDL3 target.

### macOS Manual Verification Matrix

After unit tests pass, run a short manual verification pass on macOS:

1. **Startup and shutdown.**
   Launch SDL3, verify a window appears, then close the window. Confirm the app
   exits through the lifecycle sink and logs a clean shutdown.

2. **Driver selection.**
   Run `--display-driver=sdl3` and confirm SDL3 is selected. Run
   `--display-driver=x11` in an SDL3-only build and confirm startup reports a
   configuration error instead of falling back silently. Run
   `--display-driver=auto` and confirm the selected driver matches the compiled
   registry priority.

3. **Retina output.**
   On a Retina display, log logical window size and render output size. Confirm
   viewport math uses render output pixels and that the image is sharp with
   nearest-neighbor scaling.

4. **Resize.**
   Resize the window smaller, larger, and across aspect ratios. Confirm the
   image remains centered, letterbox borders clear to black, and no frame
   storage resize is triggered by display resize.

5. **Keyboard.**
   Verify letters, shifted letters, digits, shifted digits, arrows, page
   up/down, home/end, return, backspace, delete, escape, function keys, and
   keypad digits. Confirm behavior matches X11 keymap behavior.

6. **Overlay.**
   Toggle interface panels and FPS overlay. Confirm normal, highlighted, and
   error text are legible, aligned, clipped, and removed cleanly on subsequent
   frames.

7. **Frame presentation.**
   Run several display modes and zoom values. Confirm source-sized,
   double-sized, and presentation effects display correctly.

8. **Fullscreen.**
   If fullscreen support is added in the first SDL3 pass, verify entering and
   exiting fullscreen. If not, explicitly document fullscreen as deferred and
   ensure the config parser rejects or ignores it with a clear diagnostic.

9. **Failure behavior.**
   Temporarily request an invalid renderer name or impossible texture size in a
   test build. Confirm startup failure releases SDL resources and reports the
   reason.

10. **No X11 dependency.**
    Configure with `CTH_BUILD_X11=OFF`, uninstall or hide XQuartz if present,
    and verify the SDL3 build still configures, builds, and runs.

### Debug And Diagnostics

Add debug logs that make SDL3 problems diagnosable without a debugger:

- selected display driver;
- SDL version/build information if available through SDL APIs;
- window logical size;
- render output size;
- texture format and size;
- palette expansion format;
- renderer name;
- high-pixel-density setting;
- each resize/pixel-size event;
- close-request events;
- present failures.

Add an SDL3 frame dump option mirroring the X11 frame-dump idea, but keep it
driver-local:

- `sdl3.frame_dump_directory`;
- `sdl3.frame_dump_limit`;
- `sdl3.frame_dump_every`.

Dump either the expanded texture pixels before overlay or the final composited
software buffer if the overlay renderer draws into software. Do not require
macOS screenshots for automated pixel verification.

### Incremental Implementation Order

1. Add `CTH_BUILD_SDL3` CMake option, SDL3 package lookup, and an empty
   compiled-in SDL3 source file behind the option.
2. Add `DisplayDriverId`, parser support, config tests, and help text.
3. Add `DisplayDriverFactory`, `DisplayDriverRegistry`, and fake-factory tests.
4. Refactor `Application` enough that driver selection lives in the Display
   registry rather than in X11-specific calls.
5. Add `DisplayLifecycleEventSink` or a close-request field in
   `DisplayEventStats`.
6. Add pure SDL3 key mapper and tests.
7. Add pure SDL3 palette expansion and overlay layout tests.
8. Implement `Sdl3VideoContext`, `Sdl3Window`, and `Sdl3Renderer` with fake API
   lifecycle tests.
9. Implement the streaming texture present path with a deterministic unit test
   over fake locked texture memory.
10. Implement the real SDL3 event pump.
11. Add a small window-open smoke binary gated behind an opt-in CMake option or
    environment variable.
12. Run the macOS SDL3-only build with `CTH_BUILD_X11=OFF`.
13. Run the full application with `--display-driver=sdl3 --no-sound --silent`.
14. Add coexistence tests for builds where both X11 and SDL3 are compiled.
15. Only after SDL3 is stable, decide whether the default `auto` priority should
    remain X11-first or become platform-specific.

### Acceptance Criteria

The SDL3 macOS buildout is complete when:

- `cmake -S . -B build-sdl3-macos -G Ninja -DCTH_BUILD_X11=OFF
  -DCTH_BUILD_SDL3=ON -DCTH_BUILD_TESTS=ON` configures on macOS with Homebrew
  SDL3;
- `cmake --build build-sdl3-macos` succeeds;
- default `ctest` passes without opening windows;
- opt-in SDL3 window smoke tests pass from a macOS GUI session;
- the full application opens and presents frames with
  `--display-driver=sdl3 --no-sound --silent`;
- explicit unsupported driver selection fails with a configuration diagnostic;
- SDL3 source files contain the only SDL includes;
- X11 and SDL3 drivers can coexist behind the same registry when both are
  compiled;
- no SDL3 path reads or writes display globals, X11 globals, Interface globals,
  or `CthughaBuffer::current`;
- Display still receives all dependencies through constructors, open requests,
  per-frame method parameters, or `DisplaySystem`-owned objects.
