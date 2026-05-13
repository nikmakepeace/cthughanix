# DOS Verification Notes

These notes record how I checked the DOS source map and the DOS/Linux comparison.

## Source Inventory

I inventoried `cthug53s/` with:

```text
find cthug53s -maxdepth 3 -type f
wc -l cthug53s/*.C cthug53s/*.H cthug53s/*.ASM cthug53s/*.INC cthug53s/MAKEFILE
```

Result:

- 32 `.C` files.
- 41 `.H` files.
- 2 `.ASM` files.
- 3 `.INC` files.
- 4 `.LIB` files.
- 1 `.OBJ` file.
- 8 `.TD` files.
- About 21k lines in the inspected DOS source/header/assembly/makefile set.

## Build Inputs

I treated `cthug53s/MAKEFILE` as the authority for active source files and cross-checked its source lists against the flat directory. This is why `LUTS.C`, `ENVIRON.C`, and `FACEPREP.C` are called out as present but not active build inputs.

I did not build the DOS project. The makefile targets MSVC 1.0-era `cl`, MASM, DOS `link`, external binary libraries, and case-insensitive DOS paths.

## Runtime Flow Checks

The startup flow was traced through:

- `CTHUGHA.C::main()`
- `CTHUGHA.C::initstuff()`
- `CTHUGHA.C::pcx_and_palettes()`
- `FLAMES.C::flame_cro()`
- `AUDIO.C::get_stereo()`
- `DISPLAY.C::set_display_mode()`
- `DISPLAY.C::display_up()` and `display_shadowup()`

The frame order in `DOS_RUNTIME_MAP.md` is based on the active loop in `FLAMES.C`, not on a generic visualizer assumption.

## Effect Counts

I verified effect counts from the arrays themselves:

- `FLAMES.C::flamearray`: 15 real entries plus `<BAD>`.
- `MODES.C::wavearray`: 24 real entries plus `<BAD>`.
- `DISPLAY.C::disparray`: 8 real entries plus `<BAD>`.

I also checked `OPTIONS.C::check_flame_ok()`, `check_wave_ok()`, and `check_disp_ok()` to confirm that the enable/disable screens use those same arrays.

## Audio Checks

I inspected:

- `AUDIO.C` and `AUDIO.H` for the common function-pointer interface and noise/peak logic.
- `AUDIOGUS.C` for `ULTRASND` parsing and UltraSound record flow.
- `AUDIOSBP.C` plus `SB_DRIVE.C/H` for Sound Blaster/SB16 DMA, IRQ, DSP, mixer, and `BLASTER` parsing.
- `AUDIOPAS.C` and `AUDIOSDK.C` for PAS and old Creative SDK backends.
- `RECFILA.ASM` for PAS mixer muting/restoration.

This is also where the `close_audio` return-type mismatch and suspicious `audio_dma16` override condition were found.

## Asset Loader Checks

I specifically checked stale-comment risk in `CTHUGHA.C::pcx_and_palettes()`:

- Comments mention `PCX\*.pcx` and `MAP\*.map`.
- Active `findfirst()` calls scan `"*.pcx"` and `"*.map"`.

I checked:

- `SHOWP.C::pcxfilebuf()` for PCX constraints and palette conversion.
- `CTHUGHA.C::LoadLuts()` for `.map` parsing and 6-bit DAC conversion.
- `TRANSLAT.C::read_tabfile()` for fixed 320x204 translation table loading.
- `HI.C` for EMS/XMS storage behavior.

## Linux Cross-Checks

For `DOS_LINUX_DIFFERENCES.md`, I rechecked the Linux port's current source and existing root docs:

- `PROJECT_SUMMARY.md`
- `PROJECT_STRUCTURE.md`
- `PROJECT_RUNTIME_MAP.md`
- `src/initExitDisp.cc`
- `src/CthughaBuffer.*`
- `src/SoundDevice.cc`
- `src/SoundAnalyze.cc`
- `src/AutoChanger.cc`
- `src/SoundProcess.cc`
- `src/display.cc`
- `src/translate.cc`
- `src/palettes.cc`
- `src/pcx.cc`
- `src/CoreOption.*`

The comparison is based on those source checks, not only on the previous project summary.

## Known Limits Of This Review

- No DOS executable was built or run.
- No DOS emulator was used.
- I did not disassemble `UNCRUN_F.OBJ` or inspect binary `.LIB` internals.
- I inferred `.TD` file use from makefile dependencies and generated C/header pairs rather than reconstructing the original screen-data generation process.
- The DOS package is a source snapshot; it lacks the original runtime asset directories/files that would have accompanied a full install.
