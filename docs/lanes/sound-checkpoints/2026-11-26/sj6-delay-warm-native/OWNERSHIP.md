# Exact notification and render ownership
All source paths are relative to the inspected all-controls-source-join/tree.
The initial CLI --bin path failed before bench entry at ROM PC0x13b00; no
results quoted from it. Measurements use a pinned provisioned stock emulator
boot envelope with new disposable NOR; flash untouched across the run.
No production firmware or hardware was written.

* firmware/src/app/original_presets.inc:2286–2307: permitted physical or
  shared synth parameter setter only changes the four project bytes. Its
  plat_cq_lock implementation (plat_fm1.c:10–16) masks ALNK0 on CPU0; it
  does not move DSP work to the UI or hold the IRQ for this synchronous
  coarsen.
* firmware/src/app/main_fm1.c:38–64 starts TIMER4, renders a 512-frame half as
  two 256-frame calls and measures all work within the half. Its active
  render context is the ALNK0 consumer.
* firmware/src/app/engine.c:2496–2534 prepares all NBASS owners, calls
  sj6_typed_delay_apply which configures the actual typed delay on its
  individually owned borrowed arena (original_presets.inc:1974–1986).
  The calls are inside engine_render at 3605 and 3653 (after each render
  stretch and before the next sequencer event). If both Juno slots use
  synced time, a single tempo edge can require both conversions in the
  same timed interrupt. The coarsen sits outside render_sub/core1 scheduling.
* engine.c:3423–3471 dispatches DSP jobs to whichever core, then waits and
  recovers failed jobs before returning. Old history/ingress normally do
  not race the existing synchronous configure. Pushing work to background
  without a real publication scheme would introduce this race.
* original_runtime.c:1455–1457 consumes the configured delay during the
  Juno part render. No guaranteed additional usable history arena; no
  assumption that prior Chicago/reverb/BREAK or second-owner RAM is free.

Current in-place rewrite relies on draining newest groups then publishing
locations behind the already-read source locations, with no intervening tap
reads or producer writes. coexist.py exhibits two deterministic numerical
counterexamples to just slicing those writes while retaining the live old
format: even one two-group prefix can overwrite a protected old tap at an
eligible low current; and a still-full old ring has no free ingress cell at
the old oldest boundary. A fixed anchor gets stale; using the current cursor
on both passes causes two inconsistent group epochs. This is not a proof
that every zero-extra-history algorithm is impossible. To make one exact:
freeze existing old delay mode/tap trajectory within a formally admitted
future read/write footprint; prove all converted destinations stay out of
the old tap and ingress footprints until commit; forecast final epoch,
handle oldest evictions, and finish any conflicting read/write groups at
a coarse boundary before one publication. Retargeting cannot abort after
destructive staging; receive further edits only after commit (retaining
latest publication separately). No such schedule is implemented or assumed
universally admissible. Atomic compare against a known existing raw or
model-specific stress harness still required; no new storage/clock gates.
