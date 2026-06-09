Configuration warning from dos-fidelity.ini:7 `flashlight.?': wildcard ini entries are no longer supported; entry ignored
fortune.txt:7: rejected silence message: longer than 512 CP437 characters.
fortune.txt:46: rejected silence message: longer than 512 CP437 characters.
fortune.txt:249: rejected silence message: longer than 512 CP437 characters.
fortune.txt:365: rejected silence message: longer than 512 CP437 characters.
fortune.txt:405: rejected silence message: longer than 512 CP437 characters.
fortune.txt:415: rejected silence message: longer than 512 CP437 characters.
fortune.txt:609: rejected silence message: longer than 512 CP437 characters.
fortune.txt:981: rejected silence message: longer than 512 CP437 characters.
fortune.txt:1350: rejected silence message: longer than 512 CP437 characters.
fortune.txt:2027: rejected silence message: longer than 512 CP437 characters.
fortune.txt:2299: rejected silence message: longer than 512 CP437 characters.
fortune.txt:2614: rejected silence message: longer than 512 CP437 characters.
fortune.txt:2748: rejected silence message: longer than 512 CP437 characters.
fortune.txt:3253: rejected silence message: longer than 512 CP437 characters.
fortune.txt:3554: rejected silence message: longer than 512 CP437 characters.
fortune.txt:4044: rejected silence message: longer than 512 CP437 characters.
fortune.txt:4634: rejected silence message: longer than 512 CP437 characters.
fortune.txt:4998: rejected silence message: longer than 512 CP437 characters.
fortune.txt:5101: rejected silence message: longer than 512 CP437 characters.
fortune.txt:6057: rejected silence message: longer than 512 CP437 characters.
fortune.txt:6778: rejected silence message: longer than 512 CP437 characters.
fortune.txt:6866: rejected silence message: longer than 512 CP437 characters.
fortune.txt:7033: rejected silence message: non-CP437 character U+00F8 at column 6.
fortune.txt:7138: rejected silence message: longer than 512 CP437 characters.
fortune.txt:7419: rejected silence message: longer than 512 CP437 characters.
fortune.txt:8113: rejected silence message: longer than 512 CP437 characters.
fortune.txt:8443: rejected silence message: longer than 512 CP437 characters.
fortune.txt:8457: rejected silence message: longer than 512 CP437 characters.
fortune.txt:8486: rejected silence message: longer than 512 CP437 characters.
fortune.txt:8863: rejected silence message: longer than 512 CP437 characters.
fortune.txt:8923: rejected silence message: longer than 512 CP437 characters.
fortune.txt:8954: rejected silence message: longer than 512 CP437 characters.
fortune.txt:8980: rejected silence message: longer than 512 CP437 characters.
fortune.txt:9042: rejected silence message: longer than 512 CP437 characters.
fortune.txt:9344: rejected silence message: longer than 512 CP437 characters.
fortune.txt:9361: rejected silence message: longer than 512 CP437 characters.
fortune.txt:9371: rejected silence message: longer than 512 CP437 characters.
Loaded 2862 silence messages from `fortune.txt'.
Initializing the sound device...
runtime settings: audio-input-mode=2 rate=44000 channels=2 format=0 sound-dsp-method=3 silent=0 file=`tests/fixtures/audio/prism.mp3' miniaudio-capture-device=`'
runtime environment: dev-dsp=`/dev/dsp' oss-input=0 oss-output=0 pulse-output=0 miniaudio-output=1 miniaudio-capture=1
runtime factory: created with audio-input-mode=2 sound-dsp-method=3 silent=0 output-driver=auto visual-max-dimension=320 oss-input=0 oss-output=0 pulse-output=0 miniaudio-output=1 miniaudio-capture=1 pulse-server=`default'
runtime factory: output config pulse-latency-ms=250 pulse-target-latency-ms=250 miniaudio-target-latency-ms=250 miniaudio-playback-device=`' null-target-latency-ms=0 dsp-target-latency-ms=250 output-dump=`'
    pcm source strategy: selected strategy=mp3-file audio-input-mode=2 file=`tests/fixtures/audio/prism.mp3'
    audio input strategy: selecting AudioInput for audio-input-mode=2
    audio input strategy: native PCM input from mp3-file source
    pcm source strategy: selected strategy=mp3-file audio-input-mode=2 file=`tests/fixtures/audio/prism.mp3'
    pcm source strategy: creating Minimp3PcmSource file=`tests/fixtures/audio/prism.mp3'
Playing file 'tests/fixtures/audio/prism.mp3'.
minimp3 pcm source: opening `tests/fixtures/audio/prism.mp3'
minimp3 pcm source: format rate=48000 channels=2 sample-format=3
    audio input strategy: selected AudioInput with source strategy=mp3-file
audio input: created format rate=48000 channels=2 format=3 loop=1
    audio output strategy: selecting AudioOutput silent=0 output-driver=auto pulse-output=0 miniaudio-output=1 oss-output=0
    audio output strategy: automatic candidate count=1 prefers-miniaudio=0
    audio output strategy: trying miniaudio output
    audio output strategy: miniaudio playback connected backend=PulseAudio device=`Virtio 1.0 sound Stereo' rate=48000 channels=2 format=2 direct-copy=1 target-latency-ms=250 requested-period-ms=50 requested-periods=4 internal-period-frames=1920 internal-periods=5 presentation-delay-samples=21600
    audio output strategy: selected AudioMiniAudioOutput
audio output: configured timing realtime=1 samples-per-second=48000 bytes-per-sample=4 input-chunk-samples=2400 target-buffer-ms=250 target-buffer-samples=12000 scratch-samples=12000
decoded audio history: created capacity-samples=144000 rate=48000 channels=2 format=3 bytes-per-sample=4 retained-history-samples=48000
audio output: configured timing realtime=1 samples-per-second=48000 bytes-per-sample=4 input-chunk-samples=2400 target-buffer-ms=250 target-buffer-samples=12000 scratch-samples=12000
audio runtime: miniaudio callback drain started scratch-samples=12000 target-buffer-samples=12000 queued-samples=0 input-finished=0
audio passthrough: started realtime=1 scratch-samples=12000 target-queue-samples=12000 callback-drain=1 worker-thread=0
audio ingest: started input=0xaaaabefe8250 passthrough=0xaaaabf04bcb0 sample-rate=48000 channels=2 format=3 bytes-per-sample=4 input-chunk-samples=2400 decode-ahead-samples=21600 retained-samples=48000 capacity-samples=144000 worker-thread=1
Initializing Frame Generator storage...
audio ingest: input thread started chunk-samples=2400
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=2400 submitted-end-sample=0 presentation-delay-samples=21600 underflows=0
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=4800 presentation-delay-samples=21600 underflows=0
  loading translation tables...
  number of loaded translates: 11
  loading 3-D objects...    loading: ./resources/obj/bigK.obj ... OK
    loading: ./resources/obj/Cube4.obj ... OK

  number of 3-D objects: 3
  loading palettes...
    loading: ./resources/map/560sel.map ... OK
    loading: ./resources/map/8rain.map ... OK
    loading: ./resources/map/8rain2.map ... OK
    loading: ./resources/map/8temp.map
    Reached end of palette after 128 entries (8temp) ... filling with black ... OK
    loading: ./resources/map/altern.map ... OK
    loading: ./resources/map/bandw.map ... OK
    loading: ./resources/map/basic.map ... OK
    loading: ./resources/map/bbr001.map ... OK
    loading: ./resources/map/blend1.map ... OK
    loading: ./resources/map/blend3.map ... OK
    loading: ./resources/map/blend4.map ... OK
    loading: ./resources/map/blend8.map ... OK
    loading: ./resources/map/bluein.map ... OK
    loading: ./resources/map/blues.map ... OK
    loading: ./resources/map/bluorng.map ... OK
    loading: ./resources/map/chroma2.map ... OK
    loading: ./resources/map/chroma3.map ... OK
    loading: ./resources/map/chroma4.map ... OK
    loading: ./resources/map/chroma5.map ... OK
    loading: ./resources/map/clouds.map ... OK
    loading: ./resources/map/coldfire.map ... OK
    loading: ./resources/map/cool.map ... OK
    loading: ./resources/map/darkie.map ... OK
    loading: ./resources/map/egan2.map ... OK
    loading: ./resources/map/egan3.map ... OK
    loading: ./resources/map/egan4.map ... OK
    loading: ./resources/map/fadechr1.map ... OK
    loading: ./resources/map/fadern2.map ... OK
    loading: ./resources/map/fadern3.map ... OK
    loading: ./resources/map/fadern4.map ... OK
    loading: ./resources/map/fire2.map ... OK
    loading: ./resources/map/firestrm.map ... OK
    loading: ./resources/map/fourain2.map ... OK
    loading: ./resources/map/frcoast.map ... OK
    loading: ./resources/map/froth3.map ... OK
    loading: ./resources/map/froth316.map
    Reached end of palette after 16 entries (froth316) ... filling with black ... OK
    loading: ./resources/map/froth6.map ... OK
    loading: ./resources/map/gamma1.map ... OK
    loading: ./resources/map/glasses1.map ... OK
    loading: ./resources/map/glasses2.map ... OK
    loading: ./resources/map/grayish.map ... OK
    loading: ./resources/map/greenin.map ... OK
    loading: ./resources/map/greenout.map ... OK
    loading: ./resources/map/grey.map ... OK
    loading: ./resources/map/hawaii.map ... OK
    loading: ./resources/map/hybrid.map ... OK
    loading: ./resources/map/indigo.map ... OK
    loading: ./resources/map/internal_0.map ... OK
    loading: ./resources/map/internal_1.map ... OK
    loading: ./resources/map/internal_2.map ... OK
    loading: ./resources/map/internal_4.map ... OK
    loading: ./resources/map/jellyfsh.map ... OK
    loading: ./resources/map/juteblue.map ... OK
    loading: ./resources/map/jutemap.map ... OK
    loading: ./resources/map/jutemap2.map ... OK
    loading: ./resources/map/jutes.map ... OK
    loading: ./resources/map/light.map ... OK
    loading: ./resources/map/litnin1.map ... OK
    loading: ./resources/map/longs.map ... OK
    loading: ./resources/map/lyapunov.map ... OK
    loading: ./resources/map/mandel.map ... OK
    loading: ./resources/map/mandel2.map ... OK
    loading: ./resources/map/mandmap.map ... OK
    loading: ./resources/map/necklace.map ... OK
    loading: ./resources/map/neon.map ... OK
    loading: ./resources/map/new.map ... OK
    loading: ./resources/map/newmap.map ... OK
    loading: ./resources/map/nice.map ... OK
    loading: ./resources/map/nkohala.map ... OK
    loading: ./resources/map/okay.map ... OK
    loading: ./resources/map/owl.map ... OK
    loading: ./resources/map/povray.map ... OK
    loading: ./resources/map/rain4g1.map ... OK
    loading: ./resources/map/rainbow.map ... OK
    loading: ./resources/map/rainbow2.map ... OK
    loading: ./resources/map/rainbow3.map ... OK
    loading: ./resources/map/rainbow4.map ... OK
    loading: ./resources/map/rainbow5.map ... OK
    loading: ./resources/map/rainbow6.map ... OK
    loading: ./resources/map/rainbow7.map ... OK
    loading: ./resources/map/rainhyb.map ... OK
    loading: ./resources/map/rby.map ... OK
    loading: ./resources/map/redand.map ... OK
    loading: ./resources/map/redgrn.map ... OK
    loading: ./resources/map/reds.map ... OK
    loading: ./resources/map/royal.map ... OK
    loading: ./resources/map/sanfran.map ... OK
    loading: ./resources/map/sea.map ... OK
    loading: ./resources/map/sea2.map ... OK
    loading: ./resources/map/smooth.map ... OK
    loading: ./resources/map/topo.map ... OK
    loading: ./resources/map/tworain3.map ... OK
    loading: ./resources/map/volcano.map ... OK
    loading: ./resources/map/volcano2.map ... OK
    loading: ./resources/map/volcano3.map ... OK
    loading: ./resources/map/volcano4.map ... OK
    loading: ./resources/map/volcano5.map ... OK
    loading: ./resources/map/VOLCANO6.map ... OK
    loading: ./resources/map/volcano7.map ... OK
    loading: ./resources/map/yelpurp1.map ... OK
  number of loaded palettes: 101
  palette set filter `cthugha53': 23 palettes enabled
