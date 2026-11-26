# Eight phase accumulators at exact heavy-event boundaries
Matched same table-share S1 A/arp + Chicago B diagnostic, exact unchanged
922-entry event tape and all firmware effect/control jobs present. Instrument
only engine_render and half exit, no broad memory dump beyond established
capture. Adds 76 B pool, guarded +488 over pool floor and 564/572 B APP free.
No hardware writes. Both disposable NORs compare exactly before/after,
each board has 4 settled owner readbacks and 319,488 PCM frames, no skips
or worker errors. Instrument affects exact trajectory and is not a gate.

| Fresh measured worst half | Elite µs | Pro µs |
|---|---:|---:|
| Whole timestamp | 13,021 | 13,191 |
| Initial plus per-span owner prepare | 178.5 | 177.9 |
| Seq advance and pending BREAK MIDI drain after prepare | 512.2 | 569.7 |
| Serial output master remainder and stopping-state logic | 542.1 | 521.5 |
| Poll/select span/arp boundary up to renderer | 59.8 | 56.4 |
| Frame-zero queues and transport | 130.6 | 146.7 |
| Output conversion/scope/meters | 79.3 | 95.8 |

Trace sees 36 late Elite, 59 late Pro. Job submission/admission is inside
render_sub and separately measured in v10 (28–33 µs from span enter to
dispatched, including more than plan choice). Current spans/half ordinals
differ from v10 where the 1277/750 µs observed specifically between spans
excluded first-frame and end-of-block work; this table separately attributes
all eight windows across a complete worst half. Do not subtract rows across
the two independently instrumented worst sample phases.

A cache cannot recover the full gap: both prepare windows together are below
179 µs before any nonremovable work, and even all plan admission costs below
100 µs across the observed three spans. Prepare runs before notes for owner
admission; Chicago reinitializes its block-prepared flag on each rendered
span. Synth One arp_params derives cfg/dynamic tuning and explicitly clears
the clocked regime, so cannot skip wholesale on unchanged BPM. Late sequencer
window dominates because it performs real step state, motion, note/ARP and
MIDI handling; changing engine event appointments is outside bit-exact scope.
A serial short prefix in advance of subsequent render could enable early
shared FX+master pipelining, but the original remainder is deliberately
computed after all in-half events: master_key can alter pump target,
motion_step can edit parameters, and master_process_st decays gr_view once per
call (so mere call splitting differs even with identical PCM controls).
No bit-exact move of ≥0.5 ms proved on either board; no candidate applied.
