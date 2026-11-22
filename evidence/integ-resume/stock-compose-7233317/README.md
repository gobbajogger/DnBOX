# Scratch stock composition

Measured Integration `7233317f`, Sound `eeb91398` and the final FWUI input from [#987 comment 7073368435](https://github.com/gobbajogger/DnBOX/pull/987#issuecomment-7073368435). The FWUI base is `22a34d6e` and its final `ui.c` is blob `f7104b3f8a7a87d7f03daaa7845d7f513f9213f4`. The local FWUI commit and public `48651ff7` pin the same full tree. The earlier FWUI fit result was superseded. All three input histories are ancestors of the scratch merge, with zero deleted paths from each.

| Scratch configuration | Elite XIP free | Pro XIP free | Pool margin on each |
|---|---:|---:|---:|
| Plain composition | 1,000 B | 1,008 B | 7,908 B |
| Both Sound engines, full Sound effects routing and bound Remote scalar | −3,432 B | −3,424 B | 7,908 B |
| Same with the lossless boot tanh table | −472 B | −460 B | 3,804 B |

The stock floors are 1,024 B free XIP and 512 B pool margin after the 8,192 B reserve. Both plain links emitted stock-size packages with valid CRCs and stock partition metadata, but miss the XIP floor by 24 B and 16 B. Both bound configurations failed the normal stock linker. The diagnostic runner widens only a disposable copy of the application linker script, measures its ELF sections against the original window and exits before app or package creation. The boot table recovers 2,960 B Elite and 2,964 B Pro at a 4,104 B pool cost per model. It preserves the pool floor on this composed input. The full profile still requires 1,496 B Elite and 1,484 B Pro more XIP to meet the floor. Retain the boot table for measurement while stock activation remains held.

## Software checks

The bound scratch profile steps the production editor through address, maintenance, project and boot-flash gates. Its 909 MIX binding writes the authoritative mirror only after the single producer queue accepts the update. A separate audio tick proves consumption. The real host path passed Apply, duplicate replay, Undo, full-queue refusal, panel A–B–A rejection and project adoption revision invalidation. Nightly candidate `995d5493` exercised the actual Apply and Undo module against this same real host binding and confirmed the guarded readback sequence. The catalog and timer incarnation are software-fixture inputs. A production catalog, boot identity, full writer coverage and final combined TouchOSC remain unqualified.

The merged BREAK test kept gain, interpolation and the bit-exact packed table. Its registered three-case law test passed after the separately recorded include-path fix. Existing shipped offline five-sample package evidence stays attached to its own source and packages. The two firmware offline browser registrations passed with pinned sandboxed Chromium and external temporary storage. The six compact-program input blobs still match the earlier genuine positive LeakSanitizer proof. The fresh executor-local sanitizer run fails during LeakSanitizer initialization. Full Local gates and strict ten-minute timing await the final candidate.

## Reproduce the measured source

Use a disposable Git checkout with the three input histories. Pin [public FWUI](https://github.com/gobbajogger/DnBOX/commit/48651ff773b72ef1fdbb097376dc1b0d90ab3f52) and verify its tree equals the local FWUI tree in [the machine receipt](receipt.json). The verbatim local FWUI and first merge commit objects can be restored locally with `git hash-object -w -t commit FILE`. The first merge tree can be reproduced with `git merge-tree --write-tree 7233317f d12c86f3`. Then `git merge-tree --write-tree --messages 5f1e4de7 eeb91398` returns the conflicted virtual tree in the receipt. Load that tree into a disposable Git index and apply `sound-resolutions.patch` to reproduce the plain tree. Apply `bound-scratch.patch` for the bound source. The separately supplied `break-harness.patch` affects the test fixture alone. [Replay verification](replay-verified.log)

Use the pinned JieLi compiler and exact SDK already shared for Integration. Run `MODEL=elite|pro X0X_SYNTHONE_PATCH=1 X0X_DEV_PADCAL=0 sh build.sh` with `X0X_SUPERJ6_FACTORY=0` on the plain tree. On the bound tree set `X0X_SUPERJ6_FACTORY=1 X0X_COMPOSE_S1_EFFECTS=1 X0X_DNR2_SCRATCH=1` and measure `X0X_TANH_BOOT_TABLE=0` and `1`. After a stock linker rejection, run `python3 -B measure_failed_stock_link.py --board smk37 --model MODEL` with the same options. The captured hashes identify the two plain scratch packages. They are not promoted to release artifacts.
