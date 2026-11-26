# SuperJ6 independent level probe: Apple Distortion

The native Apple effect responds nonlinearly at publicly reachable Crusher
settings even with Rounding zero. The installed SuperJ6 AUv3 does **not**
produce the same wet/dry response as that bare setting. Do not infer its
input amplitude or promote a firmware gain change from this capture.

## Evidence

* Apple component `aufx/dist/appl`, version 67072; exact stereo offline
  44.1 kHz. Factory Decimation Mix 100 and Soft Clip Gain -0.9 dB left
  intact. Constructor-mapped Delay, Polynomial and Ring Mod mix disabled;
  Rounding 0; Decimation 6.06%; Final Mix 0 or 100. All physical controls
  read before and after render. Two input shapes across six peak levels
  plus repeats, 28 signed renders. The bare 0.2 inputs repeat bit exactly.
* Installed AUv3 `aumu/j6ct/AuPr`, version 67072, known binary hash and
  one fixed A4 MIDI cue, 87 velocity, no concurrent effects varied. All
  public controls read before and after. Both arms hold #107 enable and
  #109 amount at 1. #108 takes 0 or 1 after a forced toggle so an unchanged
  stored parameter cannot suppress the write. Three oscillator fader
  positions and mid-position repeats give eight signed renders.
* Bare wet/dry fundamental falls monotonically from -2.66 to -7.89 dB
  across peaks .05 to 1.2, with relative phase -47.7 to -48.4 degrees.
  Installed wet/dry instead observes **-1.34, -1.01 and -0.97 dB**, with
  **-16.35, -15.78 and -14.81 degrees**. Even at the lowest bare peak
  neither gain nor phase admits a direct bare full-wet match. The native
  render repeats' signed whole-buffer differences are -38.85 dBr dry and
  -29.39 dBr wet, so do not interpret small changes as exact. The installed
  dry arm's harmonics also change with fader drive. These are observations
  at public readback, not a probe directly inside the graph.

Next: resolve effective wet routing, mode/order and whether the dry arm
truly bypasses the nonlinear stage before attempting inlet reconstruction.
This is not proof that a particular route, blend or gain owns the mismatch.
No production DSP, pool budget, flash budget or dropout gate is changed.

Recompute from the signed stereo .npy files:

```sh
python3 score-bare.py native/bare
python3 score-installed.py native/bare native/installed
shasum -a 256 -c SHA256SUMS
```

The GUI app supplies the actual render context; the standalone CLI did not
see the Apple component registry. `bench.swift` is the exact GUI source
used for the installed test and includes the standalone Apple-stage
profile. It reuses the existing offline bench app resources and does not
contain an original app or plug-in binary.
