# Build9 original pitch-bend receiver checkpoint

Apply qualification-sj6-midi-bend.patch after the separate Synth One SEQ and SuperJ6 OTA wheel corrections. Only the SuperJ6 MIDI word is normalized by 16384 from the original receiver; the calibrated physical SMK Pitch wheel still reaches its exact endpoints. The prior original mapping is in the corresponding sj6-bend-build9 handoff.

The exact updated control source was fully object-compiled for both models first (source pin 13f8a6ce85fb7ed9cdd46916de9d6e1f7ee548deaff93c8327269df7440bba9d); then both linked in the existing guarded diagnostic configuration:
```
guarded elite XIP 552484 free 188 pool usable 632 read-only FDATA 61328 diagnostic slot3 span 32292 slot3 free 476 read-only FITTAB 3929 header 44395244020000000100000070ef000036162a590f2e1043a0ad28593ee2f83c ELF 5fd7a4e2ed0cc2926de0c0ffd726a39001b9ded13710ef22bdb6350505be1e0d
guarded pro XIP 552472 free 200 pool usable 632 read-only FDATA 61328 diagnostic slot3 span 32292 slot3 free 476 read-only FITTAB 3929 header 44395244020000000200000070ef000036162a594b7f0e3b76a36d86ae7a1094 ELF 3cb76eaa128c5dcc2b601cc95b39a78837524ee3aa991eb1d51a4e1c075b09c5
```

On the immediately preceding snapshot with the same other control patches: focused Synth One UI/engine 155 checks, including both independent SEQ controls and legacy normalization; Delay Input Resonance remained editable and produced bit-identical stereo active output across 122,880 frames at distinct target values; SuperJ6 original OTA Mod Wheel matched the decoded transfer within 0.001953125 Hz for all 128 positions and did not alter the normal receiver. These are focused host checks, not a claim that all direct original panel controls are qualified. All twelve populated strict timing shots of that preceding snapshot failed (Elite 12,281–12,456 µs, 18 total xruns; Pro 12,002–12,712 µs, 16 total xruns). Do not publish or flash either image. Exact latest MIDI tests were started only after its fit and not yet completed at this checkpoint.
