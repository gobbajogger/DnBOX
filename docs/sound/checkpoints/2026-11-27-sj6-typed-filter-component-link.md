# SuperJ6 components physically linked; not audio-path activation

Using the native pair that physically contains two independent 6×52 B Sub histories plus latch storage and still preserves the allocator floor, linked actual two-word g/k mapper and the complete current nonlinear test leaf together with both engine configurations. The linker roots bounded_g, bounded_k and physical_replay as code-footprint proof only, not live DSP ingress. Both guards pass.

| Board | Free XIP | Audio pool usable | Headroom beyond mandatory 512 B |
|---|---:|---:|---:|
| Elite | 2900 B | 584 B | 72 B |
| Pro | 2912 B | 584 B | 72 B |

2530–3000 B of mapper text+data before actual link; test-leaf and mapper combined physical APP increase 3120 B. Standalone same PI32 grid showed exact 3072/3072 and 2.84333 µs per pair with no external double math helpers, versus 140.73188 µs and 5060 B closure of the binary64 route. Four exact scoped source cues preserved the mapped waveform; broad g discrepancies from reference host libm remain (540/1000000; 37 in high mode). Do not infer all-continuous-setting validity or end-to-end original sound. Coefficient-rate cost is not filter/render CPU cost. No active nonlinear calls or all-source/control memory and no production fit, release or flash actions from this proof.

ELFs Elite e4c9af5e88445eeca286eac84b95babe2f5e470f8cd3e8873ab5babede85d528; Pro 46967bc93a90800ed359fde6bfea42339f82515bd3bd26ca06e146b0f13789da. Rebuild on native-widen-sj6-independent-storage guarded donor, append mapper and filter units to the original synth compile list, and retain those three entry symbols for the link-only experiment. Source-only mapper archive SHA256 f4aac17941db34f139e318cb1bceec84a6df70542501a05fe426b39d52 (see full hash in Library receipt); its files contain no original executable or audio. Independent Main/Sub typed source adapter is still being prepared for a scoped target cue, with no guessing of installed mix gains.

## Independent populated S1 companion proof

Retain the fully voiced original favorable tape and its 5 owner reads. A companion fixture changes only nine typed A settings bytes in the synthetic project before replay, keeps the exact 922-event tape plus real six-channel-2 note edges, and checks loaded/effective controls: audible Sub, distinct morphs, +7 semi, and a routed LFO of ~0.278 Hz at ~.756 depth. No virtual changes during either new run. Both morph and fixed-pitch guards actually skipped zero work across 422882/423950 voiced samples and 52870/53004 KR ticks. The new workload still fails: 15087 µs / 90 XRUN Elite and 15912 µs / 81 Pro. This defines an important additional active regime; do not generalize the favorable guard wins. A patched semitone lookup in a different code layout produced 15993/90 and 15903/91, no consistent timing win and not one-byte attribution. No adoption.
