# Synth One-only native timing fixture

Apply the scratch-only `workload.patch` in a disposable worktree based on
873ec0a. Do not build it for installation. It uses normal audio and MIDI
dispatch, with a fixed generated-sample note schedule. The fixture loads
only Synth One on A, retires the already silent B part, and makes no use of
the other synth renderer, transport, or physical key tape.

At generated sample 65,536, while silent, the fixture enables Delay and
Room and explicitly reinitializes the delay buffer allocation **for this
fixture** before playing notes. This is not a change to live controls.
Simply changing the delay time after its first allocation does not
establish the correct buffer geometry for this test. At 98,304, four
Synth One notes reach the normal MIDI acceptance path; they release at
163,840. The live voice count, exact eight events, peer retirement and
actual decimation are validated from RAM. The 128 held halves are timed
separately. The observer distinguishes the 10,809 us target, the physical
512-frame budget, and the normal firmware DMA-index error counter.

Build with the project Pi32 toolchain and SDK:

```sh
MODEL=elite X0X_UI_SMALL_CANVAS=1 X0X_BASS_PRESET_PREVIEW=1 \
  X0X_BASS_TAGGED_SOUND=1 X0X_BASS_NOTE_SOUND=0 X0X_D9_TANH_BOOT=1 \
  X0X_MIAMI_N513_BOOT=1 X0X_CHI_FACTORY_COL=1 X0X_SUPERJ6_FACTORY=0 \
  X0X_SYNTHONE_PATCH=1 X0X_S1C=1 ./build.sh
```

Use the pinned emulator and matching approved staging controller, if
needed. Preserve the exact ELF as `bench.elf` in a disposable results
directory with a `work/` subdirectory. Do not supply a physical key tape,
MIDI packets, or a device destination:

```sh
FM1_STRICT=1 FM1_NO_SKIP=1 FM1_DUMPRAM="$PWD/ram.bin" \
  "$FM1_EMULATOR" --fwsc "$EMULATOR_PACKAGE" --mhz 360 --usb --shot 8000 \
  --flash "$PWD/virtual-nor.bin" >run.log 2>&1
python3 "$SOURCE/experiments/s1-only-timing/readback.py" "$PWD"
```

The initial corrected Elite baseline had delay stride 38, peak
16,791.79 us, 128 held halves over the target and 147 DMA XRUN indications.
A valid fixture is not a performance pass. Do not use this single
four-voice test to qualify all programs, the full stress scene or a release.
