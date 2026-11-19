# One proposed original active-filter capture

**Preparation only. No active capture, new build or link ran.** Root must first release its native sequence and separately authorize this one command.

The successfully frozen original-only host executable is 63,936 bytes with SHA-256 `edf9f733ff449ecf345f9843952dea5d7cdab00825de88e4d7c3c61e6135f91d`. Its public wrapper SHA-256 is `9ddfb70be9b9d26cc7d2b27f20ce228aeab197c3a9f8a65decc5c42a99f5a2b4`. That executable reproduced the baseline mono identity exactly: 1,058,304 bytes, SHA-256 `b02703f0893f9e7676e4db03048f9fefe8575ae1b1a1f44da1167253309a6cd2`.

## Exact case

Use `filter-modulation/1.2_2000_0.9` at 44,100 Hz, velocity 100, cutoff 2,000 Hz, resonance binary32 0.8999999761581421 and depth binary32 1.2000000476837158. The **600-byte active physical control vector** has SHA-256 `6f7e9834054579c2d61e7e33072c2651107e3e987a75aa59c1b222d3f5164ff4`.

The active filter ADSR is **0.2 / 0.3 / 0.4 / 0.5**, as declared by the historical modulation case. The baseline filter ADSR was 0.05 / 0.05 / 1 / 0.5. Exactly four parameter values differ: filter depth (ID 19), filter attack (22), filter decay (23), and filter sustain (24). The other **146 controls are bitwise unchanged**, including the common source, amplitude and routing controls; filter release remains 0.5. All decimal and binary32 identities are frozen in PROPOSAL.json.

Notes remain 36, 60 and 84, each with a fresh original instance, 176,448 silent warmup frames, 88,192 retained frames and note-off at retained frame 66,112. The one proposed process has a **15-second timeout**, processes **793,920 total reference frames** and must write **1,058,304 mono bytes**, with zero nonfinite samples.

## Single proposed command

The exact argv array, controlled environment, tool/runtime identities, fresh output path and explicit preflight/stop gates are in [PROPOSAL.json](PROPOSAL.json), SHA-256 `088ce262d3bcc3a336b63cf4ad4a3d3914b3ca5235e32b177a3405395a81ec35`. There is one renderer invocation and no compiler or linker invocation. No retry or expansion to the other 17 active files is included.

The output will be a **new original-reference capture from the exact-baseline-validated reconstruction**. Its mono SHA-256 is not known until capture. The old missing pair hash `f0f95c5ec0e496beaec004602b347bea41be069388e69c42f9f28b69c110e98a` refers to original-plus-old-candidate bytes and must not be used as an expected mono identity or a historical recovery claim.

No candidate comparison is run by this proposal. Existing fixed-gain and RMS/spectral gates remain unchanged. Original source and executable material stay with comparison ownership; only public wrapper/build metadata and numeric observation evidence may cross to independent implementation.
