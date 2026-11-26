# Guarded replay post-mortem, no new emulation
Run read.py against archived exact table-share pair. Source/ELF/SRAM and tape
pinned; raw xrun and nominal elapsed time are different tests. Firmware
main_fm1.c compares hardware DMA half at entry/after render to count actual
xruns; aggregate max_us cannot identify which half crossed which edge.

Stock-equivalent receipts contain 44/60 actual misses, with 56,372/55,337
nested TIMER5 entries over the whole observed run. No per-half timing cost,
first missed half or per-half class retained. Exact symbol table omits
x0x_worst, dbg_tmr, eng_dbg_nsub. The older diagnostic records only the
slowest ordinal in its different exact trace ELF, not a ring of late
ordinals. Known worst class: all three spans mask 0x04 (A/S1 Core1 after
already-split 808 prefix; Core0 909 + Chicago B and small remaining jobs).
Both measured critical intervals remain >11,319/11,870 µs under an
unrealizable fully fractional pool balance with current tails. No safe
schedule-only change established with this evidence.

Source CPU interrupt exclusion: USB MIDI/TRS poll and BLE service execute
only when !in_audio. The nested ISR must retain the fixed physical key/pad
scan, ADC/latch/shift progression. Nested UAC only has active EP4 work
when its two alt settings are enabled; it already bails before SIE calls
otherwise. Thus no measured nested BLE culprit, no case for skipping
the transport, pad scan or audio-alt guard. Counter rates/aggregate
USB transaction counts cannot supply per-miss attribution.

Minimum next evidence is read-only per-half records keyed to produced
audio ordinal at ALNK entry/exit: DMA half change + exclusive nested
TIMER5 ticks, core0/core1 per-stage work, solver mask/scene readiness,
plus full producer-frame consumption witness pending exact runner source.
Do not patch or substitute the pinned release runner; no new run or
transport/dropout waiver.
