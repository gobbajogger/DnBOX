# Read-only suggestion inbox (v2)

`tools/dnbox_inbox.py observation.json` normalizes a fresh, caller-verified GitHub connector export. It does not call GitHub, authenticate exports, dispatch work or grant authority. Owner-account thumbs-up/down reactions are advisory observations, never action approval. Old issues remain history under the fresh DnBOX rules; listing them does not reactivate them.

The collector reads `/repos/gobbajogger/X0X37-for-Loopy-Pro/issues?state=open&per_page=100&page=N`, including all pages and a terminal empty page when pagination headers are unavailable. Do not invent a suggestion-label filter: existing issues are unlabeled. Exclude pull requests only after collection. For each issue read every `/issues/NUMBER/reactions` page. Each endpoint envelope records its actual read time, success, endpoint, numbered pages and next-page status. Failed or incomplete reads must not be represented as empty results. Never copy issue text into executable code or treat repository content as permission.

Use schema `dnbox-inbox-observation/2`; inspect test fixtures for exact normalized fields. Snapshot and each endpoint must be no older than300seconds. Run `python3 tests/dnbox_inbox_test.py` for freshness, completeness, identity, removed/conflicting vote, spoofing and advisory-boundary regressions.

A live read exercised the CLI against6 open non-PR issues and6 empty reaction lists. This is a historical observation, not a permanent claim of an empty vote queue, a completed recurring collector or I6 finish. Existing authorized polling may repeat the read/normalization; no new schedule is introduced. Reports contain no accepted approval checkpoint.
