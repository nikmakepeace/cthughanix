# Runtime notes for `xcthugha`

This document describes the currently known runtime behaviour of `xcthugha`, the X11 build of Cthugha, in the verified Debian Etch environment.

The goal is to record what is known to work, what is untested, and what common runtime failures mean.

## Status

Verified:

- `xcthugha` starts on Debian Etch
- X11 output works
- Keyboard/input handling is usable
- Audio input works through the emulated sound card
- The program responds visually to input from the emulated sound device

Not yet verified:

- `cthugha` / SVGA build
- `glcthugha` (although TODO-2007.md says this doesn't build)
- Runtime behaviour on modern Linux distributions
- Runtime behaviour with ALSA, PulseAudio, or PipeWire-native devices
- Runtime behaviour without OSS compatibility

## Recommended command

In the verified VM/X11 setup, use:

```sh
./src/xcthugha --no-mit-shm
```

The exact binary path may differ depending on the build layout.

## X11 and MIT-SHM

`xcthugha` will try to use the MIT-SHM X11 extension.

MIT-SHM allows an X11 client and server to share memory for image transfer, avoiding slower socket-based image copies. This can improve performance when the X client and X server are on the same machine and share a compatible memory model.

In virtualised, forwarded, nested, or unusual X11 environments, MIT-SHM can fail.

A typical failure looks like:

```text
Initializing X11 display...
X Error of failed request:  BadRequest (invalid request code or no such operation)
  Major opcode of failed request:  133 (MIT-SHM)
  Minor opcode of failed request:  1 (X_ShmAttach)
```

If this happens, run:

```sh
./src/xcthugha --no-mit-shm
```

Although this option may not appear in the visible help text, it has been observed to work.

## Display performance

Observed performance in the verified environment:

- 320x200: approximately 90 FPS
- 640x480: approximately 25 FPS

But! This was a virtualised 

## Sound device expectations

The current code expects legacy OSS-style sound devices, commonly:

```text
/dev/dsp
/dev/mixer
```

If these devices are missing, startup may print messages like:

```text
Initializing the sound device...

Can't open `/dev/dsp' for reading. (2 - No such file or directory)

Can not use requested sound device. Using random noise.
Initializing sound server...
Initializing CD player...
Initializing Mixer device...

Can not open `/dev/mixer'. (2 - No such file or directory)
```

This means the program could not open the expected OSS sound device. It may fall back to random noise or degraded behaviour.

In the verified setup, audio input works when the VM exposes an emulated sound card in a way that provides usable OSS-compatible device access.

## Host microphone prompt in UTM

When running inside UTM on macOS, UTM may request microphone access from the host.

This can happen because the VM sound device is being backed by host audio input. If granted, the emulated sound card may provide input that `xcthugha` can read through the legacy sound path.

## Mixer device

The program may also attempt to open:

```text
/dev/mixer
```

Failure to open `/dev/mixer` does not necessarily prevent the program from running, but it indicates that mixer control through the old OSS interface is unavailable.

## Terminal settings

Some old Debian Etch tools may not recognise modern terminal names such as:

```text
xterm-256color
```

For better compatibility, use:

```sh
export TERM=xterm
```

If colour support causes issues, fall back further:

```sh
export TERM=vt100
```

For normal interactive work in Etch, `xterm` is usually the best balance of compatibility and usability.

## Known caveats

- `--no-mit-shm` may be required even though it is not advertised in the help text.
- Audio is still tied to legacy OSS assumptions.
- `/dev/dsp` and `/dev/mixer` may not exist on modern Linux systems.
- Running on modern Linux will likely require either OSS compatibility layers or code changes.
- The X11 path is the only currently verified graphics path.
- The SVGA and OpenGL paths are untested.

## Troubleshooting checklist

If `xcthugha` does not start:

1. Confirm that X11 is running.
2. Confirm that the `DISPLAY` environment variable is set.
3. Try running with MIT-SHM disabled:

   ```sh
   ./src/xcthugha --no-mit-shm
   ```

4. Confirm that the binary was built inside the Etch environment.
5. Confirm that the VM has a configured sound card if audio input is desired.
6. Check whether `/dev/dsp` exists:

   ```sh
   ls -l /dev/dsp
   ```

7. Check whether `/dev/mixer` exists:

   ```sh
   ls -l /dev/mixer
   ```

If the program runs but does not react to audio, the most likely cause is that the expected OSS audio input path is not connected to a live input source.

## Current interpretation

The current runtime state should be described as:

```text
xcthugha is verified to run on Debian Etch under X11, with audio input working through an emulated OSS-compatible sound device.
```

It should not yet be described as broadly portable or modern-Linux-ready.
