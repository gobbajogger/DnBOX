# Synth One exact fixed-point glide (local; no promotion)

Source d54a4ebb992ee014920ed5b3051cbc0b19abff5b, plus arch-delay-fold/delay-fold.patch then glide-fixed.patch. No other source edits. Applied on an isolated local checkout only. Native headroom +16/+28 B XIP (64 B growth over fold, fixed pools 512 B, no static RAM/scratch growth). Official packages/ELFs tested at identities in the receipts. No PSRAM, no reset/guard/transport change, no discarded effects or arp.

Read-only actual native frozen state measurement first: arch-delay-fold/glide-count.py uses verified target struct offsets and the bit-exact mul/mul/add binary32 recurrence, not equality of current to target. Both fold-model 2 s and 8 s snapshots had 46/46 bitwise fixed controls; original bogged model at boot 16/46, by 8 s 46/46. These are endpoint observations, not a whole-scene occupancy claim.

For S1C_FAST+S1C_CSM+S1C_NO_SEQ only, run original frame-zero recurrence exactly once and OR the XOR of each old/new IEEE value. If bits match for all, the recurrence is fixed with immutable audio-owned targets and coefficients, so do not repeat it in that span. If any value moves use the unmodified dense loop from frame 1 to last. No mixed precision, parameter snapping, altered FMA, or changed rate. Adjacent spans recheck after all native sequencer/arp/settings boundaries. FX and note order left alone. For other configs the existing path stays unchanged.

Proof against fold-only actual host DSP: still exact for 36,900 tap and 34,689 feedback checks, complete PCM and state across 2,400 spans / 183,400 samples from eight real presets, signed zero targets with both signs and zero-length span checks included. Reproduction scripts here. This is not the original-vs-compact Synth One A/B acceptance gate.

| Strict native regression | Fold Elite/Pro | Fold+glide Elite/Pro |
|---|---|---|
| Original stock unchanged hot shift 2 | 11039/10920 us | 10886/10884 us |
| Original stock shift 2 completed halves, late/XRUN | 647, zero both | 647, zero both |
| Native 2712-byte S1 preset + Miami + arp real boot | 11127/11122 us, no XRUN, 129 halves | 11135/11127 us, no XRUN, 130 halves |
| Native real active shift 0 | 16288/16306 us, 158 XRUN each, 488 halves each | 13118/14663 us, 36/107 XRUN, 610/539 halves |

Cold/warm standard PCM and pixels compared independently when files present; official heavy offset2 full PCM and pixels differ on both boards from unmodified d54a4 despite passing the threshold on that single offset. Everything still red as a release candidate; never represented as 12-offset passing. Active real fixture verified all 2712 bytes including arp at boot (existing authorized project object overlay; official scene not mutated).

RVHP coefficient-cache opportunity was reviewed rather than guessed: in fold active profile exact caller coefficient sequence PC 0x0205cb34..0x0205cb89 ~0.115% of core0 counts, fm_tanf self ~0.081%; downstream tanf helpers shared with other math. No basis to expect required multims gains by caching this alone. A future exact stable-glide extension can retain original sample phase, recompute at f=0 every span (rate/owner/initial state) and skip only a repeated f&7 identical RVHP sample, without growing persistent DSP state.

## Exact source/checkpoint and last-image active profile

Local-only source history: d54a4ebb992ee014920ed5b3051cbc0b19abff5b (base), f70ea4ed851fe3f8a9f071620a86632c38246f43 (fold), 1cb4b0af6361e3d1a1821db87650c20a79ab7270 (fold + glide). Native runs used exact source archives of these trees, not an embedded release/nightly version. All 1707 tracked native build-input blobs matched in all three archives. See source-identities.txt, build-source-manifest.txt and local-experiments.mbox.

All 694 factory programs plus INIT exact output digest remains 24d1dab508f35ebc across base/fold/glide. No silent or nonfinite programs. Host CPU overload is not a native pass. No new Synth One original-reference listening/objective A/B was run or claimed.

Last-image Elite real active 5100–6500 ms TPROF (exact 1cb4 image, same physical events and valid project objects) does not preserve PCM, RAM, panel, core CSV or log. Same flash and input confirm a diagnostic workload only. Core0 s1_part_render 18.73% (0x0205c2a2+0xea6), s1u_comp 6.15% (0x0205c1f2+0x94), master_process_st 4.32%, xl_tick 3.60% (0x0205c13a+0xb8), s1u_ladder 3.46%, plat_cpu2_wait 3.45%, s1u_lfo_tick 2.63%, TIMER5 2.51%. Core1 d9_render 29.87% (0x0205ef6c+0x1a02), fx_reverb 4.70%, stage_wait 4.12% (0x0200ecb2+0x64), Miami voice 2.87%. Voice_run_kr still absent from leading active ranges. See last-image-tprof-summary.txt. No timing requalification in profile mode.

Concrete next ranking (none implemented):
1. S1 active exact FX microkernel: s1u_comp reads per-span s1u_cc and repeats 1.0f - a after choosing attack or release every sample across the enabled compressor paths. A small side-by-side test can compute the two complements once in s1u_comp_coef, leaving the original mul/add, detector, envelope, gain refresh and signed-zero state untouched across parameter changes. No persistent storage; verify stack/flash debt. This is the most isolated newly measured residual, but saving one arithmetic operation out of 6.15% has low likelihood of closing several ms alone. RVHP alone is smaller.
2. More potential is the 909 serial core1 window plus waits. d9_render in drum909.c contains active per-voice processing. Previous sampler, sparse and clap candidates lost physical parity or timing; do not repeat those. First attribute measured PCs to a different concrete loop or prove an ownership-safe render-before-mix schedule, then run a fixed-input voice/RNG/per-bus experiment and validate physical admission separately. Stage wait is a dependency symptom, not guaranteed spare parallel compute; core1 also owns Miami and tail FX.
3. Splitting S1 voices or changing LFO/sequence rates is still not justified for this specific measured workload. Physical UI-timestamp stability is separate.
