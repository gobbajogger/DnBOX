# Variant C follow-up: ownership gap closed; static sound gate still FAIL

The new focused API test passes for two differently configured active voices
processed independently versus interleaved, including irregular block groups,
asymmetric note-off and live physical-control edits. The completed C audio
screen remains **109/180 passing notes, 30/60 passing files, and overall FAIL**.
All 15 zero-resonance candidate files are byte-identical to frozen B. C removes
the large retained high-cutoff oscillation observed in B's stored worst case,
but the remaining level and spectral errors still exceed the unchanged gate.

No candidate, reference or metric run was repeated to produce this analysis.
The only newly executed DSP work was the separately requested bounded
ownership/block-partition API test. All follow-up files are in this new sibling
directory. Frozen A/B/C source, declarations and existing results are unchanged.

## Correction to the earlier ownership wording

The earlier C scalar test's **720 checks** established history-shift behavior,
finite selected substeps, and that a never-processed object remained untouched.
Its `memcmp` check did **not** interleave two active owners or compare their
independent execution with their interleaved execution. The earlier phrase
"history-shift/ownership checks" must therefore be read narrowly as
**history-shift checks plus untouched-object checks**. It did not close the
dynamic ownership or block-partition gap.

Those original files and claims remain preserved as historical evidence. This
additive correction and the focused test below supply the missing evidence;
they do not reinterpret a weak test as stronger than it was.

## One new focused test

`tests/ownership_test.c` links directly against frozen C's source and unchanged
vendor envelope, without copying or changing its algorithm. Two logical owners
have different frequencies, velocities, oscillator phases, component gains,
amplitude envelopes, filter envelopes, initial cutoff, resonance and depth.

Each owner runs 4096 frames independently using single-frame calls. The same
owner configuration then runs 4096 frames interleaved with the other owner,
using repeating requested groups of **1, 17 and 64 frames**, split at scheduled
events. The interleave order also varies; it is not a single shared block call.
Thus each logical owner has **8192 evaluated frames total** across the two
execution orders, within the requested bound.

Owner A has four live physical-control edits and note-off at frame 2049. Owner
B has five edits and note-off at frame 3071. Controls include cutoff endpoints
64 and 22050 Hz, resonance 0 through 0.98, depth **1.2**, amplitude/filter ADSR
changes, and component level/balance changes. Edits continue after note-off.
The same numbered-frame ordering applies to both execution orders.

The independent runs save output words and full state at every endpoint of the
planned irregular blocks. Every interleaved block checks its output words and
complete **160-byte state** against that owner’s independent baseline. The
other owner's complete state and control object must remain byte-identical
before and after each processing call.

| Check | Actual result |
| --- | ---: |
| Independent versus interleaved sample words | 8192/8192 identical |
| Finite output checks across both execution orders | 16384/16384 |
| Full 160-byte state checkpoint comparisons | 310/310 identical |
| Untouched-other-owner checks | 310/310 unchanged |
| Interleaved blocks for owners A/B | 156 / 154 |
| Live physical-control edits for owners A/B | 4 / 5 |
| Note-off frames for owners A/B | 2049 / 3071 |
| Evaluated frames per logical owner, both execution orders combined | 8192 |

The host compilation and test both exited 0 on attempt 01. The test wrote no
audio files and read no reference PCM. Its output arrays existed only in the
bounded host-test workspace. This is candidate API invariance evidence; it
does not qualify the original instrument's dynamic behavior or the missing
72 dynamic-filter audio cases.

Evidence is in `TEST-DECLARATION.json`, `RESULT.json`,
`logs/build-attempt01.json`, and `logs/test-attempt01.json`. They preserve the
exact source, binary and compiler identities, commands, statuses and output.
The test source SHA256 is
`bf7bba35b180e3291146b7267ccd4c16ec3286126c64478ad73ee72cfa81450e`.
The linked C implementation remains
`29e04eb07c512bb390bf6f7cc7b9429baf46cf927db126bced2f1fffe1ae1bf9`.

## Completed static audio screen

The root's one authorized portable-checker run completed from **00:03:44 to
00:04:10 UTC on 2026-11-19**, exiting **1** for normal audio-gate failure. This
follow-up reads that completed `audio180/SUMMARY.json` and its saved outputs.
All 60 candidate, paired-PCM and metric-report hashes were checked against the
saved receipts before aggregation. No FFT or acceptance metric was rerun.

The gate remains comparison gain **1**, RMS error **at most 1 dB**, spectral
error **at most 3 dB**, and **zero nonfinite** samples. The unchanged metric
SHA256 is `bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1`.

| Variant | Passing notes | Passing files | Maximum RMS error | Maximum spectral error | Nonfinite |
| --- | ---: | ---: | ---: | ---: | ---: |
| Frozen B | 109/180 | 30/60 | 45.163240 dB | 15.629318 dB | 0 |
| Frozen C | 109/180 | 30/60 | **7.691473 dB** | **8.959005 dB** | 0 |

