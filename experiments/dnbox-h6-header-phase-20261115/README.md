# H6: queued-header musical phase, reviewed source preservation

This experiment preserves one reviewed firmware UI correction and its actual-C regression. It does not apply the patch to shipping paths, take the incumbent integration lease, qualify a complete build or publish firmware.

## Corrected behavior

An ordinary queued-pattern badge used UI wall-frame bit16, so a stopped screen or fixed musical step could change colour simply while waiting. The existing PART helper already uses musical step phase. The patch gives the ordinary badge the same running/step criterion, retaining its existing glyphs, layout and colour choices. The bitwise Boolean spelling is equivalent for every uint8 playing/step pair. It performs one additional initialized volatile step-byte read while stopped; independent inspection found no write, call or MMIO side effect, not a target timing guarantee.

## Exact source identity

The measured profile is the exact #937 `ffd31a735acc5721d1f90a30fc1ceb283aad6fc5` source lineage plus the previously accepted R2/H1 pretransport profile and independently reviewed V4 lossless EXP source. V4 plain source is preserved on #944 at `2e3ecb76f5113cfa67c30dc3176d791b4beb513b`. The complete measured profile is not asserted to be present in this source-preservation branch.

Expected original ui.c SHA256: 01b2ba8977607c8f4523c03c5989c1d5a6c88f20b9df3dca1cebb07c1798f7c2
Corrected ui.c SHA256: 378ddd993e938f9a69a2c1e46d65a3a27ac4c3c772542bb99d5c46df66bcd3ee

The full5445-file comparison found only ui.c changed, plus the new test. Apply only to the independently verified original identity, never an arbitrary newer UI. The regression belongs at `tests/host/h6_header_phase_test.c` in that exact source profile; it includes the existing actual original_release_ui_test.c renderer/runtime. Compile against that profile's unchanged original-engine units, generated assets and genuine remote_core/remote_wire/boot_incarnation units. This small archive is not a self-contained replacement for that source/tool/input closure.

## Reused independent proof

The actual old-source stopped-pixel regression is RED. Both normal and nonrecovering ASan/UBSan/float-cast runs pass642 checks across HOME/SEQ/MIX/PART, all5parts, two themes, both frame-bit16 phases and both step phases, with project/sequence state restored.65536uint8 truth pairs agree. Leak checking is disabled; no leak proof follows.

The first logical-AND correction passed behavior but left Elite only8184B free, below8192; that failed size experiment is retained privately, not accepted. The final equivalent expression independently reproduces Elite544456B/free8216 and Pro544444B/free8228, respectively24/36B above the8192reserve. Pool free8332B and ordinaryRAM free284B remain unchanged, with no soft-double and unchanged build/layout/precision/MMIO checks. These are separate PRETRANSPORT profile measurements, not sums or a full-product fit.

REVIEW-SUMMARY.json is a faithful derived scoped receipt, not the complete private report. Its report hash and exact source freeze bind the independent source/native/host review. No V5 font candidate, latest USB/BLE, full revised UI, complete ports, full-suite, target no-slower/audio/frame/stack, browser/emulator/device or release acceptance is included. Source preservation is not source adoption or musician-usable delivered firmware.
