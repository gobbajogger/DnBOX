# Independent review of the clean DnBOX candidate

## Decision and scope

**The final candidate passes the declared captured-amplitude subset and the tested candidate lifecycle, ownership, error, partition, and finite-bound checks. This is not full Sound acceptance, original-app equivalence, native timing admission, or a complete-firmware callback result.**

The reviewed implementation is a new analytic candidate. The only original-reference inputs used by this review were the supplied numeric behavior brief, captured-envelope metadata, and measured-amplitude CSV. This reviewer did not open or reuse original engine source, execute an original app/AU binary, touch a device, use PSRAM, or modify the candidate, production source, or Git. All review code and artifacts live in `candidate_review/`.

The implementer was given the same numeric fixtures. Consequently this is an independently implemented verification of a fitted, known subset; it is not held-out evidence that the fit generalizes to unseen original behaviors.

### Exact final inputs

| File | SHA-256 |
| --- | --- |
| `frozen_03/snapshot/dnbx_core.c` | `9cdcea0b90b691d05cd07306433b9b4cf42c023bf419a251d7ff436fa13487fa` |
| `frozen_03/snapshot/dnbx_core.h` | `a863f6b04f65202a3ba7f5640b4dc89e0730a15c096894bf9d914caaf7c8671c` |
| `frozen_03/snapshot/fastmath.h` | `fbe052086010b17d8194b9cd3d61d3cb349cc1def3817ae9651a6683d7850b09` |
| Tested host shared object | `fadb3f0290e3b3e76008be38f6edcdae4e38dbbf8fed322180b83af23374b795` |

The main result is `frozen_03/RESULT.json`. Build command and compiler are captured there and in `frozen_03/COMMANDS.json`; compilation uses C11, `-O2`, `-Wall -Wextra -Werror`, `-ffp-contract=off`, and `X0X_HOST`. The final header changed comments only from the header used by the timing audit. The separately rebuilt `frozen_02` and `frozen_03` shared objects have the identical hash above.

## Captured amplitude acceptance

`ACCEPTANCE-DECLARATION.json` was written before executing the comparison. It fixes the primary metric at **maximum absolute envelope-level error no greater than 0.001**, with finite values required, at the exact original sample timestamps. No fitted alignment, time shift, threshold adjustment, or skipped boundary is allowed. This is 0.1% of full scale, more than 5,000 times the supplied maximum within-interval interpolation residual of about 1.8e-7, while still rejecting a one-tick timing error and a linear contour. It is a scoped engineering tolerance, not a claimed product listening threshold.

The two original recordings are at 48 kHz. Case 1 uses physical AHDSHR `[0.01,0,0.05,0.7,0,0.1]`, note-on at 0 and note-off at 4096. Case 2 uses `[0.03,0.02,0.2,0.4,0.04,0.3]`, note-on at 0 and note-off at 16384. The CSV contains 2,560 intervals; every interval's measured first and last level is checked, giving 5,120 comparisons. These levels were inferred from original recorded output through matched-waveform cancellation, not sampled from original internal state.

| Original fixture | Comparisons | Maximum absolute error | RMS error | Worst frame | Result |
| --- | ---: | ---: | ---: | ---: | --- |
| Case 1, baseline attack/decay/release | 1,024 | 6.9401957e-7 | 2.1161314e-7 | 271 | Pass |
| Case 2, positive hold/release hold | 4,096 | 7.9466463e-7 | 1.9952365e-7 | 1103 | Pass |

The candidate's public snapshot was read after rendering the corresponding sample. To verify that the snapshot actually describes the emitted gain, the reviewer also rendered a matched constant-amplitude candidate and divided the candidate audio by it wherever the reference magnitude exceeded 1e-6. Across 40,958 well-conditioned samples, the largest audio-derived gain versus snapshot discrepancy was 1.4049037e-7, below the separately declared 2e-6 cross-check tolerance. This cancellation validates the candidate amplitude multiplier; it does not compare its oscillator or stock filter sound with the original sampled source.

