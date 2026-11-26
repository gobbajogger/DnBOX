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
  native timing remains red in an exact matched heavy replay.
  Elite: 15,840 → 15,558 µs and 105 → 39 underruns; Pro: 15,801 →
  15,793 µs and 108 → 53 underruns. There were no skipped cycles, no
  NOR writes and no XRUNs on separate cold boots. Both fail the strict
  10,809 µs, zero-XRUN deadline. Scope: isolated diagnostic, not release.
  The same right-history layout with BF16 also links at 72/84 B XIP.
  Its wider exponent avoids the proven low-tail half-format floor.
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

### Extended qualification: BF16, paired RevSC and Home strip
- BF16 now has per-preset 906-program host storage-control receipts. It is not a valid native timing improvement: both strict native boards showed more dropouts even with a lower observed peak. The observer refused the diagnostic protected overlay before replay, so the failure is not attributed to any particular stage. See `synthone/widen/native-heavy/bf16/BF16.md`.
- Full-rate paired reverb preserves the original time-base. A 37,152 B history meets the isolated Organ return's spectral threshold, but still exceeds available memory. See `synthone/reverb/paired-history/`.
- The actual default Home 80+58-row host proof is bit-identical with a stable snapshot and catches intentional mid-frame state tearing. It contributes no target RAM budget yet. See `firmware/home-strip/`.
- These changes are reproducibility material only; they do not select a shipping candidate, enable code in production, change a release gate, or publish firmware.

### Timing attribution and isolated exact-kernel trials
The sampled Elite replay is not observationally neutral. Matched strict replays of two independently tested numerical simplifications also lowered peak but increased underruns. Do not carry either into a release by looking at maximum block time alone. The ordered Core 1 join model and host test are recorded as a design lead only; it is not connected to firmware and does not prove hardware drain acknowledgment.

### Sound-first host parity
User directed complete sound matching before fitting. The current host Synth One candidate runs original unrestricted delay, room and widening from actual DnBOX voices; the named Pad and Organ are materially closer, not declared all-bank parity. SuperJ6 Main/Sub output evidence points to shared routing or gain, not a Main-only fix. The recovery folder preserves prior timing work without resuming timing or fit.
