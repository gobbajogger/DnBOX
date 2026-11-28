# Apply the independent exact changes together (still not a timing pass)

Continue the diagnostic-only recipe in `six-loaded-preferred-start.md`. In this
order, apply `osc-unfiltered-path.patch`, `break-derived-offsets.patch`,
`crc-shared.patch`, `editor-word.patch`, `909-render-local-state.patch` and
`808-shaper-zero.patch`. Use the same six-note workload, original cymbal and
same strict emulator as the previous preferred stack. New C code is not
enabled in the production tree. Do not install these builds.

Paired native links: Elite 3,188 B and Pro 3,196 B free app flash, each
27,020 B in the audio pool after the unchanged 8,192 B reserve. This measures
the stack, not the sum of isolated branches' linker deltas.

Elite timed candidate: 14,035.125 µs held peak (previously 14,311.96 µs),
15,312 µs whole-run peak (was 15,460), 473 processed halves, 151 late halves
and 151 XRUNs. All 128 measured held halves still missed both the 10,809 µs
engineering target and 11,609.98 µs physical audio period, with six actual
voices, 132 actual hits per kit, no skipped emulation cycles. Only Elite was
timed under this combined candidate; Pro linked but was not timed.

Individual 909 local RNG/IMA state result: 14,290.46 µs / 151 XRUNs and
96 B more XIP headroom, compared with the original 14,311.96. Native
per-voice profile: per 512 held frames, all voices plus their mixer emits
4.930 ms; three toms 1.784 ms, snare 0.732, kick 0.483, clap 0.420, rim 0.336,
sampled foursome 1.142, shared noise 0.034. The per-sample alias and pitch
cache arms were slower. Do not repeat the ADPCM lookup or assume independent
small speedups combine linearly.

808 zero pulse-shaper exact fast path by itself: 14,295.54 µs / 151 XRUNs,
32 B more XIP. Host comparison verified the output and state over 10 million
samples. The fused bank edge rewrite, explicit mixer right pointer/index, and
settled metal tune cache were all slower natively.

The optional `bus-reverb-unroll-flash-only.patch` frees 80 B linked XIP on
each model with exact randomized host bus and line state, but its isolated
held peak was 14,344.88 µs. It is **not** in the preferred timing candidate.
The master limiter-local rewrite was also slower and is not recommended.

The scope stays six dry Synth One voices with two active kits, not wet effects,
matching all presets, or the full real-world workload. No release, hardware
writes, installer, engine counts, feature/quality reductions, or SuperJ work.

## Where to look next

A separate stratified instrumented run of this stack sampled 98,352 active
Synth One voice-samples in the held window; 12,312 did control updates (one
in eight as expected). Approximate time per 512 frames across six voices:
oscillators/noise 3.38 ms, ADSR 2.47 ms, filter/mix 1.34 ms, control 0.69 ms.
Those buckets include sampling overhead and do not sum to total callback
time. Start with oscillator/envelope state locality and the 909's analog
voice renderers, not a third MIDI scheduler change or output-clamp tweaks.
Apply `s1-voice-phase-profile.patch` after the stack and use
`read-s1-voice-phases.py` on a separate diagnostic run.

Keep `s1-stable-voice-bulk-trial.patch` out of the preferred composition.
It changes only the dry diagnostic when all relevant input values are
stationary and there is no shared RNG or LFO feed into voices. The strict
test entered this path for 169,602 generated frames across the whole run.
At 13,978.54 µs held, 15,251 µs whole-run and 150 XRUNs, all 128 held still
missed both targets. It costs 624 B app flash for a 56.58 µs held reduction
and has not received the independent pre-DMA proof. Next, improve local
oscillator/envelope state reuse within a voice; this trial only changed
loop order and cached shared calculations, not the core cost of each tick.

A separate pre-DMA validation of the complete composition preserves all
160 ordered audio fingerprints against the preferred baseline with genuine
two-kit hits. The uninstrumented numbers in the table remain the valid
performance measurement.

Native PC sampling with the same uninstrumented Elite package and
`FM1_TPROF=pcs.csv,3200,4100` independently agrees on the most expensive
Synth One sites. Out of 81,000,332 Core 0 sampled cycles over that wall
window: `voice_run_kr` 12,890,008 (15.91%), `morph_step` 6,101,303 (7.53%),
and `s1u_env_step` 5,193,042 (6.41%). These are wall-window shares and
include UI running between audio callbacks, not a function of one ISR.
Several UI raster functions were also substantial (pixel blend 7.34%,
tile hash 5.37%, fills 5.16%). On Core 1, `d9_render` 17.95% and
`cy_run` 6.41%, with a large unmapped idle region. Do not normalize
them to active DSP without first identifying that region.
