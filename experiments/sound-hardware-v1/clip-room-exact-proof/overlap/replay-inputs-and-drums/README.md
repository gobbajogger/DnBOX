# Replay input isolation and exact drum throughput follow-up

Base is the full-cycle sine diagnostic at 26e0aa8717f85a374a2108c3836a1ddda4cf4513. No firmware branch merge, Nightly change, release, device write or SuperJ6 change.

## The early Pro solo difference comes from the replay's default inputs

I rebuilt the exact earlier two Pro model packages, confirmed their SHA-256 values against the saved input receipts, and used the same project, input tape, emulator and model companion partitions. The only replay change was adding explicit empty `FM1_BENCH_KEYS=` inside the solo helper's existing `env -i`. Leaving it unset had allowed the emulator's default panel input, producing earlier audio in the capture. With the empty override both raw captures stay exactly silent until frame 195,487, and both have the original six held channel-3 MIDI notes and no dropouts. The **full 233,823 stereo frames now match exactly**, no alignment, filtering or trimming. The complete raw PCM hash is identical. Both complete all 391 expected halves. Peak in this diagnostic is 10,870 µs baseline and 10,433 µs sine-table, which does not replace the prior canonical timing receipts. This isolates the unexplained pre-MIDI difference without claiming all-project or physical device parity.

To repeat without touching the canonical helper, copy it to a scratch path with `sed 's/FM1_STRICT=1/FM1_BENCH_KEYS= FM1_STRICT=1/' replay-solo.sh`, supply explicit model-specific seed/project/controller paths and the original MIDI events (see previous report), then feed the two raw outputs to `sine-work/pcm-delta.py`.

## Drum throughput trials, Pro both-kit stress replay

| Variant | Peak (µs) | Complete halves | Stale spans | App flash free | Audio pool margin |
|---|---:|---:|---:|---:|---:|
| Full-cycle sine control | 15,430 | 341 | 49 | 4,372 B | 27,392 B |
| 808 four-frame Schmitt batching | 15,442 | 341 | 49 | 4,020 B | 27,392 B |
| 909 exact IMA transition table | 15,882 | 340 | 49 | 4,516 B | 21,696 B |

Keep the sine-table build alone. The 808 change uses current local `dt[]`, retains the scalar drift/jitter/wrap path and uses no persistent state from the old dry build. Its compiled host proof compares 8,205,582 randomized frames in 64,000 spans across all four buses and full kit state, bit-exact. Guard counters are separate (apply the older `qualification/lazy-trace.patch` plus `808-schmitt-count.patch` on top of the 808 treatment). The instrumented runs have the same worst ordinal 305 with 256/122/134 frames. The guard admits 59 of 127 checked windows and batches 236 of 512 bank frames. The markers from 909 completion through 808 completion total about 88 µs less, but this smaller stage gain does not reach uninstrumented peak/cadence or dropouts. Do not adopt.

For 909 the boot-built 5,696 B predictor/step table changes only the sampler decode in that kit, not BREAK, format, sample data or DSP math. The full-kit proof matched 23,280 spans and entire 7,648 B state/four buses separately at -O0 and -O2. The exhaustive decoder witness checked all 186,646,528 combinations of valid signed predictor, step index, nibble and byte parity, exact result and next state. To repeat, compile the witness as C99 with `-DX0X_HOST -O2 -ffp-contract=off -fno-fast-math -fno-builtin` using patched DSP source and generated build headers, link `x0x_param.c` and libm, and execute it. A second independent Pro pair forces both Synth One wet sends at 0.5 once six voices are held. Native RAM confirms both wet switches on, both mixes at 0.5 and delay time 0.30 s. It has a different control and is not comparable directly to the unforced baseline: 15,920 → 15,879 µs, both 49 stale spans and 340 complete halves, +128 B flash headroom and −5,696 B audio pool headroom; a smoothed 909 job estimate fell by about 275 µs/256 frames but whole-block peak barely moved. Unforced candidate was slower and one fewer complete half. Do not adopt either.

Rebuild the normal full-cycle variant, then independently apply `808-schmitt-local.patch` or `909-transition.patch` and rebuild with identical lazy flags. Use previous Pro matched heavy replay, not a dry/forced FX cross-comparison. Forced comparison uses parent `wet-fixture.patch` payload inserted at the current `s1c_render` entry in both builds before any notes or rendering comparison. Hash manifests record the actual inputs. These are isolated and rejected experiments, not product changes.
