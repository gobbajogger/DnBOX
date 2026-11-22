# Synth One dual-owner delay on stock Elite and Pro

No factory-preset identity can guarantee a short delay or an empty history under the supported live controls. Both frontends accepted the full time range on all 694 restored programs. Two initially dry programs retrieved their old, independent histories at the wet tap after a live sweep. The original delay buffers cannot fit in the stock application memory windows. This investigation adds host comparison evidence and admits zero new firmware effects.

## Original programs and live controls

The probe applied the real 144 ordered setter calls per program to both the source-derived control frontend and the raw-original control frontend, using the repository's pinned factory harness at 44,100 Hz. All 99,936 commands completed. The public controls are delay on (41), feedback (42), time (43), mix (44), BPM (64) and tempo sync (139). The latter two can change the resolved time during restore. Comparing the resolved targets with the binary32 input found 395 changed times. Each row in `programs.csv` includes bank, bank-local index, name, UID, raw time, final targets and live time endpoints.

| Restored state | Programs | Feedback zero | Synced | Different raw/final times | Final base-time range | Median base time |
|---|---:|---:|---:|---:|---:|---:|
| All | 694 | 56 | 569 | 395 | 0.0124999993–2.49999976 s | 0.333333343 s |
| On with positive mix | 525 | 31 | 447 | 293 | 0.0124999993–2.49999976 s | 0.333333313 s |
| Dry at target | 169 | 25 | 122 | 102 | 0.0178571418–2.35294080 s | 0.333333343 s |

Of the 169 dry targets, 161 have delay off and eight have delay on at zero mix. Every program allowed disabling tempo sync, toggling on/off, sweeping feedback from 0 to 0.899999976, mix from 0 to 1, and time from 0.000362811785 through 2.5 seconds at the endpoints. The setter probe checks endpoints, not the duration of parameter smoothing. “BB Synth Won Sign-off” has the shortest restored time, is fully wet, and has feedback 0.827999949. “Outer Limits” starts on with mix 0.814999998, base time 2.49999976 seconds and feedback 0.132749990. Neither program needed a user edit to establish its stated restored target. The largest unedited long tap is already nearly five seconds.

## Two simultaneously running factory voices

`authored_histories.cpp` applies every original restore command for Bank A programs 1 and 8 (zero-based) to separate raw-reference schedulers with separate full FX instances. Their original presets start with delay off. Each played a different note/velocity stream with feedback zero, mix zero, and a short initial delay. Notes stopped and the base-time target changed to 2.5 s at four seconds. At 5.5 s, both owners enabled their delays and raised the mix to one. The original frontend's smoothing ran throughout. The following left wet tap measurements include only frames with actual crossfade position above 0.9 and all interpolation neighbours from before the time edit.

| Program | Wet reads / nonzero reads | Oldest tap lag | Left tap peak | Left tap RMS |
|---|---:|---:|---:|---:|
| Dat Bomb Bass, Yo | 3,158 / 3,158 | 220,416 frames | 0.214707494 | 0.103610751766 |
| June Oh 06 Sqr | 3,158 / 3,158 | 220,416 frames | 0.625464559 | 0.249184300742 |

The oldest observed age was 4.9981 seconds. The two owners’ differing peaks and RMS values came from their own note histories. These measurements are at the effect's delayed tap, before the later reverb and master processing. They use the actual comparison graph with normal smoothing. The rest of the 694-program corpus received a control-target replay.

## Capacity and scope of the bound

In `tests/fixtures/synthone-factory/fullfx/source_fx.hpp`, each owner updates two long histories at double the base time and two short histories at the base time. The fourth takes the right long delay's output. The graph advances all four when the audible crossfade is dry. The corresponding Soundpipe vdelay constructor allocates (10 × 44,100 + 1) binary32 values per ring. The authentic constructor allocates 1,764,004 B per ring, 7,056,016 B for four and **14,112,032 B** for two owners' eight rings, exclusive of metadata and other effects. Even straightforward binary32 rings right-sized to their maximum legal taps would take about 5,292,000 B for two owners before interpolation guards and metadata.