The original baseline reaches full scale at frame 479, remains flat through 495, and begins decay at 496. The final implementation matches those timestamps. Its public header explicitly documents the 16-sample hold exception after a positive-duration attack with hold=0; a zero attack has no minimum hold. This behavior is qualified by the supplied 48 kHz traces. Original 44.1 kHz alignment remains unmeasured.

### Deliberate negatives

| Wrong behavior | Case 1 error | Case 2 error | Required outcome |
| --- | ---: | ---: | --- |
| Shift candidate contour forward 16 frames | 0.06371963 | 0.02247660 | Both rejected at 0.001 |
| Substitute a linear AHDSHR contour | 0.32132317 | 0.26942002 | Both rejected at 0.001 |

The captured component's velocity relationship is linear: velocity 127 is 127 times velocity 1 within the supplied recording residual. The candidate's normalized ratio `127*output(v1)/output(v127)` has maximum error 1.8233706e-7 over 4,095 well-conditioned samples, passing 1e-4. Selecting the candidate's squared velocity mode deliberately gives normalized error 0.99212599 and is rejected. The explicit squared mode is a separate available behavior, not the mode accepted against this captured component.

## Lifecycle defect found and fixed

The first frozen implementation (`9165342a...`, retained in `frozen_01`) failed its claimed oldest-held allocation policy. After repeated retirement and reuse, surviving age ranks accumulated gaps and eventually tied at 6. A new note then stole slot 1/note 71 while the oldest held note 65 remained in slot 5.

The exact public-API reproducer fills notes 60..65; releases/retires 60..64 and replaces them with 70..74; then repeatedly releases/retires slot 0 and reuses it for 80..89. All envelopes have zero times and sustain 1, so a single rendered sample retires a note after note-off. On a final note 100, the old candidate chooses slot 1. The final candidate compacts the surviving age order during note-on and chooses slot 5, with distinct live ranks 0..5. The implementation writer made the fix; this reviewer did not edit it. The failing and passing event logs are retained in the two result files. The note-on compaction adds a bounded event-side scan, included in the callback burst timing.

The final lifecycle checks also establish, for the tested schedules:

- Bit-identical stereo output, complete part state, and render counters for the same 16,384-sample event timeline partitioned as 1, 16, 64, 1024, or the repeating sequence 1/3/5/7/13/16 frames. Event positions include 1001,1177,1201,2123,2347,3451,5003, and5131; events always precede the selected sample.
- Isolation of one part's output during 64 adversarial interleaves of activity in a second part.
- Four independently evolving amp/filter contours across two layers.
- Sustain-pedal release does not restart an unrelated envelope already releasing; a pedal-held released gate remains active and subsequently retires after pedal-up.
- Retriggering a held note reuses its slot, and repeatable all-off returns silence while preserving stored raw controls.

These are checks of the candidate's declared mechanics. They do not establish the original instrument's full polyphony, legato, retrigger, voice-steal, or early-release behavior. In particular, the original irregular-block recording has not been accepted by this comparison.

## Error handling and finite bound

The final run passes 83 argument/state/output-preservation checks, including nonfinite prepared parameters, MIDI/range errors, invalid initialization rates, oversized rendering, null and equal output arrays, absent raw values, and a zero-frame render with null arrays. Eleven raw-admission cases preserve exact binary32 bits, including positive and negative zero, 1.32,1.001,70,9,-0.008403362, maximum finite float, and the reserved/externally owned status distinctions.

Partial output overlap, insufficient buffer capacity, and output/work/state aliasing are caller preconditions. The implementation does not validate them; the final header now says this explicitly. This review does not call an overlap violation a supported error path.

The finite sweep covers 56 extreme two-layer/six-voice configurations at 8k, 44.1k, 48k, and 192 kHz, both oscillator modes, all waveform stops and blends, and extreme gains, cutoff excursions, resonance, note pitch, PWM and LFO. Four additional configurations exercise one-sample and zero-duration stages. Across 229,888 rendered frames all sampled output/envelope values are finite and the largest output magnitude is 7.1118312, within the declared 32 bound.