brightening palette 0 (560sel). Faktor: 1.028
brightening palette 1 (8rain). Faktor: 1.530
brightening palette 2 (8rain2). Faktor: 1.518
brightening palette 3 (8temp). Faktor: 1.518
brightening palette 4 (altern). Faktor: 1.012
brightening palette 5 (bandw). Faktor: 1.181
brightening palette 6 (basic). Faktor: 1.012
brightening palette 7 (bbr001). Faktor: 1.012
brightening palette 8 (blend1). Faktor: 1.118
brightening palette 9 (blend3). Faktor: 1.376
brightening palette 10 (blend4). Faktor: 1.012
brightening palette 11 (blend8). Faktor: 1.386
brightening palette 12 (bluein). Faktor: 3.036
brightening palette 13 (blues). Faktor: 1.012
brightening palette 14 (bluorng). Faktor: 1.518
brightening palette 15 (chroma2). Faktor: 1.138
brightening palette 16 (chroma3). Faktor: 1.012
brightening palette 17 (chroma4). Faktor: 1.012
brightening palette 18 (chroma5). Faktor: 1.138
brightening palette 19 (clouds). Faktor: 1.012
brightening palette 20 (coldfire). Faktor: 1.012
brightening palette 21 (cool). Faktor: 1.952
brightening palette 22 (darkie). Faktor: 1.012
brightening palette 23 (egan2). Faktor: 1.062
brightening palette 24 (egan3). Faktor: 1.292
brightening palette 25 (egan4). Faktor: 1.449
brightening palette 26 (fadechr1). Faktor: 1.292
brightening palette 27 (fadern2). Faktor: 1.518
brightening palette 28 (fadern3). Faktor: 1.518
brightening palette 29 (fadern4). Faktor: 1.518
brightening palette 30 (fire2). Faktor: 1.518
brightening palette 31 (firestrm). Faktor: 1.997
brightening palette 32 (fourain2). Faktor: 1.518
brightening palette 33 (frcoast). Faktor: 1.012
brightening palette 34 (froth3). Faktor: 3.036
brightening palette 35 (froth316). Faktor: 3.036
brightening palette 36 (froth6). Faktor: 1.012
brightening palette 37 (gamma1). Faktor: 1.012
brightening palette 38 (glasses1). Faktor: 3.036
brightening palette 39 (glasses2). Faktor: 1.594
brightening palette 40 (grayish). Faktor: 1.028
brightening palette 41 (greenin). Faktor: 3.036
brightening palette 42 (greenout). Faktor: 3.036
brightening palette 44 (hawaii). Faktor: 1.542
brightening palette 45 (hybrid). Faktor: 1.518
brightening palette 46 (indigo). Faktor: 1.376
brightening palette 47 (internal_0). Faktor: 1.012
brightening palette 48 (internal_1). Faktor: 1.138
brightening palette 49 (internal_2). Faktor: 1.518
brightening palette 50 (internal_4). Faktor: 1.012
brightening palette 51 (jellyfsh). Faktor: 1.012
brightening palette 52 (juteblue). Faktor: 1.012
brightening palette 53 (jutemap). Faktor: 1.012
brightening palette 54 (jutemap2). Faktor: 1.012
brightening palette 55 (jutes). Faktor: 1.023
brightening palette 56 (light). Faktor: 1.542
brightening palette 57 (litnin1). Faktor: 1.012
brightening palette 58 (longs). Faktor: 1.012
brightening palette 59 (lyapunov). Faktor: 1.723
brightening palette 60 (mandel). Faktor: 2.013
brightening palette 61 (mandel2). Faktor: 1.012
brightening palette 62 (mandmap). Faktor: 1.012
brightening palette 63 (necklace). Faktor: 1.118
brightening palette 64 (neon). Faktor: 1.518
brightening palette 65 (new). Faktor: 1.012
brightening palette 66 (newmap). Faktor: 1.518
brightening palette 67 (nice). Faktor: 1.012
brightening palette 68 (nkohala). Faktor: 1.012
brightening palette 69 (okay). Faktor: 1.188
brightening palette 70 (owl). Faktor: 1.093
brightening palette 71 (povray). Faktor: 1.012
brightening palette 72 (rain4g1). Faktor: 1.012
brightening palette 73 (rainbow). Faktor: 1.518
brightening palette 74 (rainbow2). Faktor: 1.518
brightening palette 75 (rainbow3). Faktor: 1.992
brightening palette 76 (rainbow4). Faktor: 1.518
brightening palette 77 (rainbow5). Faktor: 1.518
brightening palette 78 (rainbow6). Faktor: 1.012
brightening palette 79 (rainbow7). Faktor: 1.518
brightening palette 80 (rainhyb). Faktor: 1.518
brightening palette 81 (rby). Faktor: 1.518
brightening palette 82 (redand). Faktor: 1.366
brightening palette 83 (redgrn). Faktor: 1.203
brightening palette 84 (reds). Faktor: 1.012
brightening palette 85 (royal). Faktor: 1.012
brightening palette 86 (sanfran). Faktor: 1.159
brightening palette 87 (sea). Faktor: 1.012
brightening palette 88 (sea2). Faktor: 1.012
brightening palette 89 (smooth). Faktor: 1.012
brightening palette 90 (topo). Faktor: 1.012
brightening palette 91 (tworain3). Faktor: 1.012
brightening palette 92 (volcano). Faktor: 1.012
brightening palette 93 (volcano2). Faktor: 1.012
brightening palette 94 (volcano3). Faktor: 1.012
brightening palette 95 (volcano4). Faktor: 1.012
brightening palette 96 (volcano5). Faktor: 1.518
brightening palette 97 (VOLCANO6). Faktor: 1.518
brightening palette 98 (volcano7). Faktor: 1.012
brightening palette 99 (yelpurp1). Faktor: 1.518
brightening palette 100 (general). Faktor: 1.012
  loading image files...
    loading: ./resources/img/cthugha.pcxpcx: version:5, compr:1, ncolpl.: 1, greysc.:1, X:0-319, Y:0-199
pcx: loaded source palette from `cthugha'
 ... OK
    loading: ./resources/img/ill.pcxpcx: version:5, compr:1, ncolpl.: 1, greysc.:1, X:0-319, Y:0-199
pcx: loaded source palette from `ill'
 ... OK
    loading: ./resources/img/sti_ephr.pcxpcx: version:5, compr:1, ncolpl.: 1, greysc.:1, X:0-319, Y:0-199
pcx: loaded source palette from `sti_ephr'
 ... OK
    loading: ./resources/img/teardrop.pcxpcx: version:5, compr:1, ncolpl.: 1, greysc.:1, X:0-319, Y:0-199
pcx: loaded source palette from `teardrop'
 ... OK
    loading: ./resources/img/torus.pcxpcx: version:5, compr:1, ncolpl.: 1, greysc.:1, X:0-319, Y:0-199
