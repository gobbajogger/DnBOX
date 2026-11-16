# DnBOX suggestion inbox

Use the existing [GitHub issue inbox](https://github.com/gobbajogger/X0X37-for-Loopy-Pro/issues) and [new issue page](https://github.com/gobbajogger/X0X37-for-Loopy-Pro/issues/new). Describe the musical problem, expected improvement, current workaround and affected controls/model. Do not post credentials, private recordings or unrelated personal information.

The owner may request a feature or approve its precise scope with a 👍 on the opening post. Community votes inform discussion; they do not authorize implementation. The coordinator checks actual owner identity, exact scope, usefulness, existing functionality and control/model/data/transport conflicts before assigning work. A request created automatically with the owner's account cannot approve itself. Source, tests, integration, publication and usable delivery are separate stages.

## Current implementation and limits

`feature_inbox.py` adds the missing read/run boundary to the pure `feature_intake.py` decision helper from merged PR #184. It performs real work when its `read(path)` callback calls the supported authenticated GitHub connector. It does not create a daemon, schedule, webhook, issue, reaction, external message, alternate ledger or new dispatcher. All URLs are generated internally for the one fixed repository. Issue prose is data, not executable instructions.

The supplied command line replays exact connector captures locally. This is reproducible real-read evidence, not by itself an automatic deployed service. The existing coordinator/root must run the live reads and consume eligible work through its legitimate current one-writer state. A scheduled prompt, launched task, draft PR or source merge does not prove that loop works.

On November 15, 2026, the old `coordination/merge-queue` ref was unavailable and the historical `safe_merge_intake` automation lookup reported no longer available. These particular lookups do not establish that every current intake owner is absent. Preserve existing schedules and canonical roadmap ownership while the root reconciles continuity. Never initialize an empty checkpoint or ledger merely to get a passing result.

## Live connector contract

A trusted coordinator supplies:

- `read(path)`: actual authenticated GET result `{status:200, observed_at:UTC, data:JSON}`. Paths are under `/repos/gobbajogger/X0X37-for-Loopy-Pro`. Do not derive HTTP success or pagination from a short excerpt. A failed tool read is a failure, not an empty page.
- The exact current relevant source closure, with full Git/SHA-256 identities.
- Separately authenticated `receipts`, explicit owner `cancellations` and source-backed `analyses`, following `FEATURE_INTAKE.md`. No receipt is extracted from a body, comment, title, reaction count or login string. The original helper's scope digest retains exact title/body/spec bytes.
- The current accepted ledger/checkpoint, authenticated and validated by their sole owner. Intake capacity defaults to zero; pass only verified free slots. A capacity-bounded batch does not authorize overlapping source writers. Receipt authenticity is a trusted caller boundary, not something arbitrary uploaded JSON can prove.

For a live callback, pass `now` as a trusted UTC clock callable. It is sampled after each read and at collection end; every recorded read is rechecked against final time and again at decision time. Fixed timestamps are for explicitly historical capture replay only.

Trusted cancellation inputs contain exact repository ID, issue ID, owner actor, scope hash and native owner evidence reference. They are normalized into `owner_cancelled`; raw comment text never creates cancellation authority.

The collector reads repository identity, all issue pages, details, all timelines/reactions and a second detail/collection pass. Pagination continues through an actual empty terminal page, including after a short page. It rejects detected scope/state/membership races, duplicate issue pages, wrong identities, failed/stale/future reads and pagination limits. Two passes detect observed races; they are not a transactional GitHub snapshot.

No raw vote becomes an implementation grant automatically. A new opening-post vote needs its real stable reaction ID/owner and a trusted exact-scope receipt. A surviving old vote is not approval of later edits. Exact previously accepted approval can survive reaction removal; material edits and explicit cancellation/reopen require the corresponding maintained evidence. A verified human owner request needs no redundant vote.

`collect` returns a fresh observation only when required reads finish. `run` returns `DISPLAY_ONLY` with no claim when current continuity is unavailable, or delegates to the existing eligibility helper, with its valid nullable-body correction when supplied. `UNKNOWN` does not mean an empty inbox. Read-only display lists actual issues and observed owner opening-post +1s without presenting them as accepted work. Markdown output escapes untrusted markup and title line breaks.

## Run and test

Python standard library only:

```sh
python3 -m unittest discover -s tests -p 'feature_*_test.py'
python3 coordination/requests/feature_inbox.py connector-capture.json \
  --source source-closure.json --now 2026-11-15T23:41:37.342Z \
  --ledger current-requests.json --checkpoint current-checkpoint.json \
  --trusted-analysis authenticated-analysis.json \
  --trusted-receipts authenticated-receipts.json \
  --trusted-cancellations authenticated-cancellations.json \
  --capacity 0
```

A capture is an array of real GET records with `path`, `status`, `observed_at`, `data`. Each repeated endpoint requires a distinct response; one saved response cannot impersonate the second read. The timestamp is the end of that capture, not permission to claim an old capture is current. Exit 2 means unknown, display-only or an unresolved eligibility gate. Exit 0 means a complete decision, not dispatched or delivered work. `--markdown` prints the local readable inbox.

The actual live-read receipt is `docs/evidence/suggestion-inbox-120/live-read-20261115.json`. It records 56 GETs, two 949-entry scans, six real issues, all 543 timeline events for #832 and zero opening-post owner reactions. The result is deliberately `DISPLAY_ONLY`, no claim, because live continuity is unavailable. Synthetic vote/dispatch tests are labelled separately. No real owner vote, new eligible claim, receiver execution or automatic downstream delivery was fabricated.

## Existing-owner run wiring and completion

The root/current intake owner consumes this code at an immutable reviewed commit, obtains fresh live reads, validates trusted continuity/authority, and assesses requests against current source. It must preserve existing IDs, tombstones, accepted scopes and busy writers. Only the legitimate single writer accepts a proposal, re-reading continuity before its actual write. It routes the exact scope/source/evidence to a capable current owner and verifies that owner's first substantive action. No second ledger or timer is needed merely to hold this handoff.

The requested #120 outcome remains open until this real chain is observed: an owner-authenticated exact request or vote; one accepted existing/new mapping without duplication; actual consumer work; a subsequent run continuing the changed state without replay; and the requested checked usable result or a precisely stated remaining delivery gate. A bodyless issue, unrelated failed request, removed vote, cancellation, close/reopen, source edit, duplicate tick and unavailable read need their covered safe dispositions. Never mark the full outcome complete from code/tests or this read receipt alone.

The capacity-aware helper/doc/test behavior is preserved from the later authentic coordination source at `4a35a6cdaa0f112db6b3f577542138afd170c20d`, not just the initial #184 merge. Zero capacity cannot issue a new claim. Existing request continuity still requires the current legitimate single writer; neither recovery of an old snapshot nor source acceptance activates it.
