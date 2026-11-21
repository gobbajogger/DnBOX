# Active-filter diagnostic: 3 of 3 notes passed

The single authorized host run completed at **2026-11-19T00:47:11.080685+00:00**, with
outer process exit **0**. Compiler and renderer allocation was released at
terminal. One build, one static replay, one active capture and one unchanged
metric execution ran serially. No retry or tuning followed.

## Measured result

| MIDI note | Absolute RMS error (dB) | Spectral error (dB) | Gate |
| ---: | ---: | ---: | --- |
| 36 | 0.232226744444 | 0.682165610250 | PASS |
| 60 | 0.239112071958 | 0.767147168739 | PASS |
| 84 | 0.196105441022 | 2.106153903352 | PASS |

The largest RMS error was **0.239112071958 dB** (note 60);
the largest spectral error was **2.106153903352 dB** (note 84).
The fixed gate requires RMS ≤1 dB, spectral ≤3 dB and zero nonfinite samples.
Observed nonfinite samples: **0**. Comparison gain was exactly
1; no gain fit, normalization, phase/time alignment or tolerance change was
applied. Full precision appears in `results/active-one/SUMMARY.json` and
`active.paired.json`.

The case is `filter-modulation/1.2_2000_0.9`: filter ADSR .2/.3/.4/.5 and depth 1.2,
velocity 100, cutoff 2,000 Hz, resonance binary32 .9 and common amp ADSR
.05/.005/.8/.05. Fresh notes 36, 60 and 84 used the exact tuning words,
176,448 silent warmup frames, 88,192 retained frames and note-off at frame
66,112 at 44,100 Hz. The original reference is one new numeric capture for
this contract, not recovered historical active PCM.

## Static identity prerequisite

Before the active capture, the same wrapper replayed the selected static
record. All 1,058,304 bytes matched the frozen D selected PCM. Both hashes
were `7aded201280455c1ddaee86abab4d82b6d791953e81d0d9cb40e17946cbfa380`. The exact result is retained in
`results/active-one/STATIC-IDENTITY.json`.

## Frozen identities and files

| Input or output | SHA-256 |
| --- | --- |
| Reviewed source-only declaration | `373711a1cbfa6d04a5fe40b45abe83cef327e05cb0ac4aa94b761d33190fa862` |
| Reviewed runner | `f661569877044d9f7c52bfaa02c8cc368818aea6e2fe94e7403972147108b6cc` |
| Reviewed wrapper | `e905c17e104db087a5f8790ff9c1761d29e8e1216d1e3861568a2253cc54fa89` |
| Active original mono PCM | `3f7303aa1383c678e4e251c36506a141a0ece419e38d4411e2c0a0447bf0bb74` |
| Active D candidate mono PCM | `7ac88e50c0d0566c1a707dbeda8c81bed5b0848cfe44634011228999ea1f02b1` |
| Original/candidate paired PCM | `338458922ef3861ec77898dfe386328a2ad6fa19897d265a010d106d96269bce` |
| Compiled host | `988006d70a648c4fc213b8047655df06f9e41e2993a8ab36e763baeecc63c936` |
| Unchanged metric | `bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1` |
| Actual summary | `10c5a4b15b51196ce010113ea89cf997b0f49d5d2516ea84e30b6252c471e4fa` |

The runner captured verified source bytes into `results/active-one/build-inputs/`.
The compiler and metric consumed those staged paths. Each of the four bounded
subprocesses returned exit 0; exact commands, timestamps, stdout and stderr
are retained in `results/active-one/logs/*-attempt01.json`. The source hashes
remained unchanged. The source-only declaration and its first pre-review
freeze were preserved without revision after authorization.

`results/active-one/static.candidate-mono.f32le`,
`active.candidate-mono.f32le` and `active.paired.f32le` retain the full output
streams. `RUN-DECLARATION.json`, `PRE-RENDER-DECLARATION.json` and
`PRE-METRIC-DECLARATION.json` record the hashes before the corresponding
operations. `EXECUTION-MANIFEST.json` indexes these artifacts additively.

## Qualification

This is one measured active-filter record at three notes. It does not qualify
the remaining dynamic matrix, arbitrary parameter modulation, full presets,
all control IDs, effects, whole-instrument identity, native cost or timing.
The independent D static physical screen remains **138/180 accepted notes,
45/60 accepted files, full-screen FAIL**; this active result does not change
that gate. Production activation remains false.