A narrower lower bound applies to an insert that preserves arbitrary input-fed left histories losslessly. With feedback zero and a five-second integer tap, each owner outputs its earlier 220,500 signed Q15 input words in order when fed zeros. The zero continuation therefore distinguishes every preceding pair of Q15 streams. There are 2^(2 × 220,500 × 16) such possibilities. Any lossless representation of these histories needs at least **882,000 B** in the worst case, before right channels, feedback, metadata or any other workload. This is a counting argument over arbitrary Q15 words, independent of the particular test generator. The stand-alone authentic-vdelay replay verifies the integer-tap behaviour for two input traces. They differ on 220,498 of their 220,500 frames, and both fixed-long and warm short-to-long experiments recover all 441,000 owner words exactly. The latter component experiment switches the tap directly. The musical test above separately exercises app smoothing.

The linked stock ordinary RAM, pool and XPOOL windows total 451,584 B (98,304 + 344,064 + 9,216). Even appropriating their entire combined capacity would leave 430,416 B missing against that narrow arbitrary-input bound. The actual fit is tighter. The fresh compact Elite and Pro links report 1,100 B ordinary RAM free, 8,972 B pool free and 0 B XPOOL free. The pool floor of 8,192 B plus 512 B required margin permits only 268 B incremental pool work. The existing 3,720 B per-owner backend union holds 1,048 B of partial Synth One state. Its conditional 2,672 B remainder per owner is shared layout capacity, and does not solve the history cost. The two selected-FX owners (4,252 B apiece), global 158,400 B delay and other concurrently active workloads already have owners. Physical layout and symbol references are in `experiments/sound-hardware-v1/resource_review/RESOURCE-REVIEW.md` and the fresh stock native ABI/fit packet.

This proof does not establish the minimum state for a *closed deterministic synth* whose reachable signals may be regenerated from less state, nor the minimum for a candidate that meets the lane's spectral/level thresholds with bounded approximation. The tests provide no threshold pass or target callback result. The prior common-send feedback and division-recall experiments verified another path: changing the common divider moved both sends by 5,292 frames, and increasing the common feedback changed later repeats. Those experiments did not exercise independent warm insert histories.

A stock-constrained alternative worth testing is per-owner resynthesis from bounded dry-engine snapshots plus timestamped note/control events. Start with feedback zero, fixed owner-dependent note streams and the two live sweeps above. Compare the competing reconstruction with the observed wet samples and predeclared final-mix thresholds. Require a measured upper bound on stored events/checkpoints and on work under rapidly moving taps before expanding to nonzero feedback, whose past wet output feeds future writes. Fixed templates or a preset-only whitelist cannot provide these guarantees for live edits. No such reconstruction is admitted by this measurement.

## Reproduce

Run these commands in the repository root. Set `here` to the absolute directory containing this README and choose a fresh output directory, for example with `work=$(mktemp -d)`. The bootstrap verifies and expands the repository's test-only factory input archive. The subsequent probes use the original test controls and FX only in their host comparison executables.

```sh
python3 tools/restore-synthone-test-inputs.py --bundle assets/test-inputs/synthone --output "$work/inputs"
python3 tests/host/run_synthone_factory.py --source-build "$work/inputs" --output "$work/build" --build-only
python3 "$here/replay_targets.py" "$work/build" "$work/results"
python3 "$here/summarize.py" "$work/build/commands/ORIGINAL-PRESET-COMMANDS.json" "$work/results/targets.csv" "$work/results"
sh "$here/replay_histories.sh" . > "$work/results/history-results.csv"
cmp "$here/programs.csv" "$work/results/programs.csv"
cmp "$here/summary.json" "$work/results/summary.json"
cmp "$here/authored_histories.csv" "$work/results/authored_histories.csv"
cmp "$here/history-results.csv" "$work/results/history-results.csv"
```

The command generator identifies original Synth One commit `6466a3715c96b7ecf1dd255a218cbd571408a314`. SHA-256 values for the generated command description and binary stream are `25a9c436f4dc7c37517c77acb632ccdf0753a885ee474e1e8f93c02b0c099a2e` and `460d0869c7287ab10dc2ac1928e56b899e8dc089df021ab82d8704ab28cfe7af`. The hash for the checked factory manifest is `35de2aa8c01be45b65de01f9f446f61b7aaf6ddd2d65786a816ac31bac0dbf98`. The original vdelay implementation has SHA-256 `9df1510722e55febadc10f26e14f48e7a8f1badbab633360c8053309c45a7f78`, and the full comparison FX graph has SHA-256 `aee3fe78b450b35bfe6d40741d12dcedf02e24388bdf07850bde7b13d8d8d33e`.
