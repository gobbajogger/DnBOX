# Lane: fwui   (copy to docs/lanes/fwui.md; update in every push)
Owner: Classic FWUI successor (Dot FWUI relinquished writing). Branch: `lane/fwui`. Draft PR: #975. Published head: `90fa6d5f0cefa9913e76a6acc8a4ba9cb57c8613`. Updated: 2026-11-18 10:03 EST.

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

### 2026-11-18 original mock recovery and renderer repair

Recovered all 19 omitted PR703 glyph and settings PNGs at their original paths. Every byte count and SHA256 matches the manifest at 64575f86375e7daa9a939de25e6eb2f5047e0665. These are historical mocks. Production comparison uses separately rendered frames.

At 10aaceca the geometry probe stopped before rendering because it still targeted the old relief caption. Updated the probe to instrument the current flat caption. The full geometry subtest then rendered 1,525 states, audited 38,731 texts with zero findings, checked 17,591 content assertions and 249 drum glyph pixel assertions, and passed its negative controls. The 94197fc8 baseline also rendered through the current walker (1,524 states). Independent visual review and the complete lane gate remain open.

- 2026-11-18: Merged #970 (6fa8bb32 → 76040daa); exact fwui gate PASS: 501 host tests, 1,530 geometry states and 46 first-slice screens; broader UI and engine work remains open.

### 2026-11-18 FREE PLAY negative compatibility repair

Integ alone retains the existing exact-90fa gate through terminal. Its [14:50 UTC observation](https://github.com/gobbajogger/DnBOX/pull/975#issuecomment-6982060401) reports the positive FREE PLAY fixture passed, but the historical negative ui.c lacked `physical_stage_label` now required by the drum include. That compiler failure is not an accepted negative. The historical source hash was also unavailable in the successor checkout and GitHub contents API.

Replace only that historical input with a current-source missing-cue mutant. Retain the independent rendered-pixel oracle and wrong-state mutant, including required `FREE_PLAY_MODE_CUE_MISSING` / `FREE_PLAY_MODE_CUE_UNEXPECTED` markers and runtime `FAIL` assertion. No firmware source, control, threshold, image fixture or C oracle changes belong to this repair. The source no longer requires an unreachable historical Git object merely to run the current test.

The separate SOUND selector correction remains an unpublished implementation unit; its candidate resource failures are not accepted or part of this gate repair. The original Integ gate is neither duplicated nor interrupted. Exact repaired-head full gate and independent integration qualification remain required before delivery. No lane completion, native resource change, merge or main write is claimed.
