# SuperJ6 Build9 delay reference and compact comparison

This is **host tooling**, not a change to the firmware release. It replaces only
the SuperJ6 delay approximation in a working copy of the owner’s ARM64 bench.
The rest of the wet chain is not qualified by this comparison.

The old bench had integer timing, additive wet mix and a final left/right
output swap. The Build9 app instead forwards AudioKit `dryWetMix` as a dry
coefficient, reads fractional taps by linear interpolation, and has two modes:
independent channel feedback and an actual two-line ping-pong feedback loop.
The ping-pong path uses the previous right tap in the next frame's left input.
It is selected by the original `setPingPong:` boolean, not by a blend.
For the three impulse contracts the old stage fails; the replacement passes.

The oracle accepts only the privately supplied Build9 AU (pinned SHA in the
script), decodes and evaluates its ARM64 instructions for the AU parameter
setter, core control setters, renderer and fractional-line push. It runs the
original arithmetic/control paths, with fixture-allocated ring memory and
host libm for the imported powf/fused multiply add. It is deliberately not a
general CPU emulator or a full Apple AU host. No original binary, PCM, factory
presets, or private bench files are included here.

`build9-receipt.json`: **3,072 bit-equal PCM words, 16 applicable cases**.
The fixture matrix includes 44.1/48 kHz, dual mono and ping-pong, stereo
impulses, fractional delay on random stereo signals, a live change of
delay time, and AU clamping for feedback and mix. Time, feedback and mix
updates are immediate in the fixture; the application's AU automation ramp,
preset-to-effect controls, LFO and downstream FX are not qualified by it. Host
libm results for arbitrary states are not claimed to be identical to Apple's.

## Compact witness

The second probe calls the actual recovered `juno6_delay_native.inc` with the
shared ring provisioned, a settled mix and zero feedback, at 44.1 kHz. It
isolates the first echo. At 496 ms the current history trims 220 Hz by 1.10 dB
and a 3 kHz tone by 22.91 dB; at 1.004 seconds the 3 kHz tone is down 26.77 dB.
These are controlled tone results, not scores on full presets. The fixed
compact buffer steps down to 1,378 or 689 stored samples per second for those
times. This explains why the compact repeats lose brightness and hands the
next compression candidate a comparison it must actually beat. Do not fix
this by changing the authored mix, and do not count increased host RAM as a
firmware fix.

## Run

Requirements: Linux, Python with numpy, C compiler; LLVM 19 and a private
copy of the Build9 AU binary for the instruction oracle.

```sh
python test_delay.py
python compare_original.py /private/path/to/original-build9-AU
python compare_compact.py /path/to/recovered/firmware/src/dsp
python apply_to_bench.py /private/path/to/copied/bench
```

The overlay imports the C host model at the bench's delay call site and
corrects the fake Swift receiver's boolean recording. The patched delay entry
was exercised with a ping-pong impulse. Full `render.py` still requires the
bench's original dependencies and private assets; this receipt does not
claim an end-to-end SuperJ6 render or an Elite/Pro flash or timing pass.

## Same-storage codec experiment (offline)

`codec_probe.py` tests 4-bit IMA ADPCM with per-channel predictor/index
headers. A hypothetical 4,096 B stereo ring can store 3,840 stereo frames
in 4,020 B including those headers. Keeping a restart block free allows
7,350 stored frames/sec for the 496 ms echo. Its 3 kHz first-echo level
now differs by +0.09 dB (about −15 dB waveform residual), rather than the
compact path's −22.91 dB level difference. A synthetic bright chord shifts
−0.11 dB in level, with about −16 dB waveform residual. At a one-second
delay the same budget still cannot reproduce 3 kHz; overall chord level
is insufficient evidence by itself.

These are offline probes using Python's IMA codec and SciPy's zero-phase
resampler, not demonstrated live firmware results. No scratch, filter or
random decode cost is included in the 4,020 B. The encoded data budget is
a lead for a streaming implementation; wet feedback, control moves,
preset renders, memory ownership, and native timing are not qualified.
Run `python codec_probe.py` with SciPy and Python 3.12 (audioop).
