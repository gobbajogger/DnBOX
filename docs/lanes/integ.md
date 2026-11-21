# Integration baseline

Firmware draft [#976](https://github.com/gobbajogger/DnBOX/pull/976) targets `integration/dnbox-consolidated` at `76040daa5b52d686a8b7b544296ba5d79610a95d`. The branch ruleset rejected `lane/integ-baseline` (422) and allows `lane/integ`.

| Item | State and evidence |
|---|---|
| I0 | **Open.** The first consolidated run stopped during host tests. The complete repeat at `76040daa` failed only the host stage (478 passed, 23 failed). Both stock builds, flash map, budgets, UI checks and retired-engine scan passed. The earlier 501-pass [PR #970 run](https://github.com/gobbajogger/DnBOX/pull/970#issuecomment-6980748381) used the same tree at a different commit. |
| I1 | **Pass at base.** The [source ledger](../../CONSOLIDATED-STATE.md) names all ten merged PRs, their source and merge SHAs. GitHub compare confirmed each merge is in this base. |
| I2 | Stock Pro and Elite built with SDK 1.2.14. The consolidated builds had 1,188 B and 1,180 B of XIP headroom respectively. On the current combined source, Pro has 6,528 B and Elite has 6,520 B. Both retain 780 B above the 8,192 B pool reserve. Final combined builds depend on [Sound #973](https://github.com/gobbajogger/DnBOX/pull/973) and [FWUI #975](https://github.com/gobbajogger/DnBOX/pull/975). |
| I3 | **Open.** All 16 ordered pairs of the four available bass banks completed heavy playback in two workloads on the local combined source. On the current stock zero-loop configuration, two independent sweeps recorded one and eight misses respectively in 32,768 blocks per sweep against the 11,609 µs thread-CPU limit. The earlier host tests used two generated loops. [First stock run](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058637831) · [Stock replay with compiler include trace](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058821910). |
| I4 | **Pass at consolidated `76040daa` and lane `f394ac03`.** Stock Linux ran the original test untraced as uid 1000. It detected the deliberate 123-byte leak, passed both sets of 796 checks and rejected corrupted data. [Consolidated log](../../evidence/integ-baseline-76040daa/compact-programs-linux.log) · [Lane log](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7056736948). |
| I5 | **Partial.** Five live previews, both URL forms per sample, archive generation and byte-exact simulated restore work. The exact 109,394 B native five-zone pack cannot fit either stock application at the available aligned headroom. See the measurements below. |
| I6 | Owner requests 26.8, 26.9, 26.11 and 26.12 appear in the roadmap. Complete/Working/Pending are checked against the pinned consolidated SHA. [Nightly #170](https://github.com/gobbajogger/DnBOX-nightly/pull/170) contains the feature inbox and vote reader. This branch applies the existing enhancement label to new requests submitted through the issue form. Authenticated end-to-end vote verification is open. |
| I7 | The owner retired the old Pages installer paths and kept GitHub’s automatic repository redirects. All current installer links use [stable](https://gobbajogger.github.io/DnBOX/install/) or [nightly](https://gobbajogger.github.io/DnBOX-nightly/install/). No legacy-name site or repository is required. The [read-only inventory](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058900972) traced all 488 historical tips and all eight protected PR heads. Those eight PRs closed unmerged on 17 November. Each pull head and archive tag still resolves to its original commit. Firmware had 11 branches and 1,188 tags, nightly had eight branches and 161 tags. Branch cleanup follows integration of every lane. |
| C1 | **Open.** At `f394ac03`, both stock models built and the unmodified gate passed its strict prerequisites, no-loss and repository limits. The first stock Linux guest was killed during host simulator compilation. On the later revision `d6244db5`, the stock guest built the complete host simulator and began the first host case before supervision ended. Read-only disk recovery found zero completed named cases and no terminal gate result. [Earlier guest log](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058191242) · [Recovered d624 gate and compiler output](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7059195374). |
| C2 | Straight descendant of the consolidated SHA. This work removes no tracked file. |
| C3 | Independent final review at every required size is open. |
| C4 | The [tracked-file scan and 50 MiB control](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058009663) passed on `f394ac03`. Both integration trees contain the owner-approved issue form under `.github/ISSUE_TEMPLATE/`. |
| C5, C6 | Checklist still has open work. Draft remains open. |

The [recovery index](../LIBRARY-INDEX.md#consolidated-firmware-baseline-evidence) identifies the two baseline archives and both verification manifests by filename, byte count and SHA-256.

### First consolidated Local run

The gate registered 500 host checks. Before interruption 247 passed and 26 failed. The other 227 produced no terminal result. The Elite package prerequisite was absent when registration started. Both stock packages were available for the repeat. [Baseline log and retest discussion](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7055474361).

| Failure in the interrupted run | Separate exact-source retest or cause |
|---|---|
| `gate-trap`, `dev-validate`, `dev-validate-composed`, `offline-browser-safety`, `offline-sequencer-browser`, `offline-touchosc-lua` | Passed after correcting checkout/temp placement, reference inputs, the browser environment and the Lua 5.1 runtime. |
| `fxbus-coef-cache`, `chicago-peaking-coef-cache`, `chicago-ens-hoist`, `chicago-modulation-hoist` | Passed once the runner had the pinned historical Git objects. |
| `compact-programs` | Passed on stock Linux with LeakSanitizer active, recorded under I4. |
| `composed-midi-epoch`, `chicago-fx2-inserts`, `original-sound-cell`, `sound-reset`, `panic-notes`, `drum-fill`, `cq-reclaim`, `optimization-equivalence`, `ble-midi-receive`, `ble-capture-replay`, `ble-device-setting-flow`, `ble-toggle-persistence-gate`, `ble-role-switch`, `chicago-span-hold`, `fma-exact-identity` | The sandbox runtime could not attach the sanitizer helper. Repeat on a Linux runner that passes both the clean and deliberate-leak controls. |

The repeat at the exact consolidated SHA also reported `panic-clear`, `original-release-integration` and `original-release-ui` as failures on this runner. Each log identifies the sanitizer helper’s failed attach. The candidate’s original-release-integration executable completed on stock Linux, including all 12 expected images. Nine complete host scripts also passed in that guest. The complete repeat is recorded below. [Sanitizer evidence](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7056160192).

A separate FREE PLAY framebuffer test passed the positive render and then exited 128 fetching an unpublished Git object. The repaired test exercises missing and unexpected cue mutations through the native renderer. Its missing-cue result matched the reachable historical predecessor in 32 of 32 comparisons, across 154,880 RGB565 bytes (SHA-256 `6e617d6c451e7b62e3bbd37fc4528f7f8279cec4abf9dbee2d3643872e115a07`). The complete repaired wrapper subsequently exited zero.

The first no-loss step failed because the checkout lacked a parent object. The missing history was restored. The generator scheduler also repeatedly tried to create a lock directory while another job held it. A focused test observed 32 rejected attempts across eight waiters. The runner now uses one blocking FIFO token. All eight waiters completed, and independent capacity, registration and admission tests passed. Host compilers, sanitizer settings, timing limits and firmware code are unchanged.

### Consolidated repeat and stock Linux qualification

The full Local gate at `76040daa5b52d686a8b7b544296ba5d79610a95d` exited 1. The only failed stage was host tests. Pool accounting, clean tree, setup, no-loss, repository limits, both stock builds, flash map, budgets, UI defect checks and the retired-engine scan passed. The offline runner used its pinned base and skipped the network fetch. The [45,892-byte gate log](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058434593) has SHA-256 `2c9b8e660bac970b0f96ec8fd87ea9a79fda1f42cf5b6ad0a1d869195a35aa7a`.

The consolidated repeat finished all 501 registered host cases. It recorded **478 PASS, 23 FAIL, 0 NOT_RUN and 0 skipped**. Its 25,592-byte host log has SHA-256 `41d85ab2843ed2cb41c70d5cd3ff21bcb0ff6497756961f82b2f87946e527d49`. This run covered all 500 originally registered cases and the additional Elite package check, which passed. Of the first run's 227 unknowns, 220 passed and seven failed. Ten of the first run's 26 failures passed on the repeat. Across the two runs there were 33 distinct failing cases. [501-case crosswalk](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058435097) · [Individual failure diagnoses](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058435595).

| Outcome in the consolidated repeat | All affected host cases |
|---|---|
| LeakSanitizer could not attach on this host. | `ble-capture-replay`, `ble-device-setting-flow`, `ble-midi-receive`, `ble-role-switch`, `ble-toggle-persistence-gate`, `chicago-fx2-inserts`, `chicago-span-hold`, `compact-programs`, `composed-midi-epoch`, `cq-reclaim`, `drum-fill`, `fma-exact-identity`, `optimization-equivalence`, `original-release-integration`, `original-release-ui`, `original-sound-cell`, `panic-clear`, `panic-notes`, `pattern-preset-notelock`, `sound-reset`, `ui-theme` |
| Missing-curve mutation exceeded its 120-second wrapper deadline. | `ui-drum-curve-axes` |
| Missing historical source object `6c418a43ff6fc5f4e1fad3bbf71e63cfd64a1987`. Git returned 128 and the GitHub commit lookup returned 422. | `ui-freeplay-state` |

The [separate drum-curve and panic checks](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058413614) used the same consolidated source. Both original wobble configurations of `panic-clear` reported zero failures under stock Linux with sanitizers active. The normal drum-curve renderer passed all 34,294 checks. Its compiled negative rejected the missing curve at `ink>100`. The original Python wrapper later passed in full in 50 seconds with its original 120-second deadline. The integration candidate's FREE PLAY test uses actual missing-cue and unexpected-cue controls and passed native framebuffer verification. [Curve readback](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058273800) · [Independent FREE PLAY controls](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7057455036).

A separate low-memory stock Linux guest executed the real consolidated `pattern-preset-notelock`, `original-release-integration`, `original-release-ui` and `ui-theme` binaries and their original output checks. It compared all 370 normal/sanitized PNG pairs and both copies of every expected digest. These results distinguish the sanitizer attach failures from firmware behavior. [Stock Linux raw results](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058333600).

At lane `f394ac03`, a stock Debian guest passed the real leak and Chromium controls. The original limit check admitted a small Git blob and rejected a tracked 52,428,800-byte blob. With those checks working, the unmodified gate scanned 5,060 Git objects and entered host compilation. The virtual machine received SIGKILL before the first host case completed. Both the guest and external copies of the partial gate log contain 9,597 identical bytes, SHA-256 `b4b915973855f8d259781dbbf13deee6204351dc4245618a95e82182259405c4`. C1 requires the complete gate on a capable runner. [Strict controls](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058009663) · [Recovered log](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058191242).

The firmware production and host-test inputs at `07a4dfcd` matched `f394ac03`. Both [fresh stock builds and their strict preflight](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058440978) passed at `07a4dfcd` (16/16). Pro retained 1,188 bytes of XIP free and Elite retained 1,180 bytes. Both preserved the 780-byte margin above the 8,192-byte pool reserve. On `f394ac03`, the original release UI passed 35,689 checks with 99 captures in both normal and sanitized executions. All paired PNG bytes and both stdout streams matched. [Stock UI replay](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058261667).

### Owner release and link update

GitHub’s Latest endpoint and the v0.0.26 tag both identify the released source as `94197fc839b00aff47b98ca02f3c6396e4a6eabf`. The owner reports successful startup and restart on a real Elite and Pro. README, CHANGELOG, the release guide and roadmap distinguish this published result from integration builds. The published release description already links both canonical installers. All checked GitHub releases in both repositories use canonical links. The docs link migration preserves issue, commit and comment destinations under the renamed repositories. [Published release](https://github.com/gobbajogger/DnBOX/releases/tag/v0.0.26).

An uncached read of the deployed installers and their embedded catalogs on 21 November 2026 UTC confirmed that the stable channel listed 0.0.25.1 (`79280e76`). Nightly listed `nightly-94197fc8-dev`. Both nightly model checksums matched the v0.0.26 release. Its older hardware-test label predates the owner’s startup and restart results. [Stable catalog](https://gobbajogger.github.io/DnBOX/versions.json) · [Nightly catalog](https://gobbajogger.github.io/DnBOX-nightly/versions.json) · [Release and checksums](https://github.com/gobbajogger/DnBOX/releases/tag/v0.0.26).

The candidate `c125477e` built stock Elite and Pro packages byte-identical to the consolidated firmware. A separate composition of Sound `933a6f40` and FWUI `6763b056` had no three-way merge conflicts. Its diagnostic tree `ecde45dde6621a82e750a7ee223a8a5ce9f84a69` linked with XIP margins of 9,104 B (Pro) and 9,092 B (Elite) and a 780 B pool margin on each. Synth One is partial on this tree. In the Chicago/Miami host stress sweep, wall time exceeded the deadline on 22 of 3,072 blocks. All-engine timing remains open. [Native builds and measured peaks](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7055761401).

### Current stock build and sample recovery

The offline combination of Sound `03f92a7d`, FWUI `11bc3f3a` and integration `d6244db5` produced tree `5155fc01b14e4223c3a787fc3f373449bcf47ced`. Both stock builds passed. The real five-loop pack contains 109,394 bytes. Its placement calculation retains 1,024 bytes of flash headroom.

| Model | Stock image | Free XIP | Extra flash the pack needs | Pool above the 8,192 B reserve |
|---|---:|---:|---:|---:|
| Elite | 546,152 B | 6,520 B | 103,906 B | 780 B |
| Pro | 546,144 B | 6,528 B | 103,890 B | 780 B |

Both BREAK velocity tests and all seven startup scenarios passed on the combined sources, including recovery before the first display frame. The source PRs remain open. The [build logs, pack calculation and startup test](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058641850) identify the individual source commits. All 32 bank-pair and workload combinations finished using the stock zero-loop header. The compiler include trace verifies the generated input. All 32 PCM files and 32 meter/scope files match the independent stock run. The stock replay recorded eight missed thread-CPU deadlines and ten wall-clock misses in 32,768 blocks. The worst thread time was 92,119.090 µs. The independently timed stock run recorded one thread-CPU miss and fourteen wall-clock misses. [Stock replay, compiled input and raw timing records](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058821910) · [Independent stock run](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058637831).

The stock Elite and Pro packages each ran the same 30-second 360 MHz simulator test. The 140 BPM scene combined Chicago, Juno, 909 and a user-slot CRAMEN break. With observation enabled and disabled, each model produced identical PCM, RAM and virtual flash. All 774 measured intervals on each model contained a verified BREAK contribution. [Elite trace](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058994446) · [Matched Pro trace and both model controls](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7059101065).

| Measurement from 20 to 29 seconds | Elite | Pro |
|---|---:|---:|
| Completed 512-frame audio intervals | 774 | 774 |
| Missed interrupt deadlines | 0 | 0 |
| Firmware late or xrun counter increments | 0 | 0 |
| Slowest firmware interval | 8,624 µs | 8,637 µs |
| Minimum interrupt-completion slack, lower bound | 2,973.046 µs | 2,955.635 µs |

The canonical five-sample bank took 109,394 B. Three lossless alternatives measured between 97,776 B and 102,688 B before a decoder. With the 1 KiB safety floor retained, native placement leaves 5,488 B on Elite and 5,504 B on Pro for samples. [Compression, round-trip verification and stock placement](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7059044211).

The host suite also generates a two-loop factory header. In that configuration, four blocks exceeded both deadlines. Its PCM and meter/scope outputs match the earlier two-loop fixture. [Two-loop diagnostic](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058763565).

The full [stock Linux gate on `d6244db5`](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058541652) began at 06:20 UTC after both model builds. Its original limits scan passed. The guest finished the host simulator and began compiling `composed-features`. Supervision ended after the 07:12:31 UTC host sample. A read-only recovery matched both copies of the 9,623-byte gate log, verified the 1,200,840-byte simulator and found zero completed host results. The interruption had no recorded exit status. [Original output and disk recovery](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7059195374). The two model builds, all six image/loader/package hashes, strict prerequisites and pool limits were also [rechecked on the documentation commit `d1d6fbba`](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058778835). Separately, the consolidated BLE role-switch, BLE MIDI receive and composed MIDI epoch wrappers passed unmodified outside the tracer. [Complete wrapper and scoped binary replays](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058615700).

### Earlier Sound and FWUI composition

Sound `7e68fc39` and FWUI `5f80e1d6` composed against `76040daa` without a conflict. Adding integration `810867e1` required reconciling their two fixes to the FREE PLAY pixel test. The local tree `6308068178d2c1f320fde0962c4061bd52a2f3e6` preserves the inactive mode, missing cue and unexpected cue checks. Its stock Pro and Elite builds passed. Pro had 6,944 B of XIP free, Elite had 6,936 B, and both retained 780 B above the 8,192 B pool reserve. The exact five-sample bank still requires an additional 103,474 B (Pro) or 103,490 B (Elite) with the 1 KiB floor. The compiled Synth One INIT host test covered MIDI audio, note ownership, sustain, pitch, release, voice stealing and panic.

In the 3,072-block Chicago/Miami diagnostic, the chorus workload exceeded the 512-frame period (11,609.977 µs) once on CPU (14,541.589 µs). Wall time exceeded the period on 251 blocks across the three workloads. The host was shared. Device timing and all-engine acceptance remain open. The composed FREE PLAY test exceeded its unmodified 100-second compile timeout before producing a positive result. The earlier composition above remains a historical measurement. Both source PRs are still drafts. [Exact inputs, full resolution and per-workload results](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7056513397).

### Stock fit and heavy playback with four bass banks

Sound `e2e403f9`, FWUI `4c05722d` and integration `f394ac03` produced local tree `3ba62ac5e7b2b8c779c17be80e185bf702612b0b`. The stock Pro application was 546,128 bytes with 6,544 bytes of free XIP. Elite was 546,136 bytes with 6,536 bytes free. Both retained 780 bytes above the 8,192-byte pool reserve. Keeping the 1 KiB XIP floor leaves the owner's exact BREAK bank short by 103,874 bytes on Pro and 103,890 on Elite. The later Sound `96c9f154` composition had identical firmware, tools, tests and build inputs. [Model builds, size checks and packing rejection](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7057455036) · [Later source comparison](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7058041362).

The sequencer, drum voices, owner's BREAK audio, both bass parts, MIDI and effects ran together over every ordered pairing of Chicago, Miami, Synth One INIT and Juno. Each pairing ran 1,024 blocks for a master-filter sweep and 1,024 for tempo and continuous control. One pairing completed on a same-condition retry after the first attempt exceeded its original timeout; the silenced-B negative failed as expected. Eleven of the resulting 32,768 blocks exceeded 11,609 µs on the render thread. The worst was 262,311 µs for Miami/Miami. On two repeats each of the two worst pairings there were zero CPU misses over 4,096 blocks and the PCM matched exactly. The original misses remain recorded. Hardware timing and the final merged sources are still outstanding. [Complete workload results](https://github.com/gobbajogger/DnBOX/pull/976#issuecomment-7057716886).

### Five BREAK samples

The five canonical ADPCM payloads occupy 109,274 bytes, plus 120 bytes in native bank structures. The resulting pack SHA-256 is `8df1316157fc8ce8dcb87cfd267fea83790702ccd1963ae91c0de60ca255b582`. Native placement rejected both stock images. The aligned space available is 1,184 B on Pro and 1,168 B on Elite. Retaining 1 KiB free requires another 109,234 B on Pro and 109,250 B on Elite. The tested offline export is 110,844 bytes and uses three zones in slot 1 and two in slot 2. The live exporter produced SHA-256 `3a7f19858d23309dac7375b88d0537cae7854ad65680edd1f95d4dbc0beed4c6`. User-selected restore is available. A default delivery needs either additional measured space or a separately approved slot-initialization scheme which preserves existing user recordings.

### Installer receipts and Local gates

The install-report issue form records the build ID, model, firmware SHA-256, port name, startup result, restart result, browser and notes. Each startup and restart field accepts success, failure or an untested result. The dated Rule 2/C4 note distinguishes the owner’s reported disablement of `gates.yml` from the state returned by the public GitHub API. Repository-wide settings were inaccessible to the public reader.

The [receipt form and installer reader](https://github.com/gobbajogger/DnBOX-nightly/pull/170#issuecomment-7058014195) agreed on both released packages in 90 manual report fixtures and eight generated forms. Mismatched models, package checksums and cached port versions produced zero successful installs across 270 controls. The form is staged on the lane branches.

## Earlier evidence

Move: M0 (clean start and green gate)   PR: #955   Head: this commit
LEFT items PASS: 0/2; failing: L15 L16
Gate at head: M0 start checks pending; corrected limits self-match and size-loop false failures; full green gate remains follow-up
ETA merge: start files after M0 checks; green gates 2026-11-17 22:00 VLAT

# Lane: integ   (copy to docs/lanes/integ.md; update in every push)
Owner agent: <name>   Branch: lane/integ-<topic>   Draft PR: #<n>   Head: <sha8>   Updated: <YYYY-MM-DD HH:MM VLAT>

## Checklist (from CHECKLISTS.md)
| ID | Check | Status (PASS/FAIL) | Evidence (SHA + log path or PR comment link) |
|---|---|---|---|
| C1 | Local gate at head | FAIL | |
| <S1…> | | FAIL | |

## Now / next
- Now: <item, riskiest first>
- Next: <item>

## Blockers (each with its fix and the route being tried; never "waiting")
- <blocker> → <fix> (since HH:MM)

## Top 3 risks for the next hour
1.
2.
3.

## Deletes (files this lane removes on purpose, with reason; the gate's no-loss step reads this)
- <path> — <reason>

## Conflicts noted (RULES 23)
- <conflict> → reading used: <…>

## Owner-directed start-only split
This PR now contains only the supplied prep manifest, source ledger and removal list. All unrelated product/test/environment changes are removed from its effective diff by a normal successor commit, never by force push. Complete broad work remains recoverable at backup/integ-before-start-split-20261116 (246ea3718c6c72ba80bbd1aafcd5356f871c78c8).

Latest owner direction permits these two start-only merges after meaningful touched-file Local checks; this is a specific sequencing exception, not a changed RULES.md v2 text or a claim that the full before/after application gate and viewport review passed. Independent QA records exact scoped results and known defects on the PR. All lane finish lines remain open.

The gate scripts are exact supplied ZIP bytes, not later corrected versions. Known follow-up defects include firmware fail-fast/budget attribution and gate environment/prerequisite handling; the touched-file gate must not execute unsafe browser routes or treat the original scripts' possible false PASS as qualification. Full Local remains unqualified.

Removal-list exceptions retained: AGENTS.md, cloud-handoff/pipeline.json, docs/NEW.md. AGENTS.md is replaced, not deleted; other retained paths are existing test dependencies.

## Exact removals
Each path below is an obsolete brief/rule/handoff explicitly listed in the supplied remove manifest; the original bytes remain in the inherited baseline and backup history.
- `.agents/skills/x0x37-check` — supplied removal list; historical instructions, retained in Git history.
- `.agents/skills/x0x37-device-bug` — supplied removal list; historical instructions, retained in Git history.
- `.agents/skills/x0x37-docs` — supplied removal list; historical instructions, retained in Git history.
- `.agents/skills/x0x37-install` — supplied removal list; historical instructions, retained in Git history.
- `.agents/skills/x0x37-integrate` — supplied removal list; historical instructions, retained in Git history.
- `.agents/skills/x0x37-owner-decisions` — supplied removal list; historical instructions, retained in Git history.
- `.agents/skills/x0x37-shared-work` — supplied removal list; historical instructions, retained in Git history.
- `.agents/skills/x0x37-task` — supplied removal list; historical instructions, retained in Git history.
- `.agents/skills/x0x37-workflow` — supplied removal list; historical instructions, retained in Git history.
- `.agents/skills/x0x37-writing/SKILL.md` — supplied removal list; historical instructions, retained in Git history.
- `.agents/skills/x0x37-writing/references/strict.md` — supplied removal list; historical instructions, retained in Git history.
- `.agents/skills/x0x37-writing/references/terms.md` — supplied removal list; historical instructions, retained in Git history.
- `.cursor/environment.json` — supplied removal list; historical instructions, retained in Git history.
- `.cursor/skills/loopyx0x37-classic-build/SKILL.md` — supplied removal list; historical instructions, retained in Git history.
- `.cursor/skills/loopyx0x37-classic-build/references/builds.md` — supplied removal list; historical instructions, retained in Git history.
- `.cursor/skills/loopyx0x37-classic-build/references/cursor-runtime.md` — supplied removal list; historical instructions, retained in Git history.
- `.cursor/skills/loopyx0x37-classic-build/references/handoff.md` — supplied removal list; historical instructions, retained in Git history.
- `.cursor/skills/loopyx0x37-classic-build/references/provenance.md` — supplied removal list; historical instructions, retained in Git history.
- `.cursor/skills/loopyx0x37-classic-build/references/stages.md` — supplied removal list; historical instructions, retained in Git history.
- `STATUS.md` — supplied removal list; historical instructions, retained in Git history.
- `audit/HANDOFF.txt` — supplied removal list; historical instructions, retained in Git history.
- `audit/artifact-comparison.json` — supplied removal list; historical instructions, retained in Git history.
- `audit/current-device-diag.txt` — supplied removal list; historical instructions, retained in Git history.
- `audit/differential-results.json` — supplied removal list; historical instructions, retained in Git history.
- `audit/smc_differential.py` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/START-HERE.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-complete/HANDOFF.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-complete/built-packages.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-complete/elite-build.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-complete/input-and-artifact-identity.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-complete/pro-build.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/CONTINUE.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/ble-midi-receive-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/dev-validation.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/docs-check.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/focused.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/gate-configs-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/gate-configs-repaired.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/gate-reason-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/input-identity.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/install-gate-guard-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/installer-ctl-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/installer-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/installer-slots-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/overflow-baseline.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/overflow-fixed.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/regressions.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/smc-reachability-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/smk-update-contract-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/toolchain-redirect.headers` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/toolchain-retry.error` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/toolchain-retry.headers` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/toolchain.headers` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/transfer-integrity.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage2-continuation/validation-inputs-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/HANDOFF.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/SUCCESSOR.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/app/elite-build.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/app/isolation-fixture-probe.patch` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/app/isolation-probe.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/app/packages.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/app/pro-build.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/app/runner.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/app/smk-clip-isolation.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/docs-check.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/evidence-sha256.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/input-and-artifact-identity.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/REPORT.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/ble-central-diag-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/ble-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/ble-midi-receive-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/central-baseline.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/central-guard-probe.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/clip-baseline.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/clip-extractor-probe.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/clip-extractor-probe.py` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/diag-size-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/diagnostic-fixture-repairs.patch` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/focused.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/inlog-tool-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/receive-strict.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/smc-buttons-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/smc-clip-visibility-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/smc-fader-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/smc-filter-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/smc-mapping-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/smc-part-select-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/smc-protocol-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/smc-reachability-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/smk-apc40-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/smk-apc40-map-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/smk-apc40-violet-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/smk-inlog-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/receive/ubsan-strict.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/dev-validate.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/gate-configs-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/gate-reason-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/install-gate-guard-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/install-guards-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/install-head-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/installer-ctl-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/installer-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/installer-slots-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/installer-targets-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/report.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/runner.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/smk-update-contract-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/safety/validation-inputs-detail.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud-stage3-investigation/stage2-repair-prompt.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud_setup_20261020/HANDOFF.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud_setup_20261020/firmware-setup-check.sh` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud_setup_20261020/prior-draft.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/cloud_setup_20261020/validation.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/delivery_convergence_20261020/HANDOFF.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/delivery_convergence_20261020/checkpoint-manifest.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/delivery_convergence_20261020/fm1-stage2-checkpoint.patch` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/delivery_convergence_20261020/initial-inventory.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/delivery_convergence_20261020/merge-resolutions.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/delivery_convergence_20261020/smc-stage2-checkpoint.patch` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/fm1_reference_stage2_20261020/build/fm1-reference-stage2/checkpoint.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/fm1_reference_stage2_20261020/build/fm1-reference-stage2/gate-reason.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/fm1_reference_stage2_20261020/build/fm1-reference-stage2/install-slots.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/fm1_reference_stage2_20261020/build/fm1-reference-stage2/update-contract.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/fm1_reference_stage2_20261020/build/fm1-reference-stage2/validation-inputs.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/smc_receive_repair_20261020/baseline-overflow.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/smc_receive_repair_20261020/cloud-transition.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/smc_receive_repair_20261020/inlog-tool.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/smc_receive_repair_20261020/overflow_probe.c` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/smc_receive_repair_20261020/reachability-linux.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/smc_receive_repair_20261020/reachability.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/smc_receive_repair_20261020/receive-sanitizers.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-20/transfer-sha256.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/HANDOFF.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/baseline-isolation.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/baseline-receive-strict.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/baseline-visibility.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/ble-midi-receive.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/cc-strict` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/docs-check.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/evidence-sha256.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/failfast-original-fixture.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/focused.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/input-and-checks.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/isolation-strict.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/smc-clip-visibility.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/smk-clip-isolation.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/stage3-prompt.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/strict-results.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/thread-capability.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage2-fixture-repair/visibility-strict.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/HANDOFF.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/evidence-sha256.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/extraction/REVIEW.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/extraction/cc-strict` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/extraction/isolation-strict.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/extraction/normal.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/extraction/results.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/extraction/smc-clip-visibility.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/extraction/smk-clip-isolation.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/extraction/visibility-strict.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/receive/ble-midi-receive.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/receive/failfast-original-fixture.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/receive/focused.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/receive/results.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/receive/review.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/root/docs-check.log` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/root/reuse-and-integrity.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/root/thread-capability.json` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/2026-10-21/cloud-stage3-fixture-retest/stage4-prompt.txt` — supplied removal list; historical instructions, retained in Git history.
- `cloud-handoff/firmware-release-labels/NATIVE_BUILD_INPUT.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/QUICKCHECK-SCOPE.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/fm1-reference-handoff.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/installer-ctl-selection-pending-20261024-r18.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/ble-two-peer-central-port.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/build-25-m1-port-20261106/README.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/build-25-m1-port-20261106/independent-qa.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/build-25-m1-port-20261106/independent-qa.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/build-25-m1-port-20261106/validation.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/build25-usb-recut/qa-closure-20261106/README.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/build25-usb-recut/qa-closure-20261106/closure.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/build25-usb-recut/qa-closure-20261106/old-fixture-mutant.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/build25-usb-recut/qa-closure-20261106/optimization-equivalence-original.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/build25-usb-recut/qa-closure-20261106/reproduce_usb_timeout.py` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/build25-usb-recut/qa-closure-20261106/usb-v2-after-mutant.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/build25-usb-recut/qa-closure-20261106/usb-v2-baseline.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/build25-usb-recut/qa-closure-20261106/v2-fixture-mutant.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/chicago-opt.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/engines-ui-dev-plan.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/engines-ui.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/evidence/pr691-goldens-f9ffd12e-20261030.txt` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/miami-one-build-rebase.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/miami-one-build-table-fastpath.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/miami-opt2.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/miami-pr691-cloud-validation-20261030.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/miami-pr691-golden-input-check-20261030.txt` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/midi-queue-order-original.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/missing-features-plan.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/one-build-host-validation.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/one-build-optimization-recut/README.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/one-build-optimization-recut/host-suite.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/one-build-optimization-recut/inputs.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/one-build-optimization-recut/native-metrics.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/one-build-optimization-recut/post-review-test-repairs.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/one-build-optimization-recut/production-inputs.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/one-build-optimization-recut/recut-manifest.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/one-build-optimization-recut/review.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/one-build.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/orchestrator.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/pr705-cloud-qualification-20261030.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/pr705-cloud-test-output-20261030.txt` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/pr705-historical-baseline-host-results-20261030.txt` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/pr705-source-recovery-evidence-20261030.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/pr705-v121-native-retest-20261030.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/lanes/q3-editor-slots-evidence.txt` — supplied removal list; historical instructions, retained in Git history.
- `docs/live-a.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/mbootloader-usrtrim-install-plan.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/one-build-integration-plan.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/remaining-feature-host-plan.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/remaining-features-20261030.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/rules/firmware-nightly-promotion.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/rules/local-proof.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/rules/workflow-history.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/rules/workflow.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bf011-break-q16-native-recheck-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bf013-full-stereo-native-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bf014-panel-calibration.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bf015-tb3po-git-handoff-v1.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bf015-tb3po-mutation.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bf016-bass303-fenv-idempotence.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bf017-motion-release-pending.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf018-motion-clear-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf020-bd-pan-motion-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf021-project-object-collision-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf022-legacy-master-highpass-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf023-motion-pattern-pending-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf024-motion-switch-order-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf025-clip-key-note-ownership-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf025-clip-strum-ownership-r3-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf025-runner-exec-r2-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf026-strum-emitted-pitch-ownership-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf027-motion-idle-transport-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf028-human-reset-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf029-motion-queued-transport-20261025-r1.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf030-motion-external-clock-wait-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf031-tb3po-logical-bars-20261025-r1.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf032-tb3po-accent-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-bf033-random-tie-preview-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-pr139-r3-orphan-tie-native-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-pr140-r2-random-temporal-swing-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-pr140-r3-random-clock-source-handoff-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/bugfinder-pr159-r2-bass-sink-test-compat-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/classic-qa-pr103-r15-wire-fault-matrix-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/classic-qa-pr118-smc-host-overflow-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/clock_handover_interaction.c` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/clock_handover_interaction.sh` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/combined-seq-synth-reliability-20261025.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/controller-padding-first-nightly-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-backup-atomic-20261023-patched.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-backup-atomic-20261023.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-backup-atomic-20261023.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-backup-darwin-dirfd-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-backup-directory-durability-20261024-r1.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-backup-linkless-20261024-pass.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-backup-linkless-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-backup-linkless-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-backup-snapshot-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-backup-snapshot-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-ctl-readback-framing-20261024-r1.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-install-backup-noclobber-20261023-patched.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-install-backup-noclobber-20261023.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-install-backup-noclobber-20261023.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-ota-finish-ack-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-ota-finish-ack-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-ota-loader-resume-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-pr110-controller-address-echo-7bit-20261024-r2.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-pr76-per-slot-restore-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-pr96-cli-mode-20261024-pass.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-pr96-cli-mode-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-pr96-cli-mode-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-restore-max-matching-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-restore-max-matching-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-restore-status-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-slot-echo-7bit-20261024-r1.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-slot-timeout-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-slot-timeout-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-slot-write-ack-integrity-20261024-r1.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-smp-info-invariants-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-smp-info-invariants-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-smp-info-preflight-20261023-patched.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-smp-info-preflight-20261023.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-smp-info-preflight-20261023.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-stack-replay-20261024-pr91-darwin.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-stack-replay-20261024-pr91.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-stack-replay-20261024-pr96-darwin.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/data-safety-stack-replay-20261024-pr96.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/README.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/aggregate-normal.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/baseline-project-runtime-sanitized.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/baseline-regression.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/candidate-regression.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/candidate-sanitized.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/handoff.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/independent-review-receipt.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/independent-review.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/independent-runs.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/producer-dispatch-green.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/producer-dispatch-red.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/producer-dispatch-sanitized.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/producer-focused-suites-darwin.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/producer-focused-suites.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/project-protocol-sanitized.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/editor-ota-dispatch-race-20261025/project-runtime-sanitized.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/first-nightly-fulltree-recovery-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/first-nightly-fulltree-recovery-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-ctl-selection-pending-20261024-r18.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-nightly-controller-binding-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-nightly-download-guard-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-nightly-model-prewrite-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-nightly-ota-loader-model-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-nightly-postreboot-model-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-nightly-product-identity-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-nightly-publisher-release-origin-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-nightly-revocation-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-pr102-r20-size-binding-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-pr102-r21-unreviewed-stock-links-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-pr102-r22-postwrite-sample-result-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-pr102-r23-bounded-stream-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-pr102-r24-nostream-deny-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-pr102-r25-approved-package-byte-budget-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-pr102-r26-stale-selection-20261025.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-pr102-r27-local-file-size-20261025.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/installer-verified-blob-download-20261024-r11.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/integration-303-archive75-ctl49-smc79-critical-path-20261024-r7.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/integration-303-data-safety-backup-snapshot-20261024-r5.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/integration-303-data-safety-loader-resume-20261024-r3.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/integration-303-data-safety-replay-20261024-r2.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/integration-303-data-safety-restore-ota-finish-20261024-r4.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/integration-303-pr107-dirsync-critical-path-20261024-r8.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/integration-303-small-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/integration-303-small-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/integration-303-smc50-slot76-critical-path-20261024-r6.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/integration-303-smc79-63-73-critical-path-20261024-r9.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/integration-smc40-smc43-datasafety108-first-candidate-20261024-r10.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/integration-smc58-solo-overflow-first-candidate-20261024-r12.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-break-reverse-checkpoints-20261023.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-ima-decode-benchmark-20261023.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-adversarial-full-renderer-20261024-r6.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-aligned-live-r24-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-callback-tail-20261025-r10.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-checkpoint-aligned-20261025-r23.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-chunk-partition-20261025-r18.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-decoder-work-budget-20261025-r11.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-direct-nightly-compare-20261025-r19.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-dual-voice-20261025-r22.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-fast-reverse-advance-20261025-r13.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-fractional-tempo-20261025-r14.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-live-set-stress-20261025-r15.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-low-rate-long-loops-20261025-r21.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-musical-tempo-work-20261025-r12.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-rch-buffer-host-tradeoff-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-real-renderer-latency-r25-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-renderer-native-regression-20261024-r5.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/performance-pr93-seeded-live-property-20261025-r17.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/private-combined-candidate-20261025/README.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/private-combined-candidate-20261025/build-receipt.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/private-combined-candidate-20261025/package-review.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/private-combined-candidate-20261025/source-review.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/project-persistence-reconciliation-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/qa-r22-host-harness-recovery-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/qa-r22-host-harness-recovery-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/safe-merge-pr103-pr110-7bit-r15.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/safe-merge-pr103-r15-cli-mode-r16.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/safe-merge-pr103-r17-smp-info-fixture-source-handoff.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/safe-merge-pr103-slot109-20261024-r11.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/safe-merge-pr103-smc58-early-snapshot-r14.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-auto-idle-wrap-20261025-r33.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-accepted-play-20261025-r39.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-accepted-stop-20261025-r38.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-crosssource-pending-20261025-r37.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-epoch-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-handover-20261024-native.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-handover-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-handover-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-handover-interaction-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-handover-threeway-20261024-r2.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-internal-transition-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-local-stop-20261025-r40.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-owner-boundary-20261025-r30.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-owner-linked-native-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-source-tag-test.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-source-tag.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-source-tag.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-clock-uptime-rollover-20261025-r32.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-engine-event-partition-20261024-r26.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-external-dropout-resume-20261025-r28.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-first-f8-coalesced-20261025-r36.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-first-f8-reconnect-20261025-r35.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-fixture-abi-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-internal-idle-mode-transition-20261025-r34.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-link-disconnect-handoff-20261025-r31.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-local-echo-external-tick-20261024-r23.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-long-notes-20261025.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-midi-lifetime-stress-20261025-r27.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-mixed-origin-f8-grid-20261024-r25.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-mixed-origin-f8-partition-20261024-r24.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-pingpong-length-resize-native.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-pingpong-length-resize.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-pingpong-length-resize.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-pingpong-linked-firstnightly-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-pr90-firstnightly-linked-mid13-20261024-r2.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-rec-55-65-native-linked-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-stop-restart-burst-20261025-r29.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-coalesce-20261023-after.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-coalesce-20261023-before.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-coalesce-20261023-suite.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-coalesce-20261023.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-coalesce-20261023.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-epoch-output-linked-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-mixed-origin-output-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-noteoff-linked-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-origin-20261023-before.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-origin-20261023-clang-san.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-origin-20261023-gcc-ubsan.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-origin-20261023-gcc.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-origin-20261023.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/seq-transport-origin-20261023.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-action-feedback-20261025/README.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-action-feedback-20261025/caption-dispatch.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-action-feedback-20261025/independent-review.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-action-feedback-20261025/knob-framebuffer.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-action-feedback-20261025/lock-tests.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-action-feedback-20261025/native-size.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-action-feedback-20261025/negative-tests.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-action-feedback-20261025/ui-tests.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-action-feedback-20261025/validation.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-acl-start-flags-20261023-native.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-acl-start-flags-20261023.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-acl-start-flags-20261023.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-acl-zero-credit-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-acl-zero-credit-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-att-mailbox-retry-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-att-mailbox-retry-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-disconnect-failure-recovery-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-disconnect-failure-recovery-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-hci-completed-packets-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-hci-completed-packets-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-hci-mailbox-backpressure-20261023.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-link-epoch-reset-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-link-epoch-reset-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-stack-reconciled-20261024.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-stack-reconciled-20261024.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-sysex-length-preflight-20261023.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-ble-sysex-length-preflight-20261023.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/README.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/artifact-review.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/baseline-repro.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/elite-build.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/elite-package-receipt.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/evidence-manifest.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/final-affected-tests.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/frame-manifest.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/frames/00-prior-pickup.png` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/frames/01-main-pickup.png` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/frames/03-picked-fader.png` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/frames/04-delay-knob.png` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/frames/07-clip-field.png` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/frames/08-mute-action.png` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/frames/09-part-pan.png` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/frames/10-808-seq-bank2-pan.png` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/frames/11-list-on-fader.png` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/frames/12-parked-knob.png` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/input-reuse.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/late-base-observation.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/pro-build.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/pro-package-receipt.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/repro.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/routing-map-comparison.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/runtime.txt` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/source-review.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-distinct-ui-20261025/source-review.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-fader-pickup-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-fader-pickup-20261025.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-full-control-ui-20261025.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/build-receipt.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/compiled-inputs.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/elite-native.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/handoff.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/independent-review.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/object-symbol-sizes.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/pro-native.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/qa/consolidated-frame-equivalence.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/qa/consolidated-late-ack-negative.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/qa/consolidated-lock.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/qa/consolidated-pause-loss.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/qa/consolidated-solo-overflow.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/qa/consolidated-ui.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/qa/delivery-frame-equivalence.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pause-lock-sound-names-20261025/size-specialist/size-native.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-pr58-solo-early-snapshot-20261024-r1.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-selected-lane-20261025/README.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-selected-lane-20261025/independent-review.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-selected-lane-20261025/lock-tests.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-selected-lane-20261025/native-size.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-selected-lane-20261025/ui-negative.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-selected-lane-20261025/ui-tests.log` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-selected-lane-20261025/validation.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-spatial-lock-select-20261025/README.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-spatial-lock-select-20261025/independent-review.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/scheduler-handoffs/smc-spatial-lock-select-20261025/validation.json` — supplied removal list; historical instructions, retained in Git history.
- `docs/smc-mapping-handoff-2026-10-20.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/smc-reachability-handoff-2026-10-20.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/song-actions-handoff-2026-10-20.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/ui-theme-proposal-20261030.md` — supplied removal list; historical instructions, retained in Git history.
- `docs/ui-theme-proposal-20261030.svg` — supplied removal list; historical instructions, retained in Git history.
- `local-handoff/2026-10-21/stage2-fixture-repair/HANDOFF.md` — supplied removal list; historical instructions, retained in Git history.
- `local-handoff/2026-10-21/stage3-fixture-retest/HANDOFF.md` — supplied removal list; historical instructions, retained in Git history.
- `local-handoff/2026-10-21/stage4-dev-checked/HANDOFF.md` — supplied removal list; historical instructions, retained in Git history.
- `local-handoff/2026-10-21/stage4-validation/HANDOFF.md` — supplied removal list; historical instructions, retained in Git history.
- `local-handoff/2026-10-21/stage5-complete-set/HANDOFF.md` — supplied removal list; historical instructions, retained in Git history.
- `local-handoff/2026-10-21/stage6-dev-installed/HANDOFF.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-check/SKILL.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-check/references/build-environment.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-check/references/commits.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-check/references/validation.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-device-bug/SKILL.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-docs/SKILL.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-docs/references/checks.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-install/SKILL.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-install/references/artifacts.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-install/references/evidence.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-integrate/SKILL.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-integrate/references/release-gate.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-owner-decisions/SKILL.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-shared-work/SKILL.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-task/SKILL.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-task/references/ownership.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-task/references/routing.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-workflow/SKILL.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-workflow/references/handoff.md` — supplied removal list; historical instructions, retained in Git history.
- `skills/x0x37-workflow/references/stages.md` — supplied removal list; historical instructions, retained in Git history.

M0 limits correction: exclude the gate scanner itself from its PSRAM-setting diff and explicitly succeed for small tracked blobs; object-read failures still fail closed. Original supplied limits falsely rejected its own regex and the final small file. Environment and full-gate repairs remain separate.

## M5 roadmap source candidate (2026-11-17)

Owner: integ. Target: integration/dnbox-consolidated. Exact base: 7b17a64e11731de50f4fe46ef7ebe3948a730a10.
Roadmap generation and regression tests implemented; positive local ancestry required for every Complete entry against explicit DNBOX_GATE_BASE_SHA. No move declared Complete. Preserved historical ROADMAP content.
Local checks: generator tests 7 PASS; current views PASS; existing roadmap-evidence self-test 42 PASS. These checks do not qualify the complete M5 composition. Actions not used.
M5 remains WORKING: compact-program LSan outside tracer, deployed five BREAK previews/selection, and genuine feature-list item120 voting runtime acceptance remain open. No device operation or publication claimed.


### Hygiene review corrections after #957/#958

Owner: integ hygiene-review worker. Consolidated input: `ec4f4e56abc37d677475244d73588fc42effa415`. Correct docs PR target, required gate-log exception, workflow prohibition, missing documentation links and rename wording. Reuses `lane/integ-hygiene-review-fixes`; no firmware or workflow changes. Documentation assertions PASS; full gate and independent final-head review NOT_RUN. Next: independent review and applicable integration gates; no delivery claim.
- tests/host/roadmap_evidence_check.sh: evidence-only check, stale per DOT-BRIEF.
- tests/host/roadmap_evidence_check_test.sh: self-test of the removed evidence check.
- tests/host/run_host_with_evidence.py: evidence-only host-run wrapper, stale per DOT-BRIEF.
- tests/host/mboot_nightly_contract/check_source_closure.py: source-closure receipt check, stale per DOT-BRIEF.
- tests/host/mboot_readonly_contract/check_source_closure.py: source-closure receipt check, stale per DOT-BRIEF.
- tests/ui_defects/first_slice/evaluate.py: judged saved render receipts; replaced by judge.py, which renders and judges the checkout itself.
- firmware/src/dsp/drone_string.h: Drone engine removed (sound item 2); slot 2 is Synth One and old Drone projects load as Synth One init.
- tests/host/drone_slot_test.c: stale Drone test; replaced by tests/host/synthone_slot_test.c.
- tests/host/drone_string_test.c: stale Drone test; replaced by tests/host/legacy_slot2_map_test.c (old Drone projects load as Synth One init).
### BREAK and compact-program checks, 2026-11-18

- On the public [Starting breaks page](https://gobbajogger.github.io/DnBOX-nightly/starting-breaks/), Chromium played each of the five previews after choosing None. All five responses returned HTTP 200 and matched the corresponding repository WAV bytes. Playback kept all five boxes clear. The page made zero requests to the private audio route.
- Choosing All and downloading produced `DnBOX-starting-breaks.x0xslots`, 110844 bytes, SHA256 `2d5dca29b235104ffbf1efe372ba14c35f3d50c92c4630c0d3f9c1d4cfca2117`. The checks exercised the public browser and download paths. Device restoration and audible hardware playback remain unverified.
- At firmware commit `aa5b0691b686c8f543b90b94445975f9aa267aca`, `python3 tests/program_values_compact_test.py` passed both sets of 796 exact metadata checks. LeakSanitizer rejected the genuine 123-byte leak and the test rejected the corrupt metadata control. All ten active original synth inputs matched the pinned manifest.
- The combined firmware gate, stock-device fit, and independent review remain open. The public BREAK result applies to the site observed on 2026-11-18. GitHub reported deployed Pages build `241196130538b5e6da7738f0c2ebbbdc57fce02e` and branch tip `2de2355715f979ca1da7aeca1632c4bfe254ad24`.

### Prepared BREAK archive restore

The public five-sample archive at SHA256 `2d5dca29b235104ffbf1efe372ba14c35f3d50c92c4630c0d3f9c1d4cfca2117` passed parsing and preview but failed verified restore on the first slot. It carries 32 zero bytes between its 480-byte header and the sample payload at offset 512. Sending only the short header left those bytes erased. The CLI and browser installer's restore paths now use the full header area when it contains programmed bytes. They keep the short header for archives with erased padding.

On the SMK host fixture for `523d486e`, the firmware's editor and slot scanner read both original sample images back exactly with a full header (60,237 and 50,061 bytes). With short headers, both images differed in 32 bytes. The CLI and browser installer regression suites pass with nonuniform padding. All 31 possible nonempty selections from the corrected website exporter passed CLI restore and backup against the simulated SMK flash layout.

In the firmware host UI, the full archive exposed three BR1 zones and two BR2 zones. Browsing each played a distinct preview while the pattern selection stayed at zero until SEL. The five measured 350 ms peaks were −9.5, −10.8, −15.7, −12.7 and −13.3 dBFS. The idle control measured −180 dBFS. The measurements cover user-slot imports. The default firmware package and physical keyboard were outside this check.

The full gate at `523d486e` passed the firmware builds, flash map, budget and UI checks. Its sole failed host test was `ext-midi-rec-smk`: a concurrent missing-header probe removed the shared drum tables during compilation. The negative probe now runs the real test runner in a private build directory, and the external MIDI test uses the generated-header lock. Both cases pass together with eight runner jobs. Re-running the negative probe preserved the hashes of all three parent drum and BREAK headers.
