# SuperJ6 original ID1 oscillator-one level, Init-only source checkpoint

Control 192–194 HOLD remains: exact existing LFO event proof does not identify the original reset/ArpShot producers or terminal epoch across event kinds. No lifecycle alias was added.

ID1 is independently authenticated in original Build-9 AU SHA256 a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1. Conductor layer1VolumeControl ivar 0x100809a88 has dispatch 0x10026c1f8→0x1000ecf8c→0x1000ecfbc, using the same normalized-linear 0..0.808 binary64 literal and receiver setter as ID3, but selecting oscillator1 receiver ref 0x1007bee48 (offset 0x98) rather than osc2 0x1007bee50 (offset 0xa0). Public audit identifies ID1 as neonpulseVolume. Only static inspection of this authorized input was performed; no original binary execution, private file content or presets were shipped.

Actual existing OUTPUT K3 now edits OSC 1 LVL while retaining independent OSC 2 LVL K4, common level and HPF. New gain is applied only to oscillator1, before its sub; leaves layer2 and master/pan independent. Untouched compact Init shows INIT and retains exact unity branch. Uses the same normalized binary32 arithmetic as existing ID3 (0..0.808, not a whole-app audio parity claim). Retained both scalar controls under pitched/vibrato spans. No new page or unrelated controls removed.

## Wire, verification and size

All 50 canonical old cell[31] states (5 shapes × 10 vibrato/reference marker classes) remain unchanged when absent. For each explicit position, 200 previously invalid marker values encode 4 groups×50 while 5 spare bits supply 32 levels: the high bytes of [13],[14],[18],[19] and the otherwise redundant PWM occupancy flag encoded as an XOR against actual [21] occupancy. Previous parser rejects all new forms on an invalid [31] even if numeric indices look unchanged. No [30]/ID3 or [16]/[17] potential link state touched, no note-lock payload layout extended. Decode masks isolate existing chorus/PWM/shape/shared-row values. No factory data restored.

- Active pinned source pre d06208a5c2785501736e8b7299682d403d141aa609daa740d52c531dbc9590f0; post cbc17b826eab9b78d13f6ac6928473cdb2b3d6da7c2f64f0d0cc1d8381b83685; patch ebc6cb0a8894bbaea3e37795acbfaebb90b13afec7d3e8c3bf5e5b4c4bd6714e.
- 153,900 real UI host assertions: 19,200 exact cross-products covering 50 marker classes×128 levels×3 PWM destinations plus store operations, both owners actual K3 edits, existing ID3 independence, genuine stereo change in the intended oscillator lane with audible sub and opposite lane exact, and save/load plus actual held-note apply/restore. Native-renderer SDL host stills inspected with all four OUTPUT roles distinct.
- Exact old host reader mounted both owners' combined Sound+lock and lock-only project shapes and refused all 4 before any write (96 assertions). The previous ID3 control/physical/stereo/save/held fixture also passes unmodified (451).
- Existing default stock/typed PCM byte-identical core probe SHA256 adcbbe1f7c64badb22b4a81a94f09c8db78d38102241730047a950da253d258a; AMD64 host juno/core/backend-union/owner all unchanged 2992/2456/3896/5520. Core-only host -Os 9226→9285 bytes (+59); not total code growth, native XIP, hardware or heavy-replay timings. This is an isolated source candidate, not a release claim.

Reproduce the principal current/old two-owner fixture via replay.sh EXACT_CONTROLS_SOURCE GENERATED_HEADER_DIR. No Git, device, preset, or product writes.