pcx: loaded source palette from `torus'
 ... OK
    loading: ./resources/img/zaph.pcxpcx: version:5, compr:1, ncolpl.: 1, greysc.:1, X:0-319, Y:0-239
pcx: loaded source palette from `zaph'
 ... OK
    loading: ./resources/img/dolphin.pngpng: width=320 height=200 bit-depth=8 color-type=3 interlace=0
 ... OK
  number of loaded image files: 8
Setting initial effect controls...
changing option `display' to `locked:15'.
Initializing interface...
Registering key actions...
Initializing keymaps...
      adding new keymap 'default'.
      defining keymap 'default'.
keymap: parsed new keymap entry line 'q setInterface(main)'
keymap: parsed new keymap entry line 'o   setInterface(EffectControls)'
keymap: parsed new keymap entry line 'O   setInterface(Options)'
keymap: parsed new keymap entry line '?   setInterface(Help)'
keymap: parsed new keymap entry line 'F1  setInterface(Help)'
keymap: parsed new keymap entry line 'F2  setInterface(EffectControls)'
keymap: parsed new keymap entry line 'F3  setInterface(Options)'
keymap: parsed new keymap entry line 'F4  toggleFPS()'
keymap: parsed new keymap entry line 'F5  setInterface(sound)'
keymap: parsed new keymap entry line 'F6  setInterface(mixer)'
keymap: parsed new keymap entry line 'F7  setInterface(playList)'
keymap: parsed new keymap entry line 'F8  setInterface(playList)'
keymap: parsed new keymap entry line 'F9   setInterface(display)'
keymap: parsed new keymap entry line 'F10  setInterface(flame)'
keymap: parsed new keymap entry line 'F11  setInterface(border)'
keymap: parsed new keymap entry line 'F12  setInterface(translate)'
keymap: parsed new keymap entry line 'F13  setInterface(wave)'
keymap: parsed new keymap entry line 'F14  setInterface(table)'
keymap: parsed new keymap entry line 'F15  setInterface(waveScaling)'
keymap: parsed new keymap entry line 'F16  setInterface(object)'
keymap: parsed new keymap entry line 'F17  setInterface(palette)'
keymap: parsed new keymap entry line 'F18  setInterface(image)'
keymap: parsed new keymap entry line 'F19  setInterface(flashlight)'
keymap: parsed new keymap entry line 'Left  prevInterface()'
keymap: parsed new keymap entry line 'Right nextInterface()'
keymap: parsed new keymap entry line 'Up   up(1)'
keymap: parsed new keymap entry line 'Down down(1)'
keymap: parsed new keymap entry line 'home home()'
keymap: parsed new keymap entry line 'end  end()'
keymap: parsed new keymap entry line '.    toggleStatus()'
keymap: parsed new keymap entry line ',    toggleStatus()'
      adding new keymap 'main'.
      defining keymap 'main'.
keymap: parsed new keymap entry line 'q credits()'
keymap: parsed new keymap entry line 'Q quit()'
keymap: parsed new keymap entry line 'C stopAndContinue()'
keymap: parsed new keymap entry line 'd screenChg(+1)'
keymap: parsed new keymap entry line 'D screenChg(-1)'
keymap: parsed new keymap entry line 'z zoomChg(+1)'
keymap: parsed new keymap entry line 'Z zoomChg(-1)'
keymap: parsed new keymap entry line 'f flameChg(+1)'
keymap: parsed new keymap entry line 'F flameChg(-1)'
keymap: parsed new keymap entry line 'g flameGeneral()'
keymap: parsed new keymap entry line 'w waveChg(+1)'
keymap: parsed new keymap entry line 'W waveScaleChg(+1)'
keymap: parsed new keymap entry line 'j objectChg(+1)'
keymap: parsed new keymap entry line 'J objectChg(-1)'
keymap: parsed new keymap entry line 't translateChg(+1)'
keymap: parsed new keymap entry line 'T translateChg(-1)'
keymap: parsed new keymap entry line 'm soundProcessChg(+1)'
keymap: parsed new keymap entry line 'M soundProcessChg(-1)'
keymap: parsed new keymap entry line 's flashlightChg(+1)'
keymap: parsed new keymap entry line 'S flashlightChg(-1)'
keymap: parsed new keymap entry line 'p paletteChg(+1)'
keymap: parsed new keymap entry line 'P paletteChg(-1)'
keymap: parsed new keymap entry line 'b tableChg(+1)'
keymap: parsed new keymap entry line 'B tableChg(-1)'
keymap: parsed new keymap entry line 'x imageChg(+1)'
keymap: parsed new keymap entry line 'X imageChg(-1)'
keymap: parsed new keymap entry line 'l lockChg()'
keymap: parsed new keymap entry line 'L lockChg()'
keymap: parsed new keymap entry line 'r randomPalette()'
keymap: parsed new keymap entry line 'R newRandomPalette()'
keymap: parsed new keymap entry line 'a writeIni()'
keymap: parsed new keymap entry line 'A writeIni()'
keymap: parsed new keymap entry line 'backSpace restore()'
keymap: parsed new keymap entry line 'h toggleSave()'
keymap: parsed new keymap entry line '0 saveOrRestore(0)'
keymap: parsed new keymap entry line '1 saveOrRestore(1)'
keymap: parsed new keymap entry line '2 saveOrRestore(2)'
keymap: parsed new keymap entry line '3 saveOrRestore(3)'
keymap: parsed new keymap entry line '4 saveOrRestore(4)'
keymap: parsed new keymap entry line '5 saveOrRestore(5)'
keymap: parsed new keymap entry line '6 saveOrRestore(6)'
keymap: parsed new keymap entry line '7 saveOrRestore(7)'
keymap: parsed new keymap entry line '8 saveOrRestore(8)'
keymap: parsed new keymap entry line '9 saveOrRestore(9)'
keymap: parsed new keymap entry line '\ changeAll()'
keymap: parsed new keymap entry line 'return changeOne()'
keymap: parsed new keymap entry line 'S-0 save(0)'
keymap: parsed new keymap entry line 'S-1 save(1)'
keymap: parsed new keymap entry line 'S-2 save(2)'
keymap: parsed new keymap entry line 'S-3 save(3)'
keymap: parsed new keymap entry line 'S-4 save(4)'
keymap: parsed new keymap entry line 'S-5 save(5)'
keymap: parsed new keymap entry line 'S-6 save(6)'
keymap: parsed new keymap entry line 'S-7 save(7)'
keymap: parsed new keymap entry line 'S-8 save(8)'
keymap: parsed new keymap entry line 'S-9 save(9)'
      adding new keymap 'help'.
      defining keymap 'help'.
keymap: parsed new keymap entry line 'up   scrollUp()'
keymap: parsed new keymap entry line 'down scrollDown()'
keymap: parsed new keymap entry line '\\   toggleScrolling()'
keymap: parsed new keymap entry line '?    setInterface(main)'
keymap: parsed new keymap entry line 'F1   setInterface(main)'
      adding new keymap 'Sound'.
      defining keymap 'Sound'.
keymap: parsed new keymap entry line 'F5 setInterface(main)'
      adding new keymap 'Options'.
      defining keymap 'Options'.
keymap: parsed new keymap entry line 'Q setInterface(main)'
keymap: parsed new keymap entry line 'O setInterface(main)'
      adding new keymap 'EffectControls'.
      defining keymap 'EffectControls'.
keymap: parsed new keymap entry line 'o setInterface(main)'
keymap: parsed new keymap entry line 'O setInterface(options)'
      adding new keymap 'OptionElement'.
      defining keymap 'OptionElement'.
keymap: parsed new keymap entry line 'Right chgValue1(+1)'
keymap: parsed new keymap entry line 'Left  chgValue1(-1)'
keymap: parsed new keymap entry line '+     chgValue2(+1)'
keymap: parsed new keymap entry line '-     chgValue2(-1)'
keymap: parsed new keymap entry line '*     chgValue3(+1)'
keymap: parsed new keymap entry line '/     chgValue3(-1)'
keymap: parsed new keymap entry line '0     setValue(0)'
keymap: parsed new keymap entry line '1     setValue(1)'
keymap: parsed new keymap entry line '2     setValue(2)'
keymap: parsed new keymap entry line '3     setValue(3)'
keymap: parsed new keymap entry line '4     setValue(4)'
keymap: parsed new keymap entry line '5     setValue(5)'
keymap: parsed new keymap entry line '6     setValue(6)'
keymap: parsed new keymap entry line '7     setValue(7)'
keymap: parsed new keymap entry line '8     setValue(8)'
keymap: parsed new keymap entry line '9     setValue(9)'
      adding new keymap 'EffectControlElement'.
      defining keymap 'EffectControlElement'.
keymap: parsed new keymap entry line 'l   lockElement()'
      adding new keymap 'ListOption'.
      defining keymap 'ListOption'.
keymap: parsed new keymap entry line 'Right  toggleUse()'
keymap: parsed new keymap entry line 'Left   toggleUse()'
keymap: parsed new keymap entry line '+      toggleUse()'
keymap: parsed new keymap entry line '-      toggleUse()'
keymap: parsed new keymap entry line '/      toggleUse()'
keymap: parsed new keymap entry line '*      toggleUse()'
keymap: parsed new keymap entry line 'Return setInterface(main) activate()'
keymap: parsed new keymap entry line '\\     activate()'
Initializing display...
Initializing X11 display...
    display size       : 640x480
    color planes       : 24
    using no shared image; staging through server pixmap.
    bytes/pixel        : 4
    bytes/line         : 2560
    red   mask/shift   : 0xff0000/16
    green mask/shift   : 0xff00/8
    blue  mask/shift   : 0x  ff/0
