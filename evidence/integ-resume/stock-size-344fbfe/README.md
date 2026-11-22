# Stock XIP reduction on the latest bound composition

The scratch input includes Sound `5b1d1c71`, FWUI `1ec2d61a`, and the guarded 909 MIX producer from the prior Integration composition. The FWUI input includes the shared native line coverage and both Bass note cues, superseding the earlier UI-only input. Exact source replay retained every path from the three input histories.

| Profile | Elite free XIP | Pro free XIP | Pool margin | Elite/Pro populated peak |
|---|---:|---:|---:|---:|
| Matching `-O2` diagnostic control | −500 B | −492 B | 3,804 B | Stock link fails |
| Retained `-Os` for 909, 808 and BREAK | 1,036 B | 1,044 B | 3,804 B | 7,013 / 7,841 µs |
| Reserve boot token variant | 1,468 B | 1,480 B | 3,804 B | 7,010 / 7,878 µs |

The retained profile closes the measured deficit with 12 B Elite and 20 B Pro above the 1,024 B reserve floor. All four calibrated public-emulator runs were eight seconds, with zero late halves, xruns or skipped cycles, two ready Sound owners, both 11-row drum kits active and a loaded BREAK loop. The package containers and stock partition maps passed.

The three engine reductions change only compile optimization settings. Focused native 808 and BREAK PCM matched byte for byte. The 909 target PCM checks matched. The boot compression preserves all 1,026 reconstructed table words and the focused PCM checks. Its populated full-scene PCM differed from the retained profile late in both replays, starting at stereo frames 313,072 Elite and 308,465 Pro with zero-based indexing. Keep that boot change in reserve pending the full-scene difference review.

The earlier whole-unit 909 `-Oz` Pro profile passed the calibrated public runner with zero late halves and xruns. A separate archived raw observer measured 12,366 µs, eight late halves and eight xruns on the same non-sub input bytes. Its internal tick reporting differs from the pinned public recorder. The two results establish a runner discrepancy and do not isolate an optimization regression. Both source packages and exact captures remain local with the digest and evidence reports.

## Other qualification

The bound 909 MIX producer, addressed Nightly Apply/Undo, full-queue refusal, panel A–B–A detection, project reset revision invalidation and project protocol all pass on the retained source. The stock application contains the tested binding. Firmware incarnation and catalog admission remain provisional. The latest FWUI source passed 3,368 spatial line checks, the 32 Bass cue cases and 22 MIX routing bindings in focused host tests. The current compact-program generator reproduced its tracked header and passed 3,591 exact metadata cases plus corruption rejection. LeakSanitizer initialization still prevents an exact current deliberate-leak result. The final combined gates, strict ten-minute run and production connection acceptance are pending.

## Project Sources

The five original files are preserved. The verified local one-file archive is 59,608,737 B and reproduces every original SHA-256. LoopyX0X37 showed 33 visible sources on a Pro account with a documented 40-file project limit, and the account meter showed 15.9 GB of 100 GB used. The earlier project-capacity rejection and those counts leave the specific limiting quota and a usable slot unverified. No existing exact equivalent was established. The prior authorization remains recorded, and the archive stays staged.

## Source replay

Begin with the [prior composition](../stock-compose-7233317/README.md). Incorporate its bound and BREAK test-path patches, then the exact Sound `5b1d1c71` merge. Restore the supplied local commit objects in a scratch checkout only after verifying their hashes. The virtual FWUI merge with `1ec2d61a` and `size-fwui-resolutions.patch` reconstruct the latest pre-optimization bound tree. Apply `size-all-os.patch` for the retained tree. `size-boot-token.patch` is a separate reserve step. See [verified replay](size-final-replay.log) and [machine receipt](receipt.json).

Build each model with the prior exact configuration, including `X0X_SYNTHONE_PATCH=1 X0X_SUPERJ6_FACTORY=1 X0X_DEV_PADCAL=0 X0X_COMPOSE_S1_EFFECTS=1 X0X_DNR2_SCRATCH=1 X0X_TANH_BOOT_TABLE=1`. Set `X0X_INTEG_DRUM_SIZE=1` for the retained profile. The matched diagnostic control reuses all other fitted objects and removes the size setting from the three engine translation units. The wider diagnostic linker region is confined to its ELF measurement.
