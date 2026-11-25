# Combined guarded native fit increment, no runtime

Exact restored full control source, not the previous narrow DNR3 link.
Applied the measured same-semantics UI page/setter/undo batch, paired
Chicago FAST32 layout with three previously pinned immutable tables, and
the exact RAM patch. Both link attempts still reject XIP; neither binary
can be installed or run. No PSRAM, feature cuts or increased hard limits.

| Source/profile | Elite XIP over | Pro XIP over | Pool free, each |
| --- | ---: | ---: | ---: |
| Original guarded composition | 35,536 B | 35,528 B | 5,616 B |
| UI page/setter/undo sharing | 27,932 B | 27,924 B | 5,616 B |
| UI + FAST32 + three pinned cold tables | 27,140 B | 27,132 B | 5,616 B |
| UI + FAST32 + RAM recovery | 28,588 B | 28,580 B | 8,224 B |

Both final maps: pool 0x51fe0, 32 B above the mandatory 8 KiB reserve;
ordinary BSS free 172 B. RAM recovered 2,608 B but cost 1,448 B APP.
The exact archive generator reproduced all 32,770 Chicago A/E source
words before the guarded link. .lab_slot3 0x6c80, .lab_fittab 0xbef,
.lab_fdata 0xef90. That storage remains non-installable diagnostic paired
layout, not authorization to use or overwrite users' existing content.
Because links still reject, not all paired sealing/boot assertions execute.

Replay: fresh isolated guarded-fit tree, apply ui-fit-outline.patch,
archived chicago-filter/native/fast32.patch and the exact RAM patch in
that order. Regenerate private p_a.h/p_e.h using the archived FAST32
generator, copy both model headers, then make-fast32-wrapper.py against
the guarded wrapper. All data and fit evidence are offline. No runtime
qualification, behavior tests, playback or speed claim.

Follow-up shared attack/ADSR, original MIDI/USB cold outlines, same gates,
and exact cutoff batch on the combined UI/FAST32/RAM source:
Elite +27,948 B, Pro +27,940 B, both still 8,224 B free POOL.
The 909 buffer change advances the same sample sequence but its periodic
history copy occurs more often for full 256-frame spans. Future timing
qualification must cover quiet advance/rollover, quiet-to-active retrigger
and active 909 spans on both boards, with exact old PCM comparison.
This is a potential timing cost, not established performance regression.
