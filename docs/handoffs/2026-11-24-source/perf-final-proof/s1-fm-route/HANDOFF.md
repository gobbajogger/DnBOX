# Synth One control completeness: FM LFO, source-only

Resonance+PAT exact PREIMAGE manifest 2580ede026ee7c031db597206893cbb1f077963406832c381af34bf4be984367;
POSTIMAGE 72bc8830364911dce8d70d2055efdc508e7d5ab4b452d10b3fe73b5efbb0f4db.
fm-route.patch SHA256 5a16b97b679793e3b40328e4f280115e0a4493f0df1b502d85e12858da662fb9.
The patch touches six app source files only and does not alter DSP, allocation, assets, or the 15-page count.

## Five missing original sound controls at this preimage

Stable IDs and the 0/OFF, 1/LFO1, 2/LFO2, 3/BOTH selector domain come from pinned
`s1-audit-data/original-inputs/closure/master/AudioKitSynthOne/DSP/{S1Parameter.h,Kernel/S1DSPKernel.hpp}`.
The authored preset register plus real DSP receiver establishes musician-facing sound behavior, not a verified enumeration of exact iOS panel names.
Original receiver anchors in `closure/experiment/probe/portable_voice.cpp` or upstream kernel process, compact in `firmware/src/dsp/s1c/s1c.c`.

| ID | Control absent from current firmware UI at preimage | Receiver comparison and status |
|---|---|---|
| 54 | FM index LFO | Original portable_voice 228–237 multiplies FM index by descending LFO1/LFO2/their mean, OFF unity, then clamps 0..15. Compact lines 351/414 exact same receiver algebra; a cached control-rate branch updates index on its own control tick. This patch binds the already-live endpoint, not whole FM rendering parity. |
| 55 | OSC 2 detune LFO | Original portable_voice 188–205 scales the additive Osc 2 detune term by ascending LFO and cached oscillator Hz/C4; OFF leaves nominal detune. Compact lines 346–348/403–405 scales the same term but uses compact voice frequency and oscillator/timing. Needs real held pitch/mono witness. |
| 56 | Filter-envelope amount LFO | Original portable_voice 329–339 scales FILTER ADSR MIX by descending LFO, then `cut − cut·mix·(1−FENV)`, using original envelope/filter bounds. Compact lines 357–359/420–421 multiplies the same parameter, compact filter/envelope and 64..22050-Hz clamp; exact full transfer not demonstrated. |
| 52 | AMP decay-time LFO | Original portable_voice 243–252 scales decay seconds then clamps 5 ms–2 s before the original Soundpipe ADSR. Compact 352/396 same control law and bounds but one-pole AMP ADSR, not an original envelope A/B. Needs decay-phase audibility and sustain/initial-phase/lock tests. |
| 59 | Tremolo LFO | Original kernel process 172–181 multiplies post-crush mono before pan/effects; compact 934–943 places the same multiplication before its autopan/phaser and subsequent FX. Crusher/pan/FX differences survive; scoped test should show at the pre-pan/wet boundaries, not claim full stereo parity. |

## Independent encoding and compatibility

Only about the active Synth One guard, original current `cell[30]` was required zero; historical slot-2 factory 0–3 used the same location, so this patch **always requires** previously invalid mask bit23 for nonzero codes. Code 1..4 is an explicit OFF/LFO1/LFO2/BOTH; zero unmarked retains the exact authored factory value. Wrong combinations are rejected without mutating the model. The marker is above the historical migration mask and cannot be reinterpreted as an old slot; admissibility first qualifies the engine. [7]/[21] are still the sole program ordinal, while tag preset identity stays unaffected. [25] and the validated note/project tails are untouched; no extents grow. A numeric proof, not allocation: 256 code values permit at most three independent five-state selectors in that new typed group (125; four would need 625); a future rollout would have to explicitly retain its current canonical 0..4 assignments at both readers. Also only one unused MOD ROUTE column remains now, so more than one new destination needs placement in relevant existing panels or an owner-approved editor consolidation, not a sixteenth panel.

## Exact host verification

`./replay-host.sh <exact preimage source> <authenticated S1 pack>` from this packet,
requires the previously supplied generated host build inputs in source's build/gen. Replay exited 0.
- Historical slot-2 forms factory bytes 0–3, seeded from both owners' actual tagged Sound structures and clear/old external tag, were admitted/retagged to identical INIT before and after. Direct typed marker and legacy collision refusal verified. Prior exact reader accepted non-FM projects and captured existing locks, then refused FM Sound or note-lock paths before any of the six project storage writes (352 checks).
- Test-only expected red preimage misses ID54. Postimage 25,389 checks: exhaustive 625 whole combined routes, 512 marker/payload possibilities, both owners actual MOD ROUTE K3 detents/owner readback, lock capture, held application, note exit/stop/factory restore, full save/reload and unrelated old routes and tails. Both owners change 4096/4096 selected real stereo frames per mode when FM is audible, and 0/4096 with zero FM level; peer, LFO clock/shape/depth and base FM index remain exact.
- Retained original Resonance regression 7,592 checks. Both synths, both owners all 15 panels physical Select forward and backward; 14 unrelated SuperJ6 native-renderer LCD bitmap captures byte-exact. Four actual FM LCD captures preserve OFF/LFO1/LFO2/BOTH. Untouched INIT + 694 stored programs, 1024 stereo frames per program, common host PCM SHA256 2b7c2c740edb57be12652f58c81a538815dd21c1ca8f66b0971cad51df0f331f.
- Host sizes (core / S1 subobject / backend union / complete owner / FX arena): 2800/3432/3896/5520/4252 before, 2800/3440/3896/5520/4252 after. The float preserves the authored route word exactly; it does not enlarge the owning union in the measured host ABI.

No native/image, release, timing, emulator, hardware, browser, or universal third-party-reader qualification. No product or remote writes.
