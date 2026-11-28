# Staged framebuffer explains foreground cost, not the heavy audio miss

Source: the e00f689d donor assembled using recorded lazy-both-kits
checkpoint 662013f85efb0be9a4dac30623bd488a6908d0d2. Reference heavy Pro:
16,047 µs peak, 50 parity-late halves and 49 nonzero stale spans.
No source/package deployment/default change.

Core 0 ALNK0 is hardware IRQ priority 3. TIMER5 (normal boot) priority 4
can nest into it; display capture is the foreground. The LZ memory source
and sink do not mask ALNK or service foreground input. LCD waits stay
outside the bounded pack/unpack measurement. Existing LCD callbacks
between transfer items are untouched. The timing witness uses one 208 B
record in the already reserved NOINIT area; viewport, pool, ordinary RAM
and DSP remain unchanged. The fixture adds timestamp work and changes
addresses, so its audio peak is not a new canonical baseline.

On strict native Pro emulation at 360 MHz, same saved two-kits project,
six Synth One B notes held, 5,300 ms cutoff, zero skipped cycles/NOR writes:
- 67 pack + 67 unpack started; ALNK source enabled at every sampled start.
- ALNK entered 90 times while pack was running and 19 during unpack.
- When preceding ISR took <10.61 ms, next-arrival maximum intervals were
  11,611.25 µs during pack (68 samples), 11,606.54 µs during unpack
  (16 samples), and 11,618.63 µs elsewhere (198 samples).
  Ideal period is 11,609.977 µs. Difference of adjacent entries is not
  absolute hardware IRQ assertion latency; it detects a stage-scale delay.
- Last six-note loaded epoch: 22 other, 13 pack, 2 unpack completed
  IRQs, all late; the largest ISR was 16,425.125 µs outside the codec.
  Whole trace still 50 late halves, same count as the uninstrumented
  selected trial.
- Pack average/max wall 18.75/64.72 ms. Subtract entry-to-exit ALNK spans
  and it is 7.41/7.65 ms (this still includes other preemption, e.g. TIMER5).
  Unpack wall 3.96/15.32 ms; similarly 1.69/1.75 ms after removing ALNK.
- Independent first trace had 90 pack and 20 unpack preemptions, same
  50 parity-late, and same 16.425 ms worst IRQ elsewhere.

Do not interpret aggregate foreground stage wall time as a blocked
audio interrupt. Direct pack/unpack deferral is not supported here; any
cache, memory-arbitration or alignment effect is unisolated. No changes
to display algorithms proposed as an audio fix. Foreground cost is real,
and would need separate refresh and input latency work before release.

Reproduce in a disposable lazy-overlap source: apply stage-irq-profile.patch,
add -DDNBOX_STAGE_IRQ=1 to existing native lazy recipe. Build/seed Pro and
run replay-kits.sh with the same project/Pro controller and
MIDI='4400:923058;4403:923458;4406:923758;4409:923b58;4412:923e58;4415:924358'.
Decode with read-stage-irq.py ELF /path/to/virtual-replay [PI32 nm path].
