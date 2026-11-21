# Consolidated source ledger

Requested clean-start references (merged start files, not full-gate qualification):
- **START-FW:** `gobbajogger/DnBOX@ea4d1d37edca8dd39469573107a49309d0ea68d7` (firmware PR #955).
- **START-NT:** `gobbajogger/DnBOX-nightly@bc3ed20d69159a01e82e079fa7e1cbbf66cab2ce` (nightly PR #136).

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
      "next_action": "Reconcile the 23 host failures recorded at firmware 76040daa and qualify the current consolidated firmware and nightly baselines."
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
      "next_action": "Compact-program LeakSanitizer passed at 76040daa. Resolve stock flash space for five BREAK samples and verify an authenticated feature vote."
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

## Merged pull requests through `1c82772bac9a33f030bf954ec6151987aef2cbb8`

GitHub reports these PRs merged into `integration/dnbox-consolidated`. Each merge commit is reachable from this snapshot. The PR head is included for squash merges and deleted branches. Lane acceptance is tracked in `docs/lanes/integ.md`.

| PR | Source branch | PR head | Merge commit |
|---|---|---|---|
| [#955](https://github.com/gobbajogger/DnBOX/pull/955) | `lane/integ-start-files` | `13b7bbb3fe67b4e9006c550223a6a58a357b4c98` | `ea4d1d37edca8dd39469573107a49309d0ea68d7` |
| [#957](https://github.com/gobbajogger/DnBOX/pull/957) | `lane/integ-repo-hygiene` | `ce3319f8e0967fdc586244f46c68f10b9de92988` | `ec4f4e56abc37d677475244d73588fc42effa415` |
| [#958](https://github.com/gobbajogger/DnBOX/pull/958) | `lane/integ-rename-links` | `f1f1739ee2f6b17a89a6a1614de73409d63ec425` | `2f631c814b239c83dd016fbb123f4837ba93a5d7` |
| [#962](https://github.com/gobbajogger/DnBOX/pull/962) | `lane/integ-gate-fix` | `3e68c3e0a8e3e53de3627ebecf9fcdc517caa587` | `7b17a64e11731de50f4fe46ef7ebe3948a730a10` |
| [#966](https://github.com/gobbajogger/DnBOX/pull/966) | `lane/integ-hygiene-tools` | `6e662d967c2c7db09aeb0a104c159d3808720ba1` | `f8fbfd6b0301e00da852e08142b2ec390d9d2d57` |
| [#968](https://github.com/gobbajogger/DnBOX/pull/968) | `lane/integ-merge-enforcement` | `78daa5d9531b0dfdef37afd84a5b5f8685076fe1` | `3567e86c05240caeac796db605b4583a3338e0d9` |
| [#969](https://github.com/gobbajogger/DnBOX/pull/969) | `lane/integ` | `d01cea4dc21a61b42d6f344e8ab06250365884f1` | `89b40a93de444044ae29421b8f95e85a17081950` |
| [#970](https://github.com/gobbajogger/DnBOX/pull/970) | `lane/fwui` | `6fa8bb32fdff0198957616d239a6c56e9be73f06` | `76040daa5b52d686a8b7b544296ba5d79610a95d` |
| [#972](https://github.com/gobbajogger/DnBOX/pull/972) | `lane/integ` | `06bd40c57cefcdf6c751b318b1cbaea0914fa8bb` | `c54d43b8806173b43c1ca49d93bd790e09833426` |
| [#974](https://github.com/gobbajogger/DnBOX/pull/974) | `lane/integ` | `66acd3f06193d6ee37e5a50eaa45ab71e1cb93da` | `f753443cf31bf912356b27df022f951b8c82fcd8` |
| [#977](https://github.com/gobbajogger/DnBOX/pull/977) | `backup/rules-fix-20261121` | `370d4c5cdc1a3601947beb8f35f0a7b0e72a3843` | `d729626b659080e59584227c03352dab07bb5b53` |
| [#978](https://github.com/gobbajogger/DnBOX/pull/978) | `backup/rules-cleanup-20261121` | `30c834c151193e956ccfd8373dabf3b55052be72` | `8e4c1a10dbc6c84d2c6d8c4bf086d1ef699f3374` |
| [#980](https://github.com/gobbajogger/DnBOX/pull/980) | `backup/rules-binary-20261121` | `4592ac7f9d0c45bef0d7769aeb40bfb1b30180fe` | `19c0de8b78b3116e89677332694836b3315756d1` |
| [#981](https://github.com/gobbajogger/DnBOX/pull/981) | `backup/rules-site-gate-20261121` | `69a650da0e7885bc516293bd573313280152d15c` | `1c82772bac9a33f030bf954ec6151987aef2cbb8` |
