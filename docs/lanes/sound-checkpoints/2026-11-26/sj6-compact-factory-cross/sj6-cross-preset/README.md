# SuperJ6 signed cross-preset diagnostic

Uses three authenticated installed original factory UUIDs and actual active
AU readback. All 170 live entries retained identical values after the cue.
The original has two Brainwave authored-vs-live differences, and zero shared
differences on Solidify and Raw. This is a supported-control projection,
not full parity or an Elite/Pro test. Manifest lists unmapped nonzero values,
many dormant UI controls and some active/legacy aliases. Compact projects
legacy cutoff #7 and resonance #8, not aliases #129/#130. Quantized receiver
values use native firmware mappings. Fixed velocity uses legacy raw 87,
without calibrated original high-note taper/jitter. Installed Your Init was
refused because no stored parameters exist; no stock substitute rendered.

A fresh typed compact original_runtime instance per arm uses exact matching
MIDI order, blocks, 16384 warm frames, 44.1k, and 148684 frames signed little
endian interleaved Float32 stereo. Host refuses active unsupported arp, seq
and scale rather than silently disabling them. Score removes only the fixed
warm prefix. No normalization, fit, phase/time adjustment or clipping.

Reproduction:
  ./build.sh /path/to/kept-elite-1 ./run.private
  python3 export.py ORIGINAL_AU_CAPTURE.tgz J6presets.zip fresh.private ./run.private
  OPENBLAS_NUM_THREADS=1 python3 score.py ORIGINAL_AU_CAPTURE.tgz fresh.private /path/to/sj6-factory

Only compact generated PCM is bundled; no original program, PCM, or raw
presets. Meter source injection is observational, preserving all PCM exactly
(as cmp verifies). Master is raw #0 × .404 rounded once after FX and limiter.
Compact pre-FX peak <.034 and pre-master <.049 on these scopes; no sign of
saturation here, and no assurance for other scenes or later boosted stages.
Whole authored level deficits ~47.7/30.3/35.8 dB and spectral RMS ~31/16/36 dB.
Dry deficits ~46.2/28.8/37.8 dB. The original repeats themselves differ
−51.6 dBr Brainwave, −9.68 Solidify, and identical Raw. This data can't
justify fitting one common gain. Wet-minus-dry is response to MIDI, not
controlled same-input effect isolation. No firmware modified.
