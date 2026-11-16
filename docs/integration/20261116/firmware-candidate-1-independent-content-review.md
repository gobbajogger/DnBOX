# Firmware candidate 1: bounded independent source/content review

## Disposition

**SOURCE IDENTITY/HYGIENE: scoped PASS. PUBLICATION PAYLOAD: HOLD until four CSV newline mismatches are corrected.**
This is not release, merge, full UI, native, audio/timing, device, cleanup or all-page acceptance.

## Exact reviewed inputs

- Base commit, freshly authenticated through GitHub API: `8a0ef30ca3e60e9c2c2721f1c3b7b303e12b229b`.
- Its advertised tree and independently reconstructed full base tree agree: `0bff565aab165b1750535a6e1cf9cbd805e1f20d`.
- Public manifest SHA256: `19bb2f9a8df94b7afbff83705ad029f77507715ffe4d6097aa715c123e96dbb8`.
- Source manifest SHA256: `dc745ce0a3e66f46497a58e6f0670a89b2bf0619771889bbbfc5cd5e81b6ec5f`.
- Expected candidate tree reconstructed from the exact 52-path manifest over the base: `e2d8c292d8ebce25b37af6348f5123df55e2073f`, 5,396 leaf blobs. This is a local expected tree, NOT a published tree receipt.
- Resource source freeze content pin: `5070f079238683340c58a74ea8361caf4898e350188aaab5557717a9b14c42b5`; SOURCE-FREEZE.json SHA256 `07a3a6821b76c7b502070aeeabdfaa2eb71cc360b5a38d4f978c7de39ddb5d48`.
- Gate commit, freshly authenticated: `37c780cdf4b18e824c863d7cc2982792882f61cb`, tree `565d6725e8da95f745975eccddf7b22c7acc9659`. Fresh recursive tree was untruncated.

## Checks actually performed

1. All 52 manifest payloads match their local SHA256, Git blob SHA, byte count and executable mode. All 5,344 unchanged base payloads/symlinks and modes match the pinned base. No unexpected base loss.
2. All 14 resource overlay files are byte-equal to resource-review-freeze-1 and match the independent SOURCE-FREEZE entries (hash, size, mode); no differences in that overlay set.
3. All 21 gate files match both public-source-pins.json and the freshly fetched untruncated authenticated gate tree (path, Git SHA, mode).
4. Read the production source diffs against the 8a base: ui.c is only the bitmap-width accessor substitution; gfx.c adds/reuses the accessor; Chicago/Miami program changes are the already frozen compact-value adapter. Remaining resource generator/tests are exactly the accepted frozen payloads. No live UI/Sound source was taken over.
5. New public changed files were checked for credential-like markers, private-key markers and internal absolute workspace/agent paths. No hits. Existing licenses/provenance were not rewritten; no new SDK/binary payload is in this 52-path source candidate. This is scoped hygiene, not proof against every possible secret format in all historical repository bytes.
6. Seven sanitized logs retain a matching SHA256 pointer to their original raw log. Recorded mechanics are 15+6+2+18 = 41 tests. Font and exp-curve recorded exit 0. Compact-program log retains 796 exact checks followed by LeakSanitizer fatal/exit 1 and CalledProcessError; result JSON records exit 1. It is not described as a sanitizer pass. No suppression or test expectation change appeared in the candidate.
7. README visibly disclaims qualification. State and focused-result scope retain full host/native/UI/audio/timing as NOT_RUN, distinguish copied changes from landed merges/ancestry and leave merge/deletion under the triple-check gate.
8. API tree-payload path set equals the 52-path manifest, but four content bytes differ (finding below). Generated build files and Python caches remain outside both payload and public manifest.

## Blocking finding: four CSV payloads are newline-normalized

`prepare_public_wip.py` uses `Path.read_text()` when preparing create-tree content. Universal-newline decoding converts the candidate CSVs' CRLF bytes to LF. These four uploaded blob identities therefore would not match the reviewed manifest:

| Path under docs/integration/20261116 | Manifest/candidate Git SHA | Current payload Git SHA |
|---|---|---|
| firmware-open-prs.csv | 37a93109c936c957b30b6b7bf70324b0d3aa9a47 | 251af31e5fae02d21b7fc3689425b9c6541bf925 |
| firmware-branches.csv | 1ab781e55c875d13a974b6b6d269ba2b1ae19744 | 7d2740bf2dc0e0187f42b76e83ea70301c47b73f |
| website-open-prs.csv | 8db8f057e6c3ab943b263e42f9f6033abebb0598 | 2dd2491002650a6f27458397758b5a94d9c65b48 |
| website-branches.csv | aa5587269292c63ff1272fe66bb642472bc4016b | 2ba4adb46fb99e1d947ca21193013686f47cfb60 |

Owner: repository steward. Next action: preserve exact decoded newline bytes, or create each exact blob and use its SHA; regenerate/check the payload, then independently read back the resulting tree/changed blobs. Do not report expected tree e2d8c292 as delivered before that check. This mismatch does not affect the 14 resource or 21 gate source payloads.

## Requested documentation corrections

- CONSOLIDATED-STATE.md line 66 first grants all assets/coefficients/captures/binaries Git authority except secrets, then still says “excluded originals/binaries remain Library-only.” Remove/clarify that stale blanket phrase so the new owner direction is not contradicted.
- Line 15 points readers to `repo-inventory/`, which is not present in this public candidate. Identify the published CSV location `docs/integration/20261116/`; distinguish locally retained raw API pages from published files.

## Review limits

No full builds, browser, native, audio, device or project tests were executed by this reviewer; existing logs were inspected, not rerun. No source or Git mutations. The historical broader qualification receipts and asset/license assertions were not independently re-established in this bounded review. Effective service model/effort identity is not exposed. A changed manifest or source after these pins requires the affected recheck.

