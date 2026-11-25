# Recovered Synth One DSP vs corrected six-preset reference

This is a headless host diagnostic. It does not stand for an exact newer
whole firmware build, UI/physical MIDI behavior, or engine sign-off.
 
The owner-supplied corrected listening ZIP contains 44.1 kHz PCM16
reference from restored upstream AudioKit Synth One + Soundpipe, not
an iOS capture. This script checks its digest and recovers the two arms.
The six 0.75 s notes, 128-frame unrecorded preroll and 1.5 s release were
verified by reproducing all six archived compact signals to PCM16
precision (`-75.35` to `-87.53` dB residual after the fixed preview gain).
No alignment, candidate gain fitting, EQ, or normalization. A fixed
per-program gain is calculated against known old raw PCM only to undo the
archived preview gain; the original unity Float32 files are not available.
 
The baseline is published source 73749f55, with private stock 694-program
FDATA `867381277802511def492ff43db13cc3b59510bc32bc409a49329bda531b7b44`.
The second arm uses the recovered composed controls' S1 DSP only, with the
S1 delay-resonance patch and S1 portions of 05,17,19 applied in order.
S1 part preimage `34c14d91dfe5f715a8242cfde1b4ce9887991d5626c86d00c253e327f0f08ec9`,
header `4ce8f1597d6a21b8478061d91358385f1a659ea4461e9fb04ca19e06d22a660d`,
both preimages pinned in the delay-velocity source handoff. Result S1 part
`f6c53f745faa59377005d10b50d8f09d9fb0d42aa3d605c6f7fe3d62e6abbf2e`,
header `e5dd8b0af0d6bbc1fb89f009989cfae98e7153bd72c6004f6babcb15d97393e9`,
renderer `20628c91a910bc99f6ccff7ce76798985e4e1b3c373fe9f14faabc2ff8bb2f56`.
The pack is enabled in this private host fixture only. The actual
Init-only native candidate hasn't been substituted or published.
 
| Program | Last-second error before | Recovered S1 DSP |
|---|---:|---:|
| 001 | -0.057 dB | -0.057 dB |
| 062 Pad | -6.555 dB | -6.299 dB |
| 113 Organ | -4.697 dB | -4.697 dB |
| 128 | +0.570 dB | +1.250 dB |
| 278 | -0.769 dB | -0.804 dB |
| 435 | +1.741 dB | +1.741 dB |
 
Five fail at least one agreed RMS limit (including full channel RMS).
The spectral score here is a diagnostic and not inferred to be the missing
original spectral implementation. With tested effects disabled the Pad and
Organ headless waveforms are bit-identical **between DnBOX builds**, not to
the original engine; an independent dry original capture is not in the ZIP.
Diagnostic Pad delay decimation is 47 in baseline / 45 recovered, with a
2,152 B delay pool. Organ both arms has delay off and reverb on. Recovered
Organ reserves delay history at decimation 35 despite bypass. Don't derive
audible equivalence or legitimate shared live effect state from those facts.
 
## Reproduce
Place these scripts in a standalone directory. Set DNBOX_AB_WORKSPACE to the
workspace with stock `fw`; set DNBOX_AB_LATEST to your recovered headless
source. Set DNBOX_AB_HANDOFF to the restored composed-controls folder.
 
```sh
bash build-run.sh
python reference.py /path/to/Synth-One-listening-check-corrected.zip /tmp/s1-six-halves
python measure.py /tmp/s1-six-halves calibration.json
```
 
The WAVs, generated Float32 captures, native FDATA, and original binaries
remain private and are not included. Cross-engine reuse evidence is separate
in EFFECT-IDENTITY.md.
