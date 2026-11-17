# Consolidated source ledger

Inherited consolidated baseline: `e742be7bd0d01323c899de6e156f40b85ee451db`.
This tree contains the supplied RULES.md v2 start-file/removal payload for PR #955. The eventual normal merge SHA is recorded by GitHub on that PR; no complete lane, full Local gate, product release or Pages publication is asserted here.

Broad unmerged work is retained at `backup/integ-before-start-split-20261116`, exact tip `246ea3718c6c72ba80bbd1aafcd5356f871c78c8`. Gate/environment corrections move to an independent follow-up; other product work remains in the backup and its original lane branches. No unique history is deleted.

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
