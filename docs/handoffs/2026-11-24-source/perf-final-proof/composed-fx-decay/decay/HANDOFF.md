# Synth One AMP Decay LFO: source-only ID52 host handoff

- Standalone FM preimage manifest `72bc8830364911dce8d70d2055efdc508e7d5ab4b452d10b3fe73b5efbb0f4db`; AMP Decay postimage `83583e9dbcd9a4e424493a5fd4178baa7852dce5c550f32ded4c46c64f0d898f`.
- `amp-decay-route.patch` SHA256 `d037e88b2b148524be77e4a889607d9453a42c17247ed40246858b84c32631c2`.
- Current FX+FM composition `0f4f18b8e7212e66228c9403bf3630156f3099ee58c6b0aebd1cbb72c5027ee2` was independently verified and the same patch dry-run/applied privately without fuzz. Result full composition manifest `02069b8e9ebdc013e8fdbe7d79e8508245aba57d1008b21182d8d20f81bd47b6` in `FX-COMPOSED-POST.sha256`. The standalone receipts below do not stand in for a composed native build.

## Receiver and UI

The authenticated upstream `S1Parameter.h` assigns ID52 to `decayLFO` and `S1DSPKernel.hpp` declares its 0..3 discrete domain, as well as the authored AMP decay-time limits .005..2 seconds. Original `closure/experiment/probe/portable_voice.cpp:243–252` multiplies AMP decay time by descending LFO1, LFO2 or their average, clamps it, and writes AMP ADSR's decay-time input. Original Soundpipe `.../Soundpipe/modules/adsr.c:74–84` samples the decay time at ATTACK→DECAY, not during already-latched DECAY. Compact `dsp/s1c/s1c.c:352,396` and `dsp/s1c/s1u.c:33–45` use the same control law and transition. Neither original nor compact should retime a sound already in decay. This is a source-proven endpoint/transition edit, not whole envelope/audio parity.

MOD ROUTE K4 `AMP DEC LFO` exposes OFF/LFO1/LFO2/BOTH, with no sixteenth page. It leaves FM, oscillator mix, resonance and existing independently sampled filter/noise controls undisturbed; no DSP/partition/rate/memory-map changes.

## Same typed protection, independent choices

Reuse existing bit23 ownership and existing byte[30] as a radix-five code: `fmDigit + 5*decayDigit`. Both digits 0 = absent/exact authored value, 1..4 = explicit OFF/LFO1/LFO2/BOTH. Prior FM 0..4 is byte-identical, 5..24 previously invalid now record Decay, and other byte/tag combinations still fail closed. Existing route byte[25], all project/note-lock tails/extent, legacy 0..3 migration and newer tag ordinal unchanged. No new record, arbitrary flag, header or edit extent. At most three independent optional five-state controls fit this byte (5^3=125; 5^4=625); no fourth control by pretending factory-absent is OFF. MOD ROUTE is now full, so future controls need a grounded free column elsewhere without adding a page.

## Host checks

Run `./replay-host.sh EXACT_FM_SOURCE AUTHENTICATED_S1_PACK` (uses verified manifests and real generated host inputs). Both physical owners and every routed mode: 0/4096 frames change on already-decaying notes and the current pole is exact; fresh attack/decay changes 4096/4096 at real factory ordinal 4; authenticated 5 ms clamp ordinal 164 yields 0/4096 with exact pole throughout. Verified actual AMP and FILTER envelope independently, plus cutoff/noise base, prior route state, LFO phase and peer PCM. Both modes and clamp programs have genuine enabled noise, not zeroed auxiliary path. Physical tests issue first adjacent OFF→LFO1, then other supported detents; checks cover same-preset loop, real capture, busy edit refusal, held lock restoration and project reload.

All 3125 independent combinations of both typed groups and all 512 tag/payload possibilities were checked. Direct bank/protected path case covers both owners' oldest slots 0..3 with tagged and untagged historical input. Exact preceding reader accepts previously valid FM + other routes and sound/locks, and refuses newly typed Decay Sound/lock before any project writes. Untouched 694+INIT compact PCM identical at 1024 stereo frames per program, common SHA256 `2b7c2c740edb57be12652f58c81a538815dd21c1ca8f66b0971cad51df0f331f`. Host core/S1/union/owner/arena sizes remained 2800/3440/3896/5520/4252.

No native, emulator, device, unsupported full original A/B, browser or publication claim. No Git or product writes.
