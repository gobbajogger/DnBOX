# Consolidated source ledger

Requested clean-start references (merged start files, not full-gate qualification):
- **START-FW:** `gobbajogger/DnBOX@ea4d1d37edca8dd39469573107a49309d0ea68d7` (firmware PR #955).
- **START-NT:** `gobbajogger/DnBOX-nightly@bc3ed20d69159a01e82e079fa7e1cbbf66cab2ce` (nightly PR #136).

Inherited consolidated baseline: `e742be7bd0d01323c899de6e156f40b85ee451db`.
This tree contains the supplied RULES.md v2 start-file/removal payload for PR #955. The eventual normal merge SHA is recorded by GitHub on that PR; no complete lane, full Local gate, product release or Pages publication is asserted here.

Broad unmerged work is retained at `backup/integ-before-start-split-20261116`, exact tip `246ea3718c6c72ba80bbd1aafcd5356f871c78c8`. Gate/environment corrections move to an independent follow-up; other product work remains in the backup and its original lane branches. No unique history is deleted.

## Firmware UI integration

On 21 November 2026, Integ merged [PR #975](https://github.com/gobbajogger/DnBOX/pull/975) from `lane/fwui` into `integration/dnbox-consolidated`. The merge `3e307cfa455d22dfca5e4ef439080816b4e4f859` contains the tested head `9dd6f2df020340f7a82ab241ef07c6ec16195ebc` and the prior consolidated tip `1c82772bac9a33f030bf954ec6151987aef2cbb8`. The 37 focused registrations, both stock builds and the independent 10/10 screen review passed. The firmware lane tracks the combined hold-GLOBE and pad/LED checks. [Merge](https://github.com/gobbajogger/DnBOX/commit/3e307cfa455d22dfca5e4ef439080816b4e4f859) · [FWUI checklist](docs/lanes/fwui.md)

## Machine-readable roadmap state

```json
{
  "items": [
    {
      "id": "M0",
      "title": "Start files and green gates",
      "move": "M0",
      "status": "working",
      "merge_sha": "",
      "owner": "integ",
      "dependencies": [],
      "next_action": "Complete exact-head checks and integrate; no move completion asserted"
    },
    {
      "id": "M1",
      "title": "Sound engines",
      "move": "M1",
      "status": "working",
      "merge_sha": "",
      "owner": "sound",
      "dependencies": [
        "M0"
      ],
      "next_action": "Complete exact-head checks and integrate; no move completion asserted"
    },
    {
      "id": "M2",
      "title": "Firmware UI",
      "move": "M2",
      "status": "working",
      "merge_sha": "",
      "owner": "fwui",
      "dependencies": [
        "M0"
      ],
      "next_action": "Coordinate M1 control lists and shared memory budget; qualify combined engines and UI before completion"
    },
    {
      "id": "M3",
      "title": "Remote",
      "move": "M3",
      "status": "working",
      "merge_sha": "",
      "owner": "remote",
      "dependencies": [
        "M0"
      ],
      "next_action": "Complete exact-head checks and integrate; no move completion asserted"
    },
    {
      "id": "M4",
      "title": "Website",
      "move": "M4",
      "status": "working",
      "merge_sha": "",
      "owner": "web",
      "dependencies": [
        "M0"
      ],
      "next_action": "Use M2 render inputs as they arrive; require final same-SHA render parity before completion"
    },
    {
      "id": "M5",
      "title": "BREAK samples, LSan and roadmap",
      "move": "M5",
      "status": "working",
      "merge_sha": "",
      "owner": "integ",
      "dependencies": [
        "M0"
      ],
      "next_action": "Complete exact-head checks and integrate; no move completion asserted"
    },
    {
      "id": "M6",
      "title": "Combined qualification and delivery",
      "move": "M6",
      "status": "pending",
      "merge_sha": "",
      "owner": "integ",
      "dependencies": [
        "M1",
        "M2",
        "M3",
        "M4",
        "M5"
      ],
      "next_action": "Complete exact-head checks and integrate; no move completion asserted"
    }
  ]
}
```
