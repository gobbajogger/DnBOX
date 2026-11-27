# Init defaults: controlled hold-level ablations

Owner-exported Your Init Preset has the same UID as the native capture
(8733EFD7-4000-4C31-BC52-5AD9E4B707E1), with no authored parameters. Thus the
native live readbacks, not zero or the compact shared defaults, define this cue.
The native preset has sustain 1 on both layers and its saw receiver slider at 0.
The current compact Init starts two equally loud in-phase saws and sustain .72.
Direct ablation only (not claiming the compact layer route is the matched receiver):

- Mask 0: no overrides; -13.07 dB held level relative to native.
- Mask 1: sustain only to 1; +2.85 dB over baseline, but worse signed residual.
- Mask 2: second layer gain 0 only; -6.02 dB from baseline.
- Mask 3: both; -16.24 dB relative to native.

These mismatches are real. Their combination does not cure the held level mismatch,
and normalizing the master would hide it. No production default or signal flow changed.
Use native readback/source mapping to finish matching envelope and receiver inputs before
changing the mixer. This experiment excludes envelope time conversion and filter routing.

    bash run.sh /path/to/firmware-fit-next /path/to/extracted-reference
