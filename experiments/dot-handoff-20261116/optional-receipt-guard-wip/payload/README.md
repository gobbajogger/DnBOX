# Offline scheduler receipt evidence guard

Optional read-only analysis aid for the existing schedules/ledger, not a new tracker, permission gate, monitor or guaranteed service. No external calls, schedule writes, retries, message sends, ownership changes or file writes are performed by receipt_guard.py.

Checks exact latest-result/run identity and creation after the actual attempt, preserves failures/truncation/action-required, and keeps completed-run, worker liveness, source-stage progress and user delivery separate. Source hashes mentioned in prose are references, not verified live heads. Exact-schedule overdue observations are checks to investigate, never proof of a missed run or permission to run again. Unknown results stay no verified progress.

Structured per-run stage receipts require exact run binding, an outcome, accountable owner, next action, and evidence for claimed source preservation/consumer intake. Structure is not proof of evidence authenticity, product completion or delivery; inspect the actual referenced input/result. Keep nonpublic metadata private.

The first RED invocation was a missing-file import setup error and is retained in RED.log. After correcting the harness, RED-MISSING-FEATURE.log has23 real missing-feature assertion failures. GREEN.log and GREEN-OPTIMIZED.log each have25 passing offline tests. They cover null/mismatched/prior-retry/future results, invalid timing, empty/truncated/failure/action-required states, early nominal start, no input mutation, unknown history, overdue/disabled classification and staged evidence structure. This is bounded producer proof, not independent review or an external scheduling guarantee.

ACTUAL-HOURLY-CLASSIFICATION.json applies the code to the retained actual15:52/15:56 service snapshots. Matching complete results are present, but verified_product_progress=false and delivery=unknown. The result bodies separately establish exact#887f89f documentary fallback consumption, not firmware or user delivery. Earlier14:52/14:55null outcomes are not retroactively proven.

Run: PYTHONDONTWRITEBYTECODE=1 python3 receipt_guard.py SNAPSHOT.json --observed-at ISO8601_WITH_TIMEZONE
Tests: PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s . -p 'test_*.py' -v

Keep all earlier receipts and denials; no automatic retry, actor switch or replaced schedule follows from this aid.

R2 adds a concrete next-action for early unknown-history/missing-result returns. Two added tests exposed missing output fields; failure retained in RED-NEXT-ACTION.log, original eight-file R1 retained unchanged. Current25-test suite passes normally and optimized; actual older null snapshots correctly say no verified progress with recovery action, while new matched snapshots remain recorded-result-only.
