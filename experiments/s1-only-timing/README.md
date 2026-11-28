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

## Independent experiments (none enabled by default)

- Apply `overlap64.patch` after the workload patch on the candidate only.
  The 64-frame ping-pong tape lets Core 1 process effects from the prior
  tile while Core 0 renders voices for the next tile. Only in this
  disposable workload does the retired peer stop reserving that worker.
  The implementation does not replay a partially completed job on timeout.
  Do not use it as a production scheduler: recovery/ownership under fault,
  other playback workloads and both board variants remain unqualified.
- Apply `render-fingerprints.patch` to **separate correctness builds** of
  both arms. Compare the generated words before DMA using
  `python3 experiments/s1-only-timing/compare-render.py BASE CANDIDATE`.
  Do not measure timing in the hashing runs. The corrected four-voice
  reference comparison matched all 160 fingerprints (128 held, 32 release).
- The independent delay iteration and reverb reader changes live in
  `delay-tick.patch` and `room-read.patch`. The latter needs
  `DNBOX_S1_ROOM_REBASE=1` on s1u.c. Test each alone, on the original
  renderer, before testing combinations. Do not apply the delay iteration
  patch on top of the overlap patch as-is.
- `delay-allocation.patch` tests an identical allocation search with
  `DNBOX_S1_PARTITION_BISECT=1` on s1c.c. It changes only initial
  allocation effort. It does not change live retime semantics.

With the correctly sized 300 ms delay (stride 38) the original Elite
peak was 16,791.79 us. The overlap experiment reached 13,308.08 us with
1,284 bytes of app flash and 15,228 bytes of pool margin. Both still
recorded 147 DMA XRUN indications. A faster measured peak is not a pass.

## Dry diagnostic

After the workload patch, apply `dry-workload.patch` to bypass all of
the synth's post-voice effects. This is not simply a zero wet mix; the
effect code does not advance. The polyphonic oscillators, envelopes and
per-voice filters still run, with the dry volume applied equally to L/R.
The firmware's ordinary master path is unchanged.

Optionally apply `dry-optimize.patch` for native optimization of the
voice compilation unit and exact sine/ladder transformations. To
exercise six notes instead of four, also apply `dry-six.patch`.
Run with the same emulator command and capture options, and use
`dry-readback.py RESULTS [4|6]`.

In the 8-second Elite test, the dry original had 0 XRUN at 11,550.38 us,
above the project target but inside the raw hardware period. The dry
optimized four-voice variant had 0 XRUN at 10,398.83 us, all 128 held
halves below the 10,809 us target. The six-voice optimized variant
reached 14,400.92 us and 147 XRUNs. This is not a release or a polyphony
limit: do not discard voices or effects in an installed build based on
this experiment.
