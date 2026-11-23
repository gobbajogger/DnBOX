# Callback admission contract for a narrow stock adapter

**The stock native runtime provides finite, source-backed ingress limits. It does not make the selected 32-note / 436-store probe a callback worst case.** Preserve these limits when implementing an adapter, and account separately for the expansions below. This is a read-only source audit of `76040daa5b52d686a8b7b544296ba5d79610a95d`; no build, benchmark, device test or production change was performed. The generic `dnbx_core` API remains non-production.

## Native boundary and ingress ceilings

`main_fm1.c::render_block` renders one **512-frame half at 44,100 Hz**, as **two `engine_render(...,256)` calls**. The physical deadline is 11.609977 ms; the existing integer-microsecond accounting uses 11,609 us. `x0x_dbg.in_audio` stays set across both calls. TIMER5 continues input scanning and optional UAC service, but defers **USB MIDI, TRS MIDI and BLE polling until outside this entire half**. [Sources A, B]

| Path | Actual capacity / drain rule | Maximum consumed per native half |
|---|---|---:|
| Main-loop command queue | `CQ=1024`; `drain()` has `budget=128` per engine call. Holes and discarded entries also consume budget. | **256 queue slots** |
| USB/TRS/BLE MIDI input | `MQ=64`; `drain()` has `guard=64` per engine call. `plat_midi_in_epoch` consumes one entry atomically against TIMER5. | **64 packets**, because this ring cannot refill while `in_audio` is set. Without that producer contract, only the explicit **128-per-half** drain ceiling follows. |
| Physical Free Play | `LIVE_KEY_Q=32` uses an empty slot, so 31 entries are usable. `live_key_drain()` loops until empty without an iteration guard. | **31 entries**, conditional on the existing main/UI-only producer being preempted for the whole half. |

Thus **351 queued descriptors per half** is a defensible aggregate ingress ceiling for the current native producer arrangement. It is not a count of note calls, envelope transitions, coefficient updates, or all callback work. The sole physical-key caller is `ui.c::live_send`'s call to `engine_live_key`; the queue's platform lock masks ALNK0 on CPU0. Do not transfer the 31-entry proof to a future higher-priority ISR or CPU1 producer. [Sources B–D]

`cq_put` drops ordinary controls when full and uses bounded scans/compaction to prioritize other commands. Its concurrent `retry:` path has no fixed retry count, but it runs in the main-loop producer, outside the callback. It must not become an audio-side submission API. Full queues do not guarantee that every new note or release is accepted. [B: `cq_put`, `cq_replace`]

## Work outside those descriptor counts

| Path / symbols | Bound or remaining condition |
|---|---|
| `midi_in`, `original_runtime_release_source`, `live_key_drain` | One ordinary MIDI note can reach both bass parts on a shared channel. One physical-key replacement can perform an old note-off and a new note-on. A source release scans **128 keys**; link-end can release that source on both parts. CQ source-off, cancellation, channel reconciliation and panic are therefore fan-outs, not one DSP event each. |
| `notes_off_req`, `original_midi_context`, `brk_channel_reconcile`, live-key cancellation, wheel mailbox | Fixed flags / finite part or ownership scans, applied outside the CQ budget. Count their full cleanup cost. A mailbox coalesces changes; it does not create a queued-event allowance. |
| `seq_until_event`, `seq_advance` | Every render stretch advances at least one frame: **at most 512 stretches per half**. Each `seq_advance` has an **8-pass × 5-track** due-event loop. There are at most 8 other boundary calls per half plus at most 64 pre-note MIDI calls. A deliberately loose structural ceiling is therefore **584 `seq_advance` calls and 23,360 `fire_step` dispatches per half**. This counts dispatch opportunities, not actual notes or a useful time estimate. A step can apply motion, note locks, kit hits, pattern changes and bass events. |
| `seq_ext_clock`, `ext_ticks` | `ext_pending` is a `uint32_t`; the increment and drain have **no independent saturation / iteration cap**. In this native call graph, `seq_midi_rt` is its only incrementing caller, and completed audio boundaries consume pending ticks. Consequently at most **64 newly admitted clock ticks per half**, under the same MIDI-producer contract. A future direct clock producer invalidates that proof. |
| `motion_tick`, `motion_step`, `motion_release` | `NLANE=160`, `MOT_PARTS=5`. Tick/release scan 160 lanes; each sequenced step uses two 160-lane passes. Gesture gap filling is bounded by `MOT_GAP=4`. These parameter applications bypass CQ admission. |
| `note_lock_onset`, `note_lock_restore`, `note_lock_stop` | Each applicable onset/restore visits the active engine's finite `original_sound_count` and five mixer parameters, with a finite lane lookup. Extending the schema to 218 controls changes this cost; it must not silently retain the old budget assumption. |
| `runtime_stale_tick` | Pending stale-note timers scan **3 sources × 128 keys per part** when a 256-frame timer tick is reached; expiry can issue further note-offs without any queued descriptor. |
| `bass_preset_boundary_voice` | Main publishes a latest request. The audio side reads its seqlock once and defers on inconsistency, queue marks, ownership, locks, quiet-time or pattern gating; it does not spin. Once admitted, retirement, initialization, control application and preparation run **synchronously inside audio**. With the main producer suspended, at most one new request per part can be adopted in the half, although readiness checks may repeat at onsets. |
| `brk_midi_drain` | CPU1/CPU0 produce at most **8 accepted BREAK descriptors** before each serialized drain (`BRK_MIDI_EVENTS=8`). This outgoing queue can refill between render stretches; eight is not a whole-half ceiling. |

