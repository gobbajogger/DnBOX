# Stock native Remote candidate

The software candidate at `156f4629451457b963c98776715ecf3d97abd5e5` carries the Integration lane at `4fa5e29f`, Sound `0ac65800`, and merged firmware UI `86e0f63f` with zero deleted paths from each parent input. Merging the qualified FWUI tip changed only the Juno slot host test relative to this composition. The focused test passed all 8,918 assertions. The measured package source remains `4eff776499ca70164e241cdf4faeecca352fc508`. Subsequent changes affect the ROM checker, tests and documentation. The stock invocation omits Nightly version and Git label injection, so no new package was built for the test-only merge.

| Model | Free XIP | Pool margin above 8,192 B reserve | Calibrated eight-second peak | Late halves / xruns |
|---|---:|---:|---:|---:|
| Elite, populated bandpass scene | 1,368 B | 732 B | 7,467 µs | 0 / 0 |
| Pro, populated highpass scene | 1,380 B | 732 B | 8,281 µs | 0 / 0 |

Both native stock packages passed standard package, model, and flashmap checks. Both emulator playbacks exercised both drum kits, the two Sound owners and BREAK with zero skipped cycles. The Pro guest readback confirms the prepared highpass coefficient selection and full sub level. The timing window is eight seconds. It does not establish sustained ten-minute performance or an active native factory chorus through an original reference.

The incoming Sound changes overran the all-`-Os` stock XIP limit by 1,604 B Elite and 1,592 B Pro and left 732 B above the pool reserve. Keep `-Os` on 808 and use `-Oz` on 909 and BREAK in this exact stock composition. The focused 909 and BREAK PCM matched. This gives 344 B Elite and 356 B Pro above the agreed 1,024 B XIP reserve floor after the native Remote binding.

## Production protocol exercised in software

With the guarded candidate build enabled, both native packages execute HELLO, OPEN, READ 75, addressed SET 76, fresh READ 76, guarded inverse SET 75, fresh READ 75 and CLOSE over emulated USB MIDI. Both refuse the stale write after CLOSE. HELLO advertises the source-owned catalog checksum `ecca5979` and one control, the 909 MIX Level (ID 1, range 0–127). The native HAL reads the AC79 RNG registers for a per-boot correlation token. The emulator starts each scripted launch with a deterministic RNG seed. The C editor test separately verifies that different boot entropy invalidates the prior address, a stuck RNG leaves HELLO silent, and the real producer rejects saturated, pre-reset and stale A–B–A writes. The token is ephemeral. Existing project state and queue rules gate writes. The candidate has no holds or DOWN/UP operations.

Pro's standard ROM guard initially interpreted two annotated relative call displacements in a switch table as absolute ROM addresses. The corrected guard checks the verified PC-relative target and retains the previous handling for unproven forms. The two negative reports disappear on the exact Pro disassembly, the Elite disassembly remains clean, and 34 parser tests pass including an actual relative call into ROM. The guard change is included directly in the Integration branch.

The browser and packed TouchOSC software traces exercise the native editor through its stdio host adapter. The production package traces independently exercise the same control over emulated USB. The shipping reviewed catalog is still closed. Independent admission, on-head actual client routing and final source-pair qualification are pending. The current runner also fails before LeakSanitizer can recognize the required 123-byte control. Both final combined gates and strict ten-minute playback remain pending.

## Replay

Reconstruct `3fdd05d077b5eb1cb0a50d362e26a955a0e2337e` from [the earlier size packet](../stock-size-344fbfe/README.md). Merge exact Sound `0ac658008639c8a27ce38facd6d058a3fbc60744` with no history rewrite and verify tree `9172bbf18817eb4bdd27ce0637a963326bb4bfbd`. Apply `native-composition.patch` to the merge from a disposable clone. Before the firmware UI and lane ledger merges, this reconstructs tree `4ac7e50ddae6ff4f2fe5c3aaab4154c8e7dad88b`. The measured application is identical in production source because that tree changes only the post-link ROM checker and host tests relative to `4eff7764`. Merge `86e0f63f` and `4fa5e29f` in that order for the recorded candidate tree. The full IDs and path checks are in [source ancestry](source-ancestry.txt).

Use the prior stock build flags and add `X0X_DNR2_NATIVE=1 X0X_INTEG_LATEST_SOUND_FIT=1`, retaining `X0X_DNR2_SCRATCH=1 X0X_INTEG_DRUM_SIZE=1`. Run `MODEL=elite sh build.sh` and `MODEL=pro sh build.sh` with the pinned inputs. Compare the resulting `.fwsc` and ELF files to [the tested hashes](native-packages.sha256). From the firmware root, `sh` the supplied `run-native-identity.sh` for lifecycle and queue assertions. Invoke `python3 verify_native_dnr2_usb.py --emulator PATH --fwsc PATH --output NEW_DIRECTORY` once per model with the pinned public emulator. It uses only a local emulated USB bus.
