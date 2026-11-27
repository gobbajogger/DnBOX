# SuperJ6 whole-render signed A/B

No engine edits. Source selection is explicit; this is Linux host DSP evidence, not a firmware timing, storage or release pass.

## Repeat from workspace root

1. Materialize existing owner-private warm Brainwave archive at `sj6-parity-suite/private/warm-brainwave-native.private.tgz`; prep pins the exact archive. See the project reference-audio index.
2. `python sj6-parity-suite/prep.py`
3. `bash sj6-parity-suite/build.sh firmware-fit-next`
4. `python sj6-parity-suite/score.py --pilot` for a fresh diagnostic. It writes private candidate PCM only into the suite. Diagnostic never admits a sound comparison.

For whole-path authored evidence (when actual compact loading exists):
`python sj6-parity-suite/score.py --arm authored <planar-f32le> <receipt.json> --arm verb-off <planar-f32le> <receipt.json>`.
Each receipt must be emitted by the real producer, not copied from a reference. Schema and checks are in score.py: rate/frames, cue SHA, exact callbacks/schedule, exact factory UID, 170 before/after active Float32 words, per-event frame/key/vel/held-count, output and whole-build SHA256. Both arms use the same build. Script admission validates matching fixture/custody fields, not the authenticity of an arbitrary hand-written receipt. Keep real knob/gate readback evidence and build alongside it. No per-patch gain, onset alignment, phase rotation, or magnitude-only wet subtraction. Room-off is **not** all-effects-off.

## Present reference coverage

- Exactly one admitted complete modern Mac cue: Brainwave Mono Bass, original Bonus UID `1E5E5DDF-B5C6-4503-BDBC-CAE9CD1D0A94`; official native AU executable SHA 1d6edf2bf5dd30906b148e10f50e2470a9f55d13d7f64ab3515e3c2294f1c257. Native authored/room-off and independent same-arm repeats, 170 control words stable per capture, only ID11 differs between arms. True silent preroll 16,384 frames, aligned 44.1 kHz for 148,684 frames, 1,164 callback blocks and eight MIDI events. Source warm pair includes native app stochastic/repeat spread; don't score the net wet signal below its repeat floor as firm independent evidence.
- Warm Brainwave Main/Sub, isolated Crusher or HP leaves, OTA order, and direct native fan-in captures are useful secondary comparators, not interchangeable with whole authored control vectors. Existing 14 original S4 cue inputs are not equivalent to a verified 14-pair modern native whole-preset corpus. Historical iOS Build 9 remains a distinct executable/profile.
- The product factory load currently exposes a limited typed row, not a proven full 170-word modern Brainwave adoption. The runnable pilot uses the real combined donor host INIT slot, verifies an invalid factory ordinal is rejected and checks actual held counts and eventual all-off state, touches only the bounded native room receiver, and verifies finite PCM. Its compare is expected not to admit Brainwave as its identity.

## Existing baseline (L/R)

- Whole authored residual −0.074/−0.076 dBr; room-off −0.054/−0.054 dBr; signed room-difference +3.505/+3.370 dBr. **Different presets; plumbing only, no isolated DSP attribution.**
- Original repeat whole authored −53.647/−53.727 dBr, off −49.144/−49.144 dBr. Whole room-difference repeat −17.713/−18.441 dBr (absolute −70.313/−70.297 dBFS); last-second difference repeat −9.113/−11.771 dBr (absolute −75.138/−75.092 dBFS). Wet-return delta is assessed separately and is much noisier than the whole render.
- Build/fixtures: host.flags, host-source.sha256, diagnostic.json. Score target remains signed dry/wet/chord/release/whole, not loudness correction. Keep the fixtures and thresholds separate from any future matched patch integration.

## Closest existing compiled control path

The current compose flags enable typed sidecars and native Delay, Room, Unison, J60/Classic chorus, LFO, OTA and limiter diagnostic receivers. The closest full product-shaped dispatcher is `engine.c` around its `live137(mot_base_snd,v)` branch; `sj6_137_host_witness.h` defines the REV137 carrier, `canonical137` validator and actual byte encodings; `original_presets.inc` admits that carrier through the staged validation path. Its live gate specifically requires the actual `proj.sound`, correct sound revision and owner header, a SuperJ6 INIT cell and **no factory tag**. The factory loader/AMP overload instead accepts the small typed row (plus explicit AMP input where selected); neither path is proof that the modern native UID and all 170 authored values were loaded. These are product/display transport values and typed receiver encodings, not a one-to-one native identifier/Float32 table. To qualify next: an authenticated downloaded UID-to-carrier map with an exact per-control effective-state/readback ledger, explicitly handling unmapped/quantized/metadata-only IDs, applied through the actual dispatcher; then independently capture both authored/no-room arms with identical other state and cues. Where a legal compact value cannot match the native word, reject the strict fixture rather than stamp native readbacks into its receipt. A later separately declared approximate-control experiment could still be scored, but not counted as this strict fixture.