frame generator: default stage sequence stages=10
frame filterchain: set sequence stages=10
frame filterchain: added stage=0 filter=0xaaaabf063000 owned=1 mode=0 size=1
frame filterchain: added stage=2 filter=0xaaaabf033f60 owned=1 mode=0 size=2
frame filterchain: added stage=3 filter=0xaaaabfcf8ac0 owned=1 mode=0 size=3
frame filterchain: added stage=4 filter=0xaaaabfcf68b0 owned=1 mode=0 size=4
frame filterchain: added stage=5 filter=0xaaaabf062530 owned=1 mode=0 size=5
frame filterchain: added stage=6 filter=0xaaaabf0625d0 owned=1 mode=0 size=6
frame filterchain: added stage=7 filter=0xaaaabfcf6510 owned=1 mode=0 size=7
frame filterchain: added stage=8 filter=0xaaaabfcf7190 owned=1 mode=0 size=8
frame filterchain: added stage=1 filter=0xaaaabf034860 owned=1 mode=0 size=9
frame filterchain: added stage=9 filter=0xaaaabf062a90 owned=1 mode=0 size=10
frame filterchain factory: created filterchain=0xaaaabfcf67a0 stages=10 filters=10
frame generator: default stage sequence stages=10
Initializing the audio-visual bridge...
audio frame pipeline: creating pipeline
frame pacing: nextFrame previous-now=0.000000 sampled-now=31788.448257 raw-delta-ms=0.000
frame pacing: updateFPS deltaT-ms=0.000 fps=0.000 rolling-fps=0.000
audio frame builder: resized raw buffer to 4096 bytes
audio frame builder: built frame center-sample=0 start-sample=0 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.006 decoded-end-sample=26400 presentation-sample=0 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=0
audio frame pipeline: frame-ms=0.005 process-ms=0.000 analyze-ms=0.004
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
palette transition target set strategy=hsl frames=0
frame filterchain: set stage=1 mode=0 entries=1
frame filterchain: set stage=2 mode=1 entries=1
frame filterchain: set stage=3 mode=1 entries=1
frame filterchain: set stage=4 mode=1 entries=1
frame filterchain: set stage=5 mode=1 entries=1
frame filterchain: set stage=7 mode=1 entries=1
frame filterchain: set stage=8 mode=1 entries=1
frame filterchain: set stage=9 mode=1 entries=1
frame filterchain: set stage=0 mode=2 entries=1
palette transition target set strategy=linear frames=0
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: executing image stage
frame filterchain: disarming one-shot stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=63965 peak-pixel=128 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=5.937 palette-ms=0.002 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=5.928 copy-ms=0.001 flush-ms=0.005 copy-text=0 full-copy=1 shm-level=0 draw=640x480
display timing: x11 frame-ms=8.135 palette-ms=0.002 compose-ms=0.015 prepare-ms=0.000 viewport-ms=0.000 overlay-ms=0.004 transfer-clear-ms=2.174 post-ms=5.940 size=640x480 draw=640x480
display timing: visual-latency observed-ms=8.137 previous-ms=0.000 alpha=0.100 estimate-ms=8.137
frame timing: total-ms=11.321 next-frame=0.003 audio=0.008 bridge=0.006 buffer=3.165 display=8.137 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=7200 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=40.000 requested-sleep-ms=28.678 actual-pacing-ms=28.820 maxfps=25
display timing: mainloop-ms=40.164 events-ms=0.018 pre-interface-ms=0.001 frame-ms=11.322 post-interface-ms=0.000 pacing-ms=28.820 events=0 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31788.448257 sampled-now=31788.488422 raw-delta-ms=40.165
frame pacing: updateFPS deltaT-ms=40.165 fps=24.897 rolling-fps=24.897
audio frame builder: built frame center-sample=0 start-sample=0 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.004 decoded-end-sample=28800 presentation-sample=0 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=0
audio frame pipeline: frame-ms=0.004 process-ms=0.001 analyze-ms=0.004
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=66550 peak-pixel=128 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.243 palette-ms=0.002 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.238 copy-ms=0.000 flush-ms=0.002 copy-text=0 full-copy=1 shm-level=0 draw=640x480
display timing: x11 frame-ms=2.351 palette-ms=0.002 compose-ms=0.006 prepare-ms=0.000 viewport-ms=0.000 overlay-ms=0.001 transfer-clear-ms=2.097 post-ms=0.245 size=640x480 draw=640x480
display timing: visual-latency observed-ms=2.353 previous-ms=8.137 alpha=0.100 estimate-ms=7.559
frame timing: total-ms=5.765 next-frame=0.002 audio=0.005 bridge=0.006 buffer=3.397 display=2.353 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=9600 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.835 requested-sleep-ms=34.069 actual-pacing-ms=34.235 maxfps=25
display timing: mainloop-ms=40.022 events-ms=0.018 pre-interface-ms=0.000 frame-ms=5.766 post-interface-ms=0.000 pacing-ms=34.235 events=7 resize-events=3 expose-events=2
frame pacing: nextFrame previous-now=31788.488422 sampled-now=31788.528434 raw-delta-ms=40.012
frame pacing: updateFPS deltaT-ms=40.012 fps=24.993 rolling-fps=24.945
audio frame builder: built frame center-sample=0 start-sample=0 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.005 decoded-end-sample=31200 presentation-sample=0 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=0
audio frame pipeline: frame-ms=0.005 process-ms=0.001 analyze-ms=0.004
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=67064 peak-pixel=128 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.104 palette-ms=0.002 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.098 copy-ms=0.000 flush-ms=0.003 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=2.384 palette-ms=0.002 compose-ms=0.006 prepare-ms=0.000 viewport-ms=0.001 overlay-ms=0.001 transfer-clear-ms=2.267 post-ms=0.106 size=640x480 draw=640x480
display timing: visual-latency observed-ms=2.386 previous-ms=7.559 alpha=0.100 estimate-ms=7.041
frame timing: total-ms=6.081 next-frame=0.002 audio=0.006 bridge=0.007 buffer=3.679 display=2.386 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=12000 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.824 requested-sleep-ms=33.742 actual-pacing-ms=33.950 maxfps=25
display timing: mainloop-ms=40.041 events-ms=0.005 pre-interface-ms=0.000 frame-ms=6.082 post-interface-ms=0.000 pacing-ms=33.950 events=0 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31788.528434 sampled-now=31788.568532 raw-delta-ms=40.098
frame pacing: updateFPS deltaT-ms=40.098 fps=24.939 rolling-fps=24.943
audio frame builder: built frame center-sample=0 start-sample=0 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.005 decoded-end-sample=33600 presentation-sample=0 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=0
audio frame pipeline: frame-ms=0.005 process-ms=0.001 analyze-ms=0.004
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=67134 peak-pixel=128 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=14400 presentation-delay-samples=21600 underflows=0
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=16800 presentation-delay-samples=21600 underflows=0
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=19200 presentation-delay-samples=21600 underflows=0
display timing: post-draw-ms=173.612 palette-ms=0.002 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.101 copy-ms=0.000 flush-ms=173.506 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=176.142 palette-ms=0.003 compose-ms=0.007 prepare-ms=0.000 viewport-ms=0.001 overlay-ms=0.002 transfer-clear-ms=2.495 post-ms=173.634 size=640x480 draw=640x480
display timing: visual-latency observed-ms=176.149 previous-ms=7.041 alpha=0.100 estimate-ms=23.952
frame timing: total-ms=180.301 next-frame=0.003 audio=0.007 bridge=0.007 buffer=4.129 display=176.150 do-display=1
frame pacing: pacing target-ms=39.726 requested-sleep-ms=0.000 actual-pacing-ms=0.002 maxfps=25
display timing: mainloop-ms=180.336 events-ms=0.009 pre-interface-ms=0.000 frame-ms=180.310 post-interface-ms=0.005 pacing-ms=0.002 events=0 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31788.568532 sampled-now=31788.748882 raw-delta-ms=180.350
frame pacing: updateFPS deltaT-ms=180.350 fps=5.545 rolling-fps=13.306
audio frame builder: built frame center-sample=0 start-sample=0 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.016 decoded-end-sample=40800 presentation-sample=0 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=0
audio frame pipeline: frame-ms=0.030 process-ms=0.003 analyze-ms=0.027
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=21600 presentation-delay-samples=21600 underflows=0
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=66988 peak-pixel=128 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=24000 presentation-delay-samples=21600 underflows=0
display timing: post-draw-ms=46.881 palette-ms=0.007 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=46.848 copy-ms=0.002 flush-ms=0.018 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=52.899 palette-ms=0.007 compose-ms=0.018 prepare-ms=0.001 viewport-ms=0.002 overlay-ms=0.005 transfer-clear-ms=5.965 post-ms=46.901 size=640x480 draw=640x480
display timing: visual-latency observed-ms=52.906 previous-ms=23.952 alpha=0.100 estimate-ms=26.847
frame timing: total-ms=67.899 next-frame=0.006 audio=0.020 bridge=0.038 buffer=14.923 display=52.907 do-display=1
frame pacing: pacing target-ms=40.000 requested-sleep-ms=0.000 actual-pacing-ms=0.001 maxfps=25
display timing: mainloop-ms=67.938 events-ms=0.020 pre-interface-ms=0.000 frame-ms=67.906 post-interface-ms=0.005 pacing-ms=0.001 events=0 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31788.748882 sampled-now=31788.816836 raw-delta-ms=67.954
frame pacing: updateFPS deltaT-ms=67.954 fps=14.716 rolling-fps=13.566
audio frame builder: built frame center-sample=2400 start-sample=1888 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.026 decoded-end-sample=45600 presentation-sample=2400 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=2400
audio frame pipeline: frame-ms=0.026 process-ms=0.003 analyze-ms=0.023
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=66729 peak-pixel=52 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.350 palette-ms=0.006 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.332 copy-ms=0.001 flush-ms=0.008 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=5.505 palette-ms=0.009 compose-ms=0.021 prepare-ms=0.001 viewport-ms=0.003 overlay-ms=0.006 transfer-clear-ms=5.107 post-ms=0.358 size=640x480 draw=640x480
display timing: visual-latency observed-ms=5.511 previous-ms=26.847 alpha=0.100 estimate-ms=24.714
frame timing: total-ms=18.806 next-frame=0.006 audio=0.032 bridge=0.034 buffer=13.219 display=5.511 do-display=1
frame pacing: pacing target-ms=12.046 requested-sleep-ms=0.000 actual-pacing-ms=0.001 maxfps=25
display timing: mainloop-ms=18.851 events-ms=0.033 pre-interface-ms=0.000 frame-ms=18.811 post-interface-ms=0.003 pacing-ms=0.001 events=2 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31788.816836 sampled-now=31788.835721 raw-delta-ms=18.885
frame pacing: updateFPS deltaT-ms=18.885 fps=52.952 rolling-fps=15.485
audio frame builder: built frame center-sample=2400 start-sample=1888 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.016 decoded-end-sample=45600 presentation-sample=2400 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=2400
audio frame pipeline: frame-ms=0.010 process-ms=0.001 analyze-ms=0.009
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=66334 peak-pixel=51 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.187 palette-ms=0.003 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.176 copy-ms=0.000 flush-ms=0.007 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.791 palette-ms=0.004 compose-ms=0.011 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.579 post-ms=0.192 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.794 previous-ms=24.714 alpha=0.100 estimate-ms=22.622
frame timing: total-ms=10.716 next-frame=0.005 audio=0.154 bridge=0.016 buffer=6.745 display=3.794 do-display=1
frame pacing: pacing target-ms=33.161 requested-sleep-ms=22.442 actual-pacing-ms=22.806 maxfps=25
display timing: mainloop-ms=33.599 events-ms=0.064 pre-interface-ms=0.000 frame-ms=10.718 post-interface-ms=0.001 pacing-ms=22.806 events=3 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31788.835721 sampled-now=31788.869281 raw-delta-ms=33.559
frame pacing: updateFPS deltaT-ms=33.559 fps=29.798 rolling-fps=16.626
audio frame builder: built frame center-sample=4800 start-sample=4288 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.012 decoded-end-sample=45600 presentation-sample=4800 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=4800
audio frame pipeline: frame-ms=0.008 process-ms=0.001 analyze-ms=0.007
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=26400 presentation-delay-samples=21600 underflows=0
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=65821 peak-pixel=49 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.236 palette-ms=0.004 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.225 copy-ms=0.001 flush-ms=0.005 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.799 palette-ms=0.005 compose-ms=0.014 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.532 post-ms=0.244 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.880 previous-ms=22.622 alpha=0.100 estimate-ms=20.748
frame timing: total-ms=9.606 next-frame=0.005 audio=0.014 bridge=0.012 buffer=5.693 display=3.880 do-display=1
frame pacing: pacing target-ms=39.602 requested-sleep-ms=29.991 actual-pacing-ms=30.312 maxfps=25
display timing: mainloop-ms=39.955 events-ms=0.017 pre-interface-ms=0.002 frame-ms=9.609 post-interface-ms=0.002 pacing-ms=30.312 events=0 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31788.869281 sampled-now=31788.909252 raw-delta-ms=39.972
frame pacing: updateFPS deltaT-ms=39.972 fps=25.018 rolling-fps=17.354
audio frame builder: built frame center-sample=4800 start-sample=4288 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.015 decoded-end-sample=48000 presentation-sample=4800 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=4800
audio frame pipeline: frame-ms=0.010 process-ms=0.001 analyze-ms=0.009
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=28800 presentation-delay-samples=21600 underflows=0
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=65157 peak-pixel=48 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.196 palette-ms=0.003 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.185 copy-ms=0.001 flush-ms=0.007 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.977 palette-ms=0.005 compose-ms=0.012 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.752 post-ms=0.204 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.981 previous-ms=20.748 alpha=0.100 estimate-ms=19.071
frame timing: total-ms=10.582 next-frame=0.005 audio=0.017 bridge=0.015 buffer=6.562 display=3.981 do-display=1
frame pacing: pacing target-ms=39.630 requested-sleep-ms=29.044 actual-pacing-ms=29.423 maxfps=25
display timing: mainloop-ms=40.052 events-ms=0.028 pre-interface-ms=0.003 frame-ms=10.585 post-interface-ms=0.001 pacing-ms=29.423 events=0 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31788.909252 sampled-now=31788.949315 raw-delta-ms=40.063
frame pacing: updateFPS deltaT-ms=40.063 fps=24.960 rolling-fps=17.962
audio frame builder: built frame center-sample=7200 start-sample=6688 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.014 decoded-end-sample=50400 presentation-sample=7200 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=7200
audio frame pipeline: frame-ms=0.010 process-ms=0.001 analyze-ms=0.009
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=64488 peak-pixel=47 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=31200 presentation-delay-samples=21600 underflows=0
display timing: post-draw-ms=46.329 palette-ms=0.003 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=46.312 copy-ms=0.002 flush-ms=0.009 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=50.039 palette-ms=0.005 compose-ms=0.012 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.673 post-ms=46.345 size=640x480 draw=640x480
display timing: visual-latency observed-ms=50.045 previous-ms=19.071 alpha=0.100 estimate-ms=22.168
frame timing: total-ms=55.979 next-frame=0.006 audio=0.016 bridge=0.016 buffer=5.891 display=50.046 do-display=1
frame pacing: pacing target-ms=39.567 requested-sleep-ms=0.000 actual-pacing-ms=0.001 maxfps=25
display timing: mainloop-ms=56.028 events-ms=0.032 pre-interface-ms=0.003 frame-ms=55.983 post-interface-ms=0.004 pacing-ms=0.001 events=0 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31788.949315 sampled-now=31789.005331 raw-delta-ms=56.016
frame pacing: updateFPS deltaT-ms=56.016 fps=17.852 rolling-fps=17.951
audio frame builder: built frame center-sample=9600 start-sample=9088 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.014 decoded-end-sample=52800 presentation-sample=9600 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=9600
audio frame pipeline: frame-ms=0.010 process-ms=0.002 analyze-ms=0.008
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=63618 peak-pixel=45 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.297 palette-ms=0.003 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.282 copy-ms=0.001 flush-ms=0.008 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=4.122 palette-ms=0.005 compose-ms=0.019 prepare-ms=0.001 viewport-ms=0.003 overlay-ms=0.006 transfer-clear-ms=3.783 post-ms=0.306 size=640x480 draw=640x480
display timing: visual-latency observed-ms=4.128 previous-ms=22.168 alpha=0.100 estimate-ms=20.364
frame timing: total-ms=12.476 next-frame=0.004 audio=0.017 bridge=0.016 buffer=8.309 display=4.128 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=33600 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=23.551 requested-sleep-ms=11.067 actual-pacing-ms=11.449 maxfps=25
display timing: mainloop-ms=23.961 events-ms=0.021 pre-interface-ms=0.000 frame-ms=12.480 post-interface-ms=0.003 pacing-ms=11.449 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.005331 sampled-now=31789.029309 raw-delta-ms=23.978
frame pacing: updateFPS deltaT-ms=23.978 fps=41.705 rolling-fps=18.931
audio frame builder: built frame center-sample=12000 start-sample=11488 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.013 decoded-end-sample=55200 presentation-sample=12000 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=12000
audio frame pipeline: frame-ms=0.009 process-ms=0.001 analyze-ms=0.008
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=62647 peak-pixel=44 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.303 palette-ms=0.004 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.286 copy-ms=0.001 flush-ms=0.009 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.893 palette-ms=0.004 compose-ms=0.011 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.564 post-ms=0.310 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.897 previous-ms=20.364 alpha=0.100 estimate-ms=18.718
frame timing: total-ms=10.017 next-frame=0.097 audio=0.015 bridge=0.013 buffer=5.992 display=3.897 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=36000 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.573 requested-sleep-ms=29.551 actual-pacing-ms=29.872 maxfps=25
display timing: mainloop-ms=39.935 events-ms=0.032 pre-interface-ms=0.001 frame-ms=10.021 post-interface-ms=0.001 pacing-ms=29.872 events=5 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.029309 sampled-now=31789.069245 raw-delta-ms=39.935
frame pacing: updateFPS deltaT-ms=39.935 fps=25.040 rolling-fps=19.324
audio frame builder: built frame center-sample=14400 start-sample=13888 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.017 decoded-end-sample=57600 presentation-sample=14400 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=14400
audio frame pipeline: frame-ms=0.009 process-ms=0.001 analyze-ms=0.008
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=61641 peak-pixel=128 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.255 palette-ms=0.004 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.245 copy-ms=0.000 flush-ms=0.004 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=4.442 palette-ms=0.004 compose-ms=0.012 prepare-ms=0.000 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=4.060 post-ms=0.362 size=640x480 draw=640x480
display timing: visual-latency observed-ms=4.446 previous-ms=18.718 alpha=0.100 estimate-ms=17.290
frame timing: total-ms=11.543 next-frame=0.005 audio=0.019 bridge=0.014 buffer=7.057 display=4.446 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=38400 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.638 requested-sleep-ms=28.091 actual-pacing-ms=28.339 maxfps=25
display timing: mainloop-ms=39.923 events-ms=0.029 pre-interface-ms=0.001 frame-ms=11.545 post-interface-ms=0.001 pacing-ms=28.339 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.069245 sampled-now=31789.109175 raw-delta-ms=39.930
frame pacing: updateFPS deltaT-ms=39.930 fps=25.044 rolling-fps=19.670
audio frame builder: built frame center-sample=16800 start-sample=16288 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.015 decoded-end-sample=60000 presentation-sample=16800 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=16800
audio frame pipeline: frame-ms=0.010 process-ms=0.001 analyze-ms=0.008
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=60188 peak-pixel=128 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.307 palette-ms=0.004 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.296 copy-ms=0.001 flush-ms=0.005 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=4.405 palette-ms=0.005 compose-ms=0.013 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=4.067 post-ms=0.314 size=640x480 draw=640x480
display timing: visual-latency observed-ms=4.409 previous-ms=17.290 alpha=0.100 estimate-ms=16.002
frame timing: total-ms=11.406 next-frame=0.005 audio=0.017 bridge=0.014 buffer=6.959 display=4.409 do-display=1
frame pacing: pacing target-ms=39.707 requested-sleep-ms=28.297 actual-pacing-ms=28.676 maxfps=25
display timing: mainloop-ms=40.129 events-ms=0.031 pre-interface-ms=0.002 frame-ms=11.409 post-interface-ms=0.001 pacing-ms=28.676 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.109175 sampled-now=31789.149309 raw-delta-ms=40.135
frame pacing: updateFPS deltaT-ms=40.135 fps=24.916 rolling-fps=19.970
audio frame builder: built frame center-sample=16800 start-sample=16288 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.018 decoded-end-sample=60000 presentation-sample=16800 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio analysis: amplitude=0 left=0 right=0 noisy=0 frame-samples=1024 center-sample=16800
audio frame pipeline: frame-ms=0.011 process-ms=0.001 analyze-ms=0.009
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=57983 peak-pixel=128 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.313 palette-ms=0.004 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.299 copy-ms=0.001 flush-ms=0.007 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=4.864 palette-ms=0.005 compose-ms=0.014 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.004 transfer-clear-ms=4.520 post-ms=0.319 size=640x480 draw=640x480
display timing: visual-latency observed-ms=4.868 previous-ms=16.002 alpha=0.100 estimate-ms=14.889
frame timing: total-ms=12.949 next-frame=0.006 audio=0.021 bridge=0.015 buffer=8.036 display=4.868 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=40800 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.573 requested-sleep-ms=26.619 actual-pacing-ms=26.997 maxfps=25
display timing: mainloop-ms=39.992 events-ms=0.031 pre-interface-ms=0.002 frame-ms=12.952 post-interface-ms=0.001 pacing-ms=26.997 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.149309 sampled-now=31789.189307 raw-delta-ms=39.997
frame pacing: updateFPS deltaT-ms=39.997 fps=25.002 rolling-fps=20.242
audio frame builder: built frame center-sample=19200 start-sample=18688 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.021 decoded-end-sample=62400 presentation-sample=19200 input-finished=0 complete=0
audio processing: processing mode=`none'
processed wave audio: mode=none amplitude=0 left=0 right=0 noisy=0
audio frame pipeline: frame-ms=0.011 process-ms=0.002 analyze-ms=0.009
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
visual buffer: wave=Spike wave-scale=scale1 flame=l-Su table=table8 nonzero-pixels=54494 peak-pixel=128 size=76800
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.255 palette-ms=0.004 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.246 copy-ms=0.001 flush-ms=0.003 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=4.484 palette-ms=0.006 compose-ms=0.015 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.004 transfer-clear-ms=4.196 post-ms=0.260 size=640x480 draw=640x480
display timing: visual-latency observed-ms=4.488 previous-ms=14.889 alpha=0.100 estimate-ms=13.849
frame timing: total-ms=14.172 next-frame=0.006 audio=0.030 bridge=0.018 buffer=9.628 display=4.488 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=43200 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.575 requested-sleep-ms=25.399 actual-pacing-ms=25.729 maxfps=25
display timing: mainloop-ms=39.947 events-ms=0.030 pre-interface-ms=0.002 frame-ms=14.176 post-interface-ms=0.001 pacing-ms=25.729 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.189307 sampled-now=31789.229364 raw-delta-ms=40.057
frame pacing: updateFPS deltaT-ms=40.057 fps=24.964 rolling-fps=20.484
audio frame builder: built frame center-sample=21600 start-sample=21088 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.017 decoded-end-sample=64800 presentation-sample=21600 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.006 process-ms=0.001 analyze-ms=0.004
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.216 palette-ms=0.003 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.206 copy-ms=0.000 flush-ms=0.003 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.472 palette-ms=0.005 compose-ms=0.014 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.004 transfer-clear-ms=3.227 post-ms=0.221 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.475 previous-ms=13.849 alpha=0.100 estimate-ms=12.811
frame timing: total-ms=9.602 next-frame=0.005 audio=0.019 bridge=0.010 buffer=6.090 display=3.475 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=45600 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.518 requested-sleep-ms=29.914 actual-pacing-ms=30.234 maxfps=25
display timing: mainloop-ms=39.895 events-ms=0.027 pre-interface-ms=0.002 frame-ms=9.604 post-interface-ms=0.001 pacing-ms=30.234 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.229364 sampled-now=31789.269294 raw-delta-ms=39.930
frame pacing: updateFPS deltaT-ms=39.930 fps=25.044 rolling-fps=20.706
audio frame builder: built frame center-sample=24000 start-sample=23488 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.016 decoded-end-sample=67200 presentation-sample=24000 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.008 process-ms=0.001 analyze-ms=0.007
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.319 palette-ms=0.004 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.307 copy-ms=0.001 flush-ms=0.004 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=4.697 palette-ms=0.004 compose-ms=0.010 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.002 transfer-clear-ms=4.354 post-ms=0.326 size=640x480 draw=640x480
display timing: visual-latency observed-ms=4.701 previous-ms=12.811 alpha=0.100 estimate-ms=12.000
frame timing: total-ms=15.251 next-frame=0.005 audio=0.020 bridge=0.018 buffer=10.504 display=4.702 do-display=1
frame pacing: pacing target-ms=39.588 requested-sleep-ms=24.222 actual-pacing-ms=24.544 maxfps=25
display timing: mainloop-ms=39.980 events-ms=0.057 pre-interface-ms=0.002 frame-ms=15.365 post-interface-ms=0.001 pacing-ms=24.544 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.269294 sampled-now=31789.309259 raw-delta-ms=39.965
frame pacing: updateFPS deltaT-ms=39.965 fps=25.022 rolling-fps=20.906
audio frame builder: built frame center-sample=24000 start-sample=23488 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.021 decoded-end-sample=67200 presentation-sample=24000 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.005 process-ms=0.001 analyze-ms=0.004
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=48000 presentation-delay-samples=21600 underflows=0
display timing: post-draw-ms=0.286 palette-ms=0.004 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.274 copy-ms=0.001 flush-ms=0.005 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=4.663 palette-ms=0.005 compose-ms=0.014 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.004 transfer-clear-ms=4.347 post-ms=0.292 size=640x480 draw=640x480
display timing: visual-latency observed-ms=4.667 previous-ms=12.000 alpha=0.100 estimate-ms=11.267
frame timing: total-ms=11.986 next-frame=0.006 audio=0.024 bridge=0.011 buffer=7.275 display=4.668 do-display=1
frame pacing: pacing target-ms=39.623 requested-sleep-ms=27.632 actual-pacing-ms=27.994 maxfps=25
display timing: mainloop-ms=40.034 events-ms=0.037 pre-interface-ms=0.002 frame-ms=11.989 post-interface-ms=0.001 pacing-ms=27.994 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.309259 sampled-now=31789.349298 raw-delta-ms=40.039
frame pacing: updateFPS deltaT-ms=40.039 fps=24.976 rolling-fps=21.087
audio frame builder: built frame center-sample=26400 start-sample=25888 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.018 decoded-end-sample=69600 presentation-sample=26400 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.006 process-ms=0.002 analyze-ms=0.005
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=50400 presentation-delay-samples=21600 underflows=0
display timing: post-draw-ms=0.320 palette-ms=0.005 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.308 copy-ms=0.001 flush-ms=0.004 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=4.604 palette-ms=0.007 compose-ms=0.025 prepare-ms=0.001 viewport-ms=0.003 overlay-ms=0.005 transfer-clear-ms=4.137 post-ms=0.426 size=640x480 draw=640x480
display timing: visual-latency observed-ms=4.609 previous-ms=11.267 alpha=0.100 estimate-ms=10.601
frame timing: total-ms=14.399 next-frame=0.006 audio=0.020 bridge=0.012 buffer=9.749 display=4.609 do-display=1
frame pacing: pacing target-ms=39.584 requested-sleep-ms=25.182 actual-pacing-ms=25.495 maxfps=25
display timing: mainloop-ms=39.948 events-ms=0.036 pre-interface-ms=0.002 frame-ms=14.402 post-interface-ms=0.001 pacing-ms=25.495 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.349298 sampled-now=31789.389251 raw-delta-ms=39.953
frame pacing: updateFPS deltaT-ms=39.953 fps=25.030 rolling-fps=21.254
audio frame builder: built frame center-sample=28800 start-sample=28288 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.016 decoded-end-sample=72000 presentation-sample=28800 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.005 process-ms=0.001 analyze-ms=0.004
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.398 palette-ms=0.004 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.384 copy-ms=0.001 flush-ms=0.006 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=4.527 palette-ms=0.004 compose-ms=0.012 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.004 transfer-clear-ms=4.098 post-ms=0.407 size=640x480 draw=640x480
display timing: visual-latency observed-ms=4.531 previous-ms=10.601 alpha=0.100 estimate-ms=9.994
frame timing: total-ms=10.971 next-frame=0.005 audio=0.018 bridge=0.009 buffer=6.404 display=4.532 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=52800 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.631 requested-sleep-ms=28.655 actual-pacing-ms=28.941 maxfps=25
display timing: mainloop-ms=39.965 events-ms=0.035 pre-interface-ms=0.002 frame-ms=10.974 post-interface-ms=0.002 pacing-ms=28.941 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.389251 sampled-now=31789.429226 raw-delta-ms=39.975
frame pacing: updateFPS deltaT-ms=39.975 fps=25.016 rolling-fps=21.407
audio frame builder: built frame center-sample=31200 start-sample=30688 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.015 decoded-end-sample=74400 presentation-sample=31200 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.006 process-ms=0.001 analyze-ms=0.004
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.521 palette-ms=0.007 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.498 copy-ms=0.001 flush-ms=0.008 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=4.357 palette-ms=0.004 compose-ms=0.012 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.004 transfer-clear-ms=3.802 post-ms=0.533 size=640x480 draw=640x480
display timing: visual-latency observed-ms=4.362 previous-ms=9.994 alpha=0.100 estimate-ms=9.431
frame timing: total-ms=12.682 next-frame=0.006 audio=0.018 bridge=0.011 buffer=8.282 display=4.362 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=55200 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.656 requested-sleep-ms=26.967 actual-pacing-ms=27.329 maxfps=25
display timing: mainloop-ms=40.067 events-ms=0.040 pre-interface-ms=0.002 frame-ms=12.686 post-interface-ms=0.002 pacing-ms=27.329 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.429226 sampled-now=31789.469289 raw-delta-ms=40.063
frame pacing: updateFPS deltaT-ms=40.063 fps=24.961 rolling-fps=21.547
audio frame builder: built frame center-sample=33600 start-sample=33088 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.013 decoded-end-sample=76800 presentation-sample=33600 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.005 process-ms=0.001 analyze-ms=0.004
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.244 palette-ms=0.003 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.235 copy-ms=0.001 flush-ms=0.004 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.906 palette-ms=0.004 compose-ms=0.010 prepare-ms=0.001 viewport-ms=0.000 overlay-ms=0.003 transfer-clear-ms=3.641 post-ms=0.248 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.909 previous-ms=9.431 alpha=0.100 estimate-ms=8.879
frame timing: total-ms=9.435 next-frame=0.004 audio=0.015 bridge=0.009 buffer=5.496 display=3.910 do-display=1
frame pacing: pacing target-ms=39.593 requested-sleep-ms=30.155 actual-pacing-ms=30.522 maxfps=25
display timing: mainloop-ms=40.004 events-ms=0.031 pre-interface-ms=0.002 frame-ms=9.438 post-interface-ms=0.000 pacing-ms=30.522 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.469289 sampled-now=31789.509300 raw-delta-ms=40.011
frame pacing: updateFPS deltaT-ms=40.011 fps=24.993 rolling-fps=21.677
audio frame builder: built frame center-sample=36000 start-sample=35488 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.014 decoded-end-sample=76800 presentation-sample=36000 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.005 process-ms=0.002 analyze-ms=0.003
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=57600 presentation-delay-samples=21600 underflows=0
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.242 palette-ms=0.003 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.232 copy-ms=0.001 flush-ms=0.005 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.987 palette-ms=0.004 compose-ms=0.009 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.722 post-ms=0.248 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.990 previous-ms=8.879 alpha=0.100 estimate-ms=8.390
frame timing: total-ms=9.700 next-frame=0.005 audio=0.016 bridge=0.009 buffer=5.678 display=3.991 do-display=1
frame pacing: pacing target-ms=39.582 requested-sleep-ms=29.879 actual-pacing-ms=30.314 maxfps=25
display timing: mainloop-ms=40.068 events-ms=0.032 pre-interface-ms=0.002 frame-ms=9.702 post-interface-ms=0.001 pacing-ms=30.314 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.509300 sampled-now=31789.549409 raw-delta-ms=40.110
frame pacing: updateFPS deltaT-ms=40.110 fps=24.932 rolling-fps=21.795
audio frame builder: built frame center-sample=36000 start-sample=35488 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.020 decoded-end-sample=79200 presentation-sample=36000 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.010 process-ms=0.002 analyze-ms=0.009
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=60000 presentation-delay-samples=21600 underflows=0
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.242 palette-ms=0.004 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.230 copy-ms=0.001 flush-ms=0.005 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.829 palette-ms=0.004 compose-ms=0.012 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.005 transfer-clear-ms=3.557 post-ms=0.249 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.834 previous-ms=8.390 alpha=0.100 estimate-ms=7.934
frame timing: total-ms=15.497 next-frame=0.006 audio=0.023 bridge=0.016 buffer=11.616 display=3.834 do-display=1
frame pacing: pacing target-ms=39.473 requested-sleep-ms=23.970 actual-pacing-ms=24.556 maxfps=25
display timing: mainloop-ms=40.159 events-ms=0.065 pre-interface-ms=0.002 frame-ms=15.501 post-interface-ms=0.001 pacing-ms=24.556 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.549409 sampled-now=31789.589783 raw-delta-ms=40.373
frame pacing: updateFPS deltaT-ms=40.373 fps=24.769 rolling-fps=21.901
audio frame builder: built frame center-sample=38400 start-sample=37888 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.021 decoded-end-sample=81600 presentation-sample=38400 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.012 process-ms=0.005 analyze-ms=0.008
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.349 palette-ms=0.004 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.335 copy-ms=0.001 flush-ms=0.007 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=4.113 palette-ms=0.004 compose-ms=0.010 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.735 post-ms=0.358 size=640x480 draw=640x480
display timing: visual-latency observed-ms=4.116 previous-ms=7.934 alpha=0.100 estimate-ms=7.553
frame timing: total-ms=12.845 next-frame=0.009 audio=0.205 bridge=0.019 buffer=8.494 display=4.117 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=62400 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.100 requested-sleep-ms=26.248 actual-pacing-ms=26.584 maxfps=25
display timing: mainloop-ms=39.709 events-ms=0.257 pre-interface-ms=0.003 frame-ms=12.850 post-interface-ms=0.002 pacing-ms=26.584 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.589783 sampled-now=31789.629300 raw-delta-ms=39.518
frame pacing: updateFPS deltaT-ms=39.518 fps=25.305 rolling-fps=22.014
audio frame builder: built frame center-sample=40800 start-sample=40288 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.022 decoded-end-sample=84000 presentation-sample=40800 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.009 process-ms=0.002 analyze-ms=0.007
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.324 palette-ms=0.003 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.302 copy-ms=0.001 flush-ms=0.016 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=4.196 palette-ms=0.004 compose-ms=0.010 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.846 post-ms=0.331 size=640x480 draw=640x480
display timing: visual-latency observed-ms=4.200 previous-ms=7.553 alpha=0.100 estimate-ms=7.217
frame timing: total-ms=14.330 next-frame=0.006 audio=0.025 bridge=0.015 buffer=10.082 display=4.200 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=64800 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.582 requested-sleep-ms=25.246 actual-pacing-ms=25.628 maxfps=25
display timing: mainloop-ms=40.040 events-ms=0.058 pre-interface-ms=0.003 frame-ms=14.335 post-interface-ms=0.002 pacing-ms=25.628 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.629300 sampled-now=31789.669420 raw-delta-ms=40.120
frame pacing: updateFPS deltaT-ms=40.120 fps=24.925 rolling-fps=22.110
audio frame builder: built frame center-sample=43200 start-sample=42688 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.014 decoded-end-sample=86400 presentation-sample=43200 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.005 process-ms=0.002 analyze-ms=0.004
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.279 palette-ms=0.003 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.268 copy-ms=0.001 flush-ms=0.006 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.723 palette-ms=0.004 compose-ms=0.011 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.419 post-ms=0.285 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.726 previous-ms=7.217 alpha=0.100 estimate-ms=6.868
frame timing: total-ms=9.040 next-frame=0.006 audio=0.016 bridge=0.012 buffer=5.279 display=3.727 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=67200 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.462 requested-sleep-ms=30.418 actual-pacing-ms=30.707 maxfps=25
display timing: mainloop-ms=39.808 events-ms=0.038 pre-interface-ms=0.003 frame-ms=9.043 post-interface-ms=0.001 pacing-ms=30.707 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.669420 sampled-now=31789.709267 raw-delta-ms=39.847
frame pacing: updateFPS deltaT-ms=39.847 fps=25.096 rolling-fps=22.204
audio frame builder: built frame center-sample=45600 start-sample=45088 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.021 decoded-end-sample=88800 presentation-sample=45600 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.012 process-ms=0.002 analyze-ms=0.009
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.277 palette-ms=0.003 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.266 copy-ms=0.001 flush-ms=0.005 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.562 palette-ms=0.004 compose-ms=0.011 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.004 transfer-clear-ms=3.258 post-ms=0.284 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.567 previous-ms=6.868 alpha=0.100 estimate-ms=6.538
frame timing: total-ms=14.491 next-frame=0.007 audio=0.024 bridge=0.018 buffer=10.873 display=3.567 do-display=1
frame pacing: pacing target-ms=39.615 requested-sleep-ms=25.045 actual-pacing-ms=25.462 maxfps=25
display timing: mainloop-ms=40.123 events-ms=0.070 pre-interface-ms=0.003 frame-ms=14.568 post-interface-ms=0.002 pacing-ms=25.462 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.709267 sampled-now=31789.749416 raw-delta-ms=40.148
frame pacing: updateFPS deltaT-ms=40.148 fps=24.908 rolling-fps=22.288
audio frame builder: built frame center-sample=45600 start-sample=45088 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.021 decoded-end-sample=88800 presentation-sample=45600 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.010 process-ms=0.002 analyze-ms=0.008
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.329 palette-ms=0.003 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.315 copy-ms=0.001 flush-ms=0.007 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.958 palette-ms=0.004 compose-ms=0.011 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.004 transfer-clear-ms=3.602 post-ms=0.335 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.962 previous-ms=6.538 alpha=0.100 estimate-ms=6.280
frame timing: total-ms=12.173 next-frame=0.006 audio=0.024 bridge=0.017 buffer=8.162 display=3.962 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=69600 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.466 requested-sleep-ms=27.288 actual-pacing-ms=27.681 maxfps=25
display timing: mainloop-ms=39.975 events-ms=0.086 pre-interface-ms=0.006 frame-ms=12.177 post-interface-ms=0.001 pacing-ms=27.681 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.749416 sampled-now=31789.789371 raw-delta-ms=39.955
frame pacing: updateFPS deltaT-ms=39.955 fps=25.028 rolling-fps=22.369
audio frame builder: built frame center-sample=48000 start-sample=47488 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.021 decoded-end-sample=91200 presentation-sample=48000 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.009 process-ms=0.002 analyze-ms=0.007
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.282 palette-ms=0.003 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.271 copy-ms=0.001 flush-ms=0.005 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.754 palette-ms=0.004 compose-ms=0.010 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.004 transfer-clear-ms=3.445 post-ms=0.290 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.836 previous-ms=6.280 alpha=0.100 estimate-ms=6.036
frame timing: total-ms=12.269 next-frame=0.006 audio=0.023 bridge=0.019 buffer=8.383 display=3.836 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=72000 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.511 requested-sleep-ms=27.236 actual-pacing-ms=27.581 maxfps=25
display timing: mainloop-ms=39.927 events-ms=0.059 pre-interface-ms=0.002 frame-ms=12.272 post-interface-ms=0.002 pacing-ms=27.581 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.789371 sampled-now=31789.829287 raw-delta-ms=39.916
frame pacing: updateFPS deltaT-ms=39.916 fps=25.052 rolling-fps=22.374
audio frame builder: built frame center-sample=50400 start-sample=49888 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.013 decoded-end-sample=93600 presentation-sample=50400 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.004 process-ms=0.001 analyze-ms=0.003
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.206 palette-ms=0.003 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.198 copy-ms=0.001 flush-ms=0.003 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.464 palette-ms=0.003 compose-ms=0.009 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.002 transfer-clear-ms=3.236 post-ms=0.213 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.467 previous-ms=6.036 alpha=0.100 estimate-ms=5.779
frame timing: total-ms=8.431 next-frame=0.005 audio=0.015 bridge=0.010 buffer=4.934 display=3.467 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=74400 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.595 requested-sleep-ms=31.160 actual-pacing-ms=31.274 maxfps=25
display timing: mainloop-ms=39.765 events-ms=0.042 pre-interface-ms=0.003 frame-ms=8.434 post-interface-ms=0.001 pacing-ms=31.274 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.829287 sampled-now=31789.869046 raw-delta-ms=39.759
frame pacing: updateFPS deltaT-ms=39.759 fps=25.152 rolling-fps=22.378
audio frame builder: built frame center-sample=52800 start-sample=52288 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.012 decoded-end-sample=96000 presentation-sample=52800 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.005 process-ms=0.001 analyze-ms=0.003
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.258 palette-ms=0.003 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.248 copy-ms=0.001 flush-ms=0.004 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.445 palette-ms=0.003 compose-ms=0.011 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.162 post-ms=0.264 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.448 previous-ms=5.779 alpha=0.100 estimate-ms=5.546
frame timing: total-ms=8.213 next-frame=0.005 audio=0.013 bridge=0.010 buffer=4.735 display=3.448 do-display=1
frame pacing: pacing target-ms=39.836 requested-sleep-ms=31.620 actual-pacing-ms=31.925 maxfps=25
display timing: mainloop-ms=40.188 events-ms=0.034 pre-interface-ms=0.002 frame-ms=8.216 post-interface-ms=0.001 pacing-ms=31.925 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.869046 sampled-now=31789.909244 raw-delta-ms=40.198
frame pacing: updateFPS deltaT-ms=40.198 fps=24.877 rolling-fps=22.376
audio frame builder: built frame center-sample=52800 start-sample=52288 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.013 decoded-end-sample=96000 presentation-sample=52800 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.004 process-ms=0.001 analyze-ms=0.003
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=76800 presentation-delay-samples=21600 underflows=0
display timing: post-draw-ms=0.227 palette-ms=0.003 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.216 copy-ms=0.001 flush-ms=0.005 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.361 palette-ms=0.003 compose-ms=0.009 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.111 post-ms=0.233 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.364 previous-ms=5.546 alpha=0.100 estimate-ms=5.328
frame timing: total-ms=7.865 next-frame=0.005 audio=0.015 bridge=0.009 buffer=4.470 display=3.364 do-display=1
frame pacing: pacing target-ms=39.638 requested-sleep-ms=31.770 actual-pacing-ms=32.014 maxfps=25
display timing: mainloop-ms=39.931 events-ms=0.037 pre-interface-ms=0.003 frame-ms=7.868 post-interface-ms=0.001 pacing-ms=32.014 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.909244 sampled-now=31789.949172 raw-delta-ms=39.928
frame pacing: updateFPS deltaT-ms=39.928 fps=25.045 rolling-fps=24.994
audio frame builder: built frame center-sample=55200 start-sample=54688 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.012 decoded-end-sample=98400 presentation-sample=55200 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.004 process-ms=0.001 analyze-ms=0.003
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
display timing: post-draw-ms=0.192 palette-ms=0.003 dump-ms=0.001 preview-ms=0.000 label-ms=0.000 put-ms=0.183 copy-ms=0.001 flush-ms=0.004 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.539 palette-ms=0.003 compose-ms=0.009 prepare-ms=0.000 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.325 post-ms=0.198 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.542 previous-ms=5.328 alpha=0.100 estimate-ms=5.149
frame timing: total-ms=8.104 next-frame=0.004 audio=0.014 bridge=0.009 buffer=4.533 display=3.542 do-display=1
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=79200 presentation-delay-samples=21600 underflows=0
frame pacing: pacing target-ms=39.710 requested-sleep-ms=31.603 actual-pacing-ms=32.017 maxfps=25
display timing: mainloop-ms=40.173 events-ms=0.031 pre-interface-ms=0.002 frame-ms=8.106 post-interface-ms=0.001 pacing-ms=32.017 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.949172 sampled-now=31789.989387 raw-delta-ms=40.214
frame pacing: updateFPS deltaT-ms=40.214 fps=24.867 rolling-fps=25.585
audio frame builder: built frame center-sample=57600 start-sample=57088 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.020 decoded-end-sample=100800 presentation-sample=57600 input-finished=0 complete=0
audio processing: processing mode=`none'
audio frame pipeline: frame-ms=0.009 process-ms=0.002 analyze-ms=0.008
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: default stage sequence stages=10
frame generator: running filterchain=0xaaaabfcf67a0 filters=10
frame filterchain: skipping disabled stage=0 filter=0xaaaabf063000
frame filterchain: executing border stage mode=3
frame filterchain: executing flame stage
frame filterchain: executing translate stage
frame filterchain: executing wave stage
frame filterchain: skipping disabled stage=6 filter=0xaaaabf0625d0
frame filterchain: committing indexed buffer frame
frame filterchain: executing palette stage
frame filterchain: skipping disabled stage=1 filter=0xaaaabf034860
frame filterchain: publishing indexed frame
audio runtime: miniaudio callback state active=1 drained=0 queued-samples=21600 submitted-end-sample=81600 presentation-delay-samples=21600 underflows=0
display timing: post-draw-ms=0.278 palette-ms=0.003 dump-ms=0.000 preview-ms=0.000 label-ms=0.000 put-ms=0.268 copy-ms=0.001 flush-ms=0.005 copy-text=0 full-copy=0 shm-level=0 draw=640x480
display timing: x11 frame-ms=3.877 palette-ms=0.003 compose-ms=0.009 prepare-ms=0.001 viewport-ms=0.001 overlay-ms=0.003 transfer-clear-ms=3.576 post-ms=0.284 size=640x480 draw=640x480
display timing: visual-latency observed-ms=3.880 previous-ms=5.149 alpha=0.100 estimate-ms=5.022
frame timing: total-ms=12.185 next-frame=0.006 audio=0.023 bridge=0.015 buffer=8.259 display=3.880 do-display=1
frame pacing: pacing target-ms=39.495 requested-sleep-ms=27.306 actual-pacing-ms=27.717 maxfps=25
display timing: mainloop-ms=39.992 events-ms=0.066 pre-interface-ms=0.002 frame-ms=12.188 post-interface-ms=0.001 pacing-ms=27.717 events=1 resize-events=0 expose-events=0
frame pacing: nextFrame previous-now=31789.989387 sampled-now=31790.029390 raw-delta-ms=40.003
frame pacing: updateFPS deltaT-ms=40.003 fps=24.998 rolling-fps=25.133
audio frame builder: built frame center-sample=60000 start-sample=59488 samples=1024 raw-bytes=4096
audio timing: audio-ingest tick-ms=0.039 decoded-end-sample=103200 presentation-sample=60000 input-finished=0 complete=0
audio processing: processing mode=`none'
