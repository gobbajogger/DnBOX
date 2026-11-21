# SuperJ6 layer 1 attack preset field

`sj6_apply_preset_fields` applies the published `neonattack` field (app ID 36, main application position 34) to layer 1 of the independently written DnBOX candidate. The function accepts an ordered array of already decoded app IDs and finite binary32 values. The caller supplies an independently configured envelope owner and serializes access. A successful attack batch returns `DNBX_OK`. A batch containing only other fields retains them using the existing raw-storage contract and returns `SJ6_FIELD_STORED`. IDs 212 through 214 remain raw metadata owned externally.

The adapter uses the existing normalized attack conversion and `dnbx_set_layer` on layer 1. It changes no per-part or per-voice layout and adds no per-sample dispatch. The setter refreshes layer-1 derived caches and keeps a running timed stage's cached rate. The verification covers application before note-on. Live preset callback behavior and original-app audio remain unqualified.

## Independent layer admission

Only explicit `SJ6_INDEPENDENT_ENVELOPES` from the caller admits a physical attack write. Exact zero in link fields never establishes independent ownership. Preflight covers the whole batch, including link IDs 115 and 116, both of which appear after ID 36 in the published main order. Any nonzero explicit link value rejects an attack batch, even when a later duplicate is zero. An omitted link whose retained value is nonzero also rejects. If an explicit zero overrides an older retained nonzero, the caller must still independently establish that the envelopes are unlinked. Invalid values and unsupported attack batches leave the complete part unchanged. A repeated valid attack field uses the final value. Other IDs, including 119, retain their existing raw values and presence semantics. The adapter does not implement an original preset decoder, linked-envelope mode, or global preset lifecycle.

## Scoped verification

The first red check compiled the untouched candidate's raw-store path and failed on the physical setting after ID 36 = 0. The part retained the raw zero while attack stayed at 0.00800000038 seconds instead of 0.00200000009. The terminal's original streams are in `evidence/red.stderr` and `evidence/red.stdout`.

The final focused run compared each published knob value (0, 0.25, 0.5, 0.75, 1) against both the published seconds target and the direct physical candidate setup. It rendered 2,048 stereo frames for each setting at 48 kHz, with note 60 at velocity 100. Both channels and the complete candidate state matched the direct path exactly at all five settings. The audible layer-2 channel matched its unchanged control. The new field changed 2,047 layer-1 samples in every case. The known scalar tolerance is the existing candidate's 0.00001 seconds. Audio and state comparisons use byte equality. The checks also exercised raw-only handling, unqualified links, malformed batches, retained-value conflicts and duplicate attack fields. The raw-only, wrong-layer and linear-attack variants each failed at the expected assertion. All compiler and test exits are recorded in `evidence/focused-01/RESULT.json`.

Each case compares against the independently written physical core. The separate recorded SuperJ6 amplitude fixture already contains 5,120 qualified endpoints. That original component fixture was pinned but was not rerendered as part of this field-binding test. Full 640-preset behavior, target footprint, timing, both stock models and production activation remain open.

From the DnBOX repository root after installing this directory as `experiments/sound-hardware-v1/sj6-id36-field-adapter`, run:

```sh
python3 experiments/sound-hardware-v1/sj6-id36-field-adapter/verify.py --repo . --out /tmp/dnbox-sj6-id36-fresh
```

Use a fresh result directory. `INPUTS.json` pins the three independently written DnBOX inputs and four published observation files at `358085d49a6b575dd6bfa79dd6c01c688741ea50`. The compiler only processes the new adapter/test and the existing independent `hardware_candidate/src/dnbx_core.c` and its two headers. There is no original engine source, header or object in that build. The published values are independently anchored in `mapping/results/implementer_brief/numeric_behavior_targets.json` and `public_control_catalog.json`.