A separate maximum-duration check renders 6,144,033 frames at 192 kHz: a 16 s attack, sustain boundary, and 16 s release. Attack remains 0.9999998212 at its penultimate sample and reaches 1 exactly at its endpoint; release remains 1.1559475e-8 at its penultimate sample and reaches 0 exactly. The note retires. Monotonicity is checked at every rendered block endpoint, not claimed as a separate exhaustive per-sample test.

The output bound also has a simple source argument for initialized state modified only through the API: both filter histories are clamped to [-4,4]; velocity and amp lie in [0,1]; six voices give at most 24 per layer before gain; gain/6 is at most 4/6, so one layer contributes at most 16; two layers with pan weights in [0,1] give at most 32 per channel. Accepted times and sample rates produce at most 3,072,000 samples per segment. Oscillator increments, phases and control preparation stay bounded, and envelope log inputs stay positive. This is a finite arithmetic argument for this candidate under its stated calling contract, not a native hardware robustness claim.

## Render counters omit event and control work

The public counters are useful render-only semantic counts. They are not CPU cycles, native instructions, or complete callback work. An independent test-only source copy intercepts all existing count sites even when the caller supplies no public counter, and separately intercepts direct exp2/log2 calls. The deployed candidate is unchanged. Trace transformation, hashes and exact commands are in `run_callback_audit.py` and `callback_audit/`.

The selected 512-frame workload contains two candidate parts, six voices per part, two layers per voice, and two 256-frame renders per part. Its burst contains 16 note-ons, 16 note-offs, 4 physical layer updates, 436 raw control stores, 4 sustain updates and 2 velocity-mode updates, all inside the timed region.

| Work in that selected burst | Public render count | Independent full selected trace | Omitted |
| --- | ---: | ---: | ---: |
| Direct exp2 calls | 48 | 168 | 120 |
| Direct log2 calls | 48 | 112 | 64 |
| Envelope stage entries | 144 | 208 | 64 |
| Envelope ticks | 96 | 160 | 64 |

The additional 56 pitch-related exponent calls consist of 24 from the four layer updates and 32 from the 16 note-ons. The other 64 omitted exponent calls occur during envelope preparation. Event allocator scans, validation, state copies, raw bitset stores and other arithmetic are also outside the public counters; the table is not a total instruction count.

Zero durations do not make event work free. A separate trace in `callback_audit/ZERO-STAGE-RESULT.json` shows that two held-note retriggers with all-zero times enter 32 envelope stages and call exp2 four times, while public render counters report zero for both. Two subsequent zero-stage note-offs enter 24 stages, again outside render counters.

For a 512-frame slice with both parts fully active, the rendered semantic maxima are 6,144 voice-slot visits,12,288 layer samples/oscillator evaluations,24,576 envelope-sample evaluations,2,048 LFO updates, and36,864 BLEP evaluations for a saw/pulse blend. With events delivered only before that slice, each of its 48 envelopes can traverse at most three forward stages during rendering, giving at most 144 render stage entries and 48 new decay/release coefficient preparations. These bounds describe fixed loops and semantic operations; they do not convert into a verified cycle bound. The immediate event API itself has no event cap, so the surrounding caller must bound event/control delivery before a complete callback cost can be bounded.

## Host timing, with explicit limits

`callback_audit/RESULT.json` records 2,000 timed callbacks per case, 64 warmups, `CLOCK_MONOTONIC_RAW`, compiler flags, host metadata and counts. Both counters-enabled and counters-disabled cases are retained. The table below uses the counters-disabled 512-frame cases; other cases are in the JSON.

| Selected two-part512-frame workload | Median | p99 | Observed host maximum |
| --- | ---: | ---: | ---: |
| Saw/pulse blend, steady | 298.032us | 688.491us | 3003.125us |
| Sine/triangle blend, steady | 274.828us | 1082.586us | 9785.132us |
| Saw/pulse blend plus full listed event/control burst | 311.742us | 820.857us | 4032.101us |

These are **observed shared-host wall times**, not proven maxima or an isolated performance baseline. The main timing run ended 2026-11-18T14:56:29.789UTC; other workers were using the same environment around this period. Scheduling, frequency, caches and thermal state were not controlled. The unusually large observed sine/triangle maximum is retained, not discarded. A first successful probe execution whose result writer failed on unavailable `lscpu` metadata is preserved in `callback_audit/first_attempt/`; the completed run makes that metadata failure nonfatal.

