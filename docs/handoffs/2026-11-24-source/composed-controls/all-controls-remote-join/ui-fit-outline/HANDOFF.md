# Combined UI fit outline (source + diagnostic map)
Apply ui-fit-outline.patch against guarded-fit/tree's exact UI/Undo sources. The patch only touches firmware/src/app/ui.c and undo.c. No layout, audio, package, page capacity, feature flag, admission policy, saved format, artwork or Remote route changes.

Fresh identical Elite/Pro PI32 guarded link recipe: ./run.sh. Link is expected to refuse oversize firmware; no device image is generated. The hashes in UI-FIT.sha256 identify the final patch, inputs and failure maps. No behavioral, emulator, hardware or release verification.

| Arm | Elite overflow | Pro overflow | Net saved per board |
|---|---:|---:|---:|
| Prior guarded exact | 35,536 B | 35,528 B | 0 |
| noinline-only diagnostic | 35,536 B | 35,528 B | 0 |
| packed owner-bound page rows | 29,936 B | 29,928 B | 5,600 B |
| plus shared typed write dispatch | 28,668 B | 28,660 B | 6,868 B |
| plus shared Undo callbacks | 27,932 B | 27,924 B | 7,604 B |

New owner row words expand to exact original kind/axis/owner/slot. Static branch predicates, labels, page order, auxiliary counts and query path remain shared with add_page. The typed setter table keeps original callback bodies/return results and the original common descriptor, range and read-only ordering; special scale, limiter, delay, and stock paths stay separate. Undo maintains preflight field checks and ready/apply order; outlined shared code only calls when changed, passes the exact original mask, and releases audio/edit hold with the same failure message. No late-counter or timing change intended, not tested.

POOL _pool_end unchanged at 0x01c72a10 both models. Both APP links still fail; remaining deficit is ~27.3 KiB even on the guarded diagnostic map. Source reconciliation of other parallel fit changes can change map ordering, so independently pair the final joined maps rather than adding savings arithmetically.