The two variants have the **same set of passing notes**: no new C passes and
no lost B passes. The reduced maxima are real changes in the saved metrics,
but they do not turn a failing case into an accepted one.

### By resonance

| Resonance | Passing notes | Maximum RMS error | Maximum spectral error |
| ---: | ---: | ---: | ---: |
| 0 | 45/45 | 0.313897 dB | 2.283731 dB |
| 0.25 | 32/45 | 2.233378 dB | 2.488333 dB |
| 0.5 | 18/45 | 2.418009 dB | 4.571931 dB |
| 0.9 | 14/45 | 7.691473 dB | 8.959005 dB |

At resonance 0.25, all spectral results are within the limit but 13 notes still
fail the RMS condition. Resolving the delayed-feedback instability has therefore
not established the required level-dependent transfer.

### By velocity

| MIDI velocity | Passing notes | Maximum RMS error | Maximum spectral error |
| ---: | ---: | ---: | ---: |
| 20 | 46/60 | 4.649818 dB | 8.959005 dB |
| 100 | 37/60 | 6.314122 dB | 8.387221 dB |
| 127 | 26/60 | 7.691473 dB | 8.062446 dB |

### By cutoff

| Cutoff Hz | Passing notes | Maximum RMS error | Maximum spectral error |
| ---: | ---: | ---: | ---: |
| 64 | 29/36 | 7.691473 dB | 5.093117 dB |
| 500 | 29/36 | 2.446291 dB | 5.517961 dB |
| 2000 | 24/36 | 2.441217 dB | 3.838778 dB |
| 8000 | 12/36 | 4.649818 dB | 4.509559 dB |
| 22050 | 15/36 | 1.244681 dB | 8.959005 dB |

The maximum RMS error is `physical-gain/127_64_.9`, note 36:
**7.691473258364919 dB**. The maximum spectral error is
`physical-gain/20_22050_.9`, note 84: **8.959004990571875 dB**. The latter has
RMS error **1.244681417522504 dB**, also above its limit.

`reports/C-STRATIFIED.json` contains all these groups and the combined velocity
by cutoff matrix for resonance 0 versus 0.9.

## Exact zero-resonance preservation

All **15** saved zero-resonance C candidate PCM files were hashed and compared
with the corresponding frozen B files and historical receipts. **All 15 hashes
match**, covering **45 notes**. Each file has 1058304 bytes. This establishes
whole-record sample identity for the supplied zero-resonance grid, beyond the
earlier selected scalar substep check.

`reports/ZERO-RESONANCE-IDENTITY.json` preserves both hashes for every file.
The comparison used saved PCM only; no file was regenerated.

## Saved tail diagnostics

These diagnostics examine the final 4096 frames, `[84096,88192)`, corresponding
to approximately **0.407800–0.500680 seconds after note-off**. They are separate
descriptive window levels, not a replacement or repetition of the full-record
acceptance metric.

The stored reference has exactly zero samples in this window in all 180 cases.
That observation does not establish a general voice-retirement rule, and no
candidate tail truncation was introduced.

For B's former high-cutoff oscillation case, `physical-gain/20_22050_.9`:

| Note | B candidate tail RMS | C candidate tail RMS | C tail peak |
| ---: | ---: | ---: | ---: |
| 36 | 0.2164390213 | 0.000000212854 | 0.000001739421 |
| 60 | 0.2164442042 | 0.000000244814 | 0.000001893942 |
| 84 | 0.2164642149 | 0.000000341606 | 0.000001987792 |

The large retained oscillation present in B is absent in these saved C outputs.
This is consistent with the independent delay-instability diagnosis and its
centered-feedback correction. It is not a proof of all nonlinear stability or
original-engine fidelity; the same note-84 case still has the worst spectral
failure in the static gate.

Across all 180 C notes, the largest tail RMS and largest tail peak both occur
at `physical-gain/127_64_.9`, note 36:

- C tail RMS: **0.00016594606019310176**.
- C tail peak: **0.0002870658354368061**.
- B tail RMS for the same stored case: **0.00016600801219447854**.
- Recorded reference tail RMS and peak: **0**.

The low-cutoff residual is therefore nearly unchanged by C. The exact saved
rows for every C tail, and selected B/C comparisons, are in
`reports/C-TAIL-DIAGNOSTICS.json`.

## Scope of the completed follow-up

The concrete dynamic ownership/block-partition gap is closed for the declared
test timeline. The static comparison now has a verified whole-PCM r=0 identity
and evidence that C removes B's large high-cutoff retained oscillation. The
remaining static transfer fails its unchanged gate. No new filter variant,
production source change, native build, timing benchmark, original-engine
execution or repeated 180-note/metric run occurred in this follow-up.

The missing 72 dynamic audio cases, all 694 intact presets, full parameter and
FX behavior, complete note lifecycle, native fit and callback deadline remain
unqualified. Nothing in this report admits C to production.
