# Sound parity checkpoint (2026-11-26)

Working evidence and patches. Not a release, production integration, or gate pass.
No patch is enabled merely by adding these files to the sound branch.

## Synth One

- Dark Theater Organ's original widener is a 2,205-frame right-only delay
  at 44.1 kHz. With the same full original room return, exact Float32 history
  improved late right residual from +3.2614 to −42.5224 dBr. Left, all room
  traces and width-off behavior are unchanged. The in-engine BFP8 replay
  uses 2,424 bytes and reaches −41.0168 dBr.
- The simpler half-history target patch reuses shipping h16 helpers. It
  leaves 72 B XIP on Elite, 84 B on Pro, and 564 B of usable pool above
  the 8 KiB reserve on each in the guarded diagnostic link. The tested
  engine fits the existing backend union. The separate BFP attempt
  overflowed by 228/216 B. The independently compiled in-engine half-history
  replay preserves all 264,600 left samples and the entire room trace; the
  right last-second residual is −42.5144 dBr against the original. Both-board
  native timing and audio qualification are still outstanding.
- A general rate-then-bit-depth selector reduces Mushroom's right
  spectral error from 4.14 to 2.65 dB (bar: 3 dB) without preset edits.
  Hall Drone is cadence-limited and Deeper Space Pad precision-limited.
  Both are still red.
- Reverb modulation alone does not fix the room. The 16-bit full-rate
  original network meets the isolated 3 dB spectral bar but takes 49.4 KB;
  /2 still misses waveform parity. Neither is a fitting implementation.
- The bank script rebuilds the 906 archived programs with 130,464 exact
  C-decoded parameter words. The prior 907 run included one stock row and
  was not repeated against these new variants.

## SuperJ6

- Two Main source owners commit at authentic sample 49875 under 1,164
  recorded callback spans, invariant to compact subblock division. The
  staged Normal 4× patch retains filter history, separates filter and amp
  clocks, and models the tested original control laws.
- The authentic generated saw scales after halfband, before Normal filter.
  The two-arm test changes only this placement; both variants pass through
  actual Apple HighPass and Crusher and the audited settled suffix. The
  change does not address the remaining 12.1 dB held Main gap.
- The guarded host core and LFO patches require the recovered donor
  (dnbx_core.c SHA d58f87a1f598fb894a184ccd57f543a1ec451ee13aea020b6eec9e92ca0da625).
  Do not assume the current lane matches that donor byte-for-byte. The
  added fixture is for timing and ADSR verification, not a production table.

This checkpoint tracks source, replay inputs that are control/timing data,
and measured results. The supplied original AU binary is not committed.
