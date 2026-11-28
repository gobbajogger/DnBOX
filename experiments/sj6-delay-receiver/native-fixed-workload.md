# Fixed-sample native dual-synth workload

Use a disposable firmware worktree. This fixture is not a build option or a
release. It must never be enabled in the installed image.

Apply `experiments/sj6-delay-receiver/native-fixed-workload.patch` to the
source tree corresponding to f316f7dd. Build with the normal SMK-37 Pi32/SDK
environment and these unversioned Elite flags:

```sh
MODEL=elite X0X_UI_SMALL_CANVAS=1 X0X_BASS_PRESET_PREVIEW=1 X0X_BASS_TAGGED_SOUND=1 \
  X0X_BASS_NOTE_SOUND=0 X0X_D9_TANH_BOOT=1 X0X_MIAMI_N513_BOOT=1 \
  X0X_CHI_FACTORY_COL=1 X0X_D9_ENTROPY=1 X0X_SUPERJ6_FACTORY=1 \
  X0X_SYNTHONE_PATCH=1 X0X_S1C=1 X0X_SJ6_NATIVE_DELAY=1 ./build.sh
```

Use the project's pinned FM1 emulator and matching approved staging controller
if required by the runner. Only a new disposable virtual NOR file is allowed.
Do not point any installer at a device. From a clean results directory with a
`work/` subdirectory, copy the exact ELF as `bench.elf` and launch:

```sh
FM1_NO_SKIP=1 FM1_STRICT=1 FM1_DUMPRAM="$PWD/ram.bin" \
  FM1_DNBOX_PCM="$PWD/capture.s16le" \
  "$FM1_EMULATOR" --fwsc "$EMULATOR_PACKAGE" --mhz 360 --usb --shot 8000 \
  --flash "$PWD/virtual-nor.bin" >run.log 2>&1
python3 "$SOURCE/experiments/sj6-delay-receiver/native-fixed-readback.py" "$PWD"
```

Do not supply a physical-key tape or outside MIDI events for this fixture.
The test configures the two actual owners before sample 98,304; Juno is
typed and using its own active Delay ring, and Synth One targets Delay and
Room on. One typed Juno note and four distinct Synth One notes reach the
real note route at generated sample 98,304 and release at 163,840. The
readback validates acceptance order, exact event samples and active voice
count; a valid event trace is **not** a zero-dropout result. The independent
922-event physical-key stress gate stays unchanged. This fixture calibrates
controlled workload timing, not final original-app sound parity.

The initial Elite run matched all ten events and the expected activation /
release counts. It did not pass performance: maximum 21,156 microseconds
and 147 XRUNs over the overall 8-second test.

## Optional observers: use separate worktrees

After applying the workload patch, apply exactly one observer patch
and rebuild both comparison arms with it:

- `native-fixed-phase.patch` reports per-phase peak time and the generated
  half indices that exceeded either the 10,809 us project target or the
  physical 512-frame budget. It tracks the DMA index change independently.
  The observed render time is taken at the existing timing boundary;
  bookkeeping runs after DMA acknowledgment and the index comparison.
  An alternating DMA index is not a count of every missed boundary.
- `native-fixed-digest.patch` fingerprints the 32-bit output words in each
  of 128 held halves and the first 32 release halves, before the DMA half
  is acknowledged. Use the fingerprints to locate a change in rendered
  output even when the post-DMA capture is affected by lost halves.
  Hashing can affect timing, so this arm must not be used to assess speed.

Record the exact ELF for each arm. The standard workload acceptance checks
still apply. From the source root compare the two results directories:

```sh
python3 experiments/sj6-delay-receiver/native-fixed-observer.py timing "$BASE" "$CANDIDATE"
# or, for two separately built correctness arms
python3 experiments/sj6-delay-receiver/native-fixed-observer.py pcm "$BASE" "$CANDIDATE"
```

The held phase covers generated samples [98,304, 163,840). During release,
physical keys are no longer down, but envelope and FX tails may still cost
time. An over-budget render does not by itself tell you how many DMA
boundaries passed. Keep the 922-event physical-input acceptance separate.