When the SMK37 SMC User-preset routing is enabled, `smc_midi_host` diverts an already admitted MIDI packet into a separate audio-to-main ring. It does not run the controller's entire control action inline; later main-loop engine commands still encounter CQ admission. [B; [smc_mixer.c](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/app/smc_mixer.c): `smc_midi_host`]

Sources: [B, E–H]. The 584-call arithmetic is `512 + 2×4 + 64`, from all six `seq_advance` call sites in `engine.c`; `fire_step` is called only by the guarded due-event loop. It assumes valid initialized sequencer/project state and the current producer arrangement, not arbitrary direct API mutation.

There are also waits without local iteration counts: CPU1's `body_wait` / metal-ready loops depend on the job handoff; CPU0's native wait uses a **24 ms fault timeout**, already longer than one audio deadline. TIMER5's elapsed-millisecond catch-up loop is time-dependent, and UAC may interrupt the render. These are not extra synth-admission allowances and prevent treating ingress arithmetic as a native WCET proof. [A, B, I]

## Conditions for the proposed implementation

1. **Retain the queue ownership and both drain limits.** Any new raw-control transport needs an explicit capacity, consumer budget, overload policy and ordering relative to notes, releases, panic and preset publication. The existing CQ format is `type:4 | a:8 | b:8 | value:12`; it cannot transport a raw binary32 control value unchanged.
2. **Separate validated control preparation from application.** The main-loop preset request/publication path is a useful staging point for schema validation, decoding and immutable coefficient preparation. A bounded audio-boundary commit must own its snapshot safely and preserve notes/tails and event order. Current `X0X_COLD` initialization is still synchronous; its attribute does not defer execution. Any staging storage needs a simultaneous RAM/pool ledger, not an assumed spare instance.
3. **Keep repeated prepare calls cheap and idempotent.** `original_prepare_all` runs before frame-zero edits and before subsequent sequencer events; each bass render job also calls `original_runtime_prepare`. Stock Juno's prepare is a no-op. Replacing it with a full 218-control or all-voice rebuild at every call would multiply work by stretch count. Bound dirty coefficient work explicitly, including motion/note-lock edits that originate in audio and cannot simply be delayed without changing behavior.
4. **Qualify the composed callback using adversarial admitted cases.** Include full CQ and MIDI drains, physical keys, source releases/panic, external-clock bursts, motion, note locks, preset adoption, other engines/FX, inter-core synchronization and UAC. Source bounds guide fixtures; the existing native `x0x_dbg` / `x0x_worst` maxima and late/xrun counters remain the relevant eventual target observations. No target WCET or pass is established here.

## Comparison with the selected probe

The saved probe sends **16 note-ons + 16 note-offs**, **436 raw stores** (218 × two parts), four layer updates, four sustain changes and two velocity-mode changes. Its 32 note calls are below the CQ ceiling alone; 64 ordinary MIDI note packets could also produce 128 part-level note calls. Source-release and internally generated work add further cases. [J]

The 436 stores are direct generic-API operations, not current stock command packets. If a future transport used one CQ slot per store, 436 would exceed the 256-slot half budget and span further boundaries; the current 12-bit payload is also insufficient. Conversely, 436 is not an upper bound on all coefficient/control work because sequenced locks, motion and adoption bypass that queue. The probe is a useful selected engine/control workload, **not the stock admission envelope or a full-callback worst case**.

## Exact evidence

All stock paths below are from the verified `76040daa5b52d686a8b7b544296ba5d79610a95d` snapshot; [native-baseline-source-manifest.json](native-baseline-source-manifest.json) records its source closure.

- **A:** [main_fm1.c](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/app/main_fm1.c): `render_block`, `fm1_timer5_irq`, `fm1_alnk0_irq`.
- **B:** [engine.c](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/app/engine.c): queue definitions, `cq_put`, `drain`, `live_key_drain`, `midi_in`, `engine_render`, `original_prepare_all`, `brk_midi_drain`, `body_wait`.
- **C:** [usb.c](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/usb.c) and [plat_fm1.c](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/app/plat_fm1.c): `MQ`, `midi_in_put`, `plat_midi_in_epoch`, `plat_cq_lock`.
- **D:** [ui.c](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/app/ui.c): the physical-key `engine_live_key` call.
- **E:** [sequencer.c](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/seq/sequencer.c): `seq_until_event`, `seq_advance`, `ext_ticks`, `seq_ext_clock`, `fire_step`.
- **F:** [motion.c](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/seq/motion.c) and [motion.h](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/seq/motion.h): lane and step bounds.
- **G:** [original_presets.inc](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/app/original_presets.inc): publication, readiness, adoption and note locks.
- **H:** [original_runtime.c](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/src/app/original_runtime.c): initialization, preparation, source release and stale timers.
- **I:** [fm1_cpu1.h](https://github.com/gobbajogger/DnBOX/blob/76040daa5b52d686a8b7b544296ba5d79610a95d/firmware/hal/fm1_cpu1.h): `fm1_cpu1_wait`.
- **J:** [selected callback probe receipt](../candidate_review/callback_audit/RESULT.json): `burst_per_selected_callback`; candidate C SHA-256 `9cdcea0b90b691d05cd07306433b9b4cf42c023bf419a251d7ff436fa13487fa`.