The measured work is the **candidate engine plus a fixed event/control slice**, not the complete firmware callback. It excludes the existing private/global FX bus, other instruments, drums/BREAK, mixing and USB/UAC tap paths, event decoding, HAL and interrupt/preemption cost. It establishes no whole-product timing pass, native WCET, available deadline margin, or xrun result. A 512-frame 44.1 kHz half lasts 11.609977 ms, but comparing the host numbers above with that duration does not establish device admission.

## Native integration and profiling work still required

The resource reviewer identified existing host fixtures under `firmware/76040daa5b52d686a8b7b544296ba5d79610a95d/tests/host/`, including `audio_path_stress.c/.py`, `audio_stress_test.c`, and `integration_perf_bench.c`. They target the current C/M instrument choices and have no candidate selector. Their engine/USB-volume composition also excludes HAL/preemption. They were not repurposed into a synthetic claim of complete candidate firmware stress.

The future native profiling recipe should preserve the actual replacement adapter and full callback architecture:

1. Pin the accepted candidate, replacement adapter, firmware snapshot and link map. Deliver a declared maximum number of prepared control changes and note events per half, or explicitly measure the existing queue/admission policy. Do not infer a cap from this review's 32-event workload.
2. Measure the enclosing native `render_block` path in `resource_review/native-baseline-76040/firmware/src/app/main_fm1.c`, including all existing instruments, independent private tails, shared FX, mixing and USB work. Use the existing elapsed-time source from `fm1_time.h` (TIMER4, 24 ticks/us), not unverified cycle-counter assumptions.
3. Record `x0x_dbg.max_us`, `last_us`, and `late`, plus `x0x_worst.us`, `over90`, `over100`, `nsub`, `wait`, and `tmr` when the existing debug profile is enabled. Also retain `audio_xruns` from the ALNK0 ISR's free-half-change check. The optional cycle path in `fm1_perf.h` is explicitly not hardware-verified.
4. Exercise both parts at the intended voice/layer load, dense same-half retriggers/steals, zero and long envelope transitions, maximal permitted control delivery, simultaneous existing FX tails, drums/BREAK, and USB modes. Report the complete measured callback maxima and xrun deltas, the supplied stimuli and duration, and instrumentation overhead. Keep renderer-only counters as explanatory diagnostics.

No device experiment was performed or authorized by this review. The recipe identifies the evidence needed for the separate timing gate; it does not fill that gate with a host estimate.

## Remaining behavior coverage

The original active filter-envelope response, pitch envelope, original filters, oscillator bandlimiting, sub/noise behavior, all original layer interactions, normalized envelope laws outside the supplied scalar targets, routing dictionaries, all preset/control applications, complete original lifecycle/polyphony, simultaneous private FX tails and full program sound remain outside this acceptance. Preserved raw controls do not mean their sounding behavior has been implemented. The 5,120 endpoint match must not be promoted into full SuperJ6 or Sound correctness.

## Reproduction and retained evidence

From `/workspace/scratch/67e685478953/superj6_research`:

```sh
python3 candidate_review/verify_candidate.py --label frozen_03
python3 candidate_review/run_callback_audit.py
```

The first command is the actual completed invocation; a new rerun needs a fresh label because existing evidence directories are intentionally not overwritten. `frozen_01` retains the failing allocator; `frozen_02` and `frozen_03` retain the verified correction and final public-header clarification. `preflight_01/HARNESS-ERROR.json` records an early harness ABI-binding error caught before audio execution and corrected to include the new work field; it is not reported as a candidate defect.

All substantive output is in `ACCEPTANCE-DECLARATION.json`, the frozen `RESULT.json` files, per-sample candidate envelope CSVs, the callback results, the zero-stage trace result, exact command logs, and `MANIFEST.json`. The independent reviewer applied the defect-and-evidence principles of the code-review skill; its Git/peer orchestration was not run for this isolated, explicitly report-only snapshot task. No external code review or production mutation was performed.
