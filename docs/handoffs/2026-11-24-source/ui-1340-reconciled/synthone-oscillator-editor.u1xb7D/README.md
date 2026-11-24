# Compact Synth One oscillator editor

This isolated patch adds nine source-authenticated compact Synth One controls. It reuses the existing SMK sound page, editor queue and per-part ownership. It changes no SuperJ6, LFO-sync, shared-map or ARP scheduler code.

| SMK page | K5 | K6 | K7 | K8 |
| --- | --- | --- | --- | --- |
| OSCILLATOR | OSC BALANCE (existing) | OSC 1 LEVEL (existing) | OSC 1 MORPH (ID 0) | OSC 2 MORPH (ID 1) |
| OSC MIX | OSC 2 LEVEL (6) | SUB LEVEL (7) | SUB OCTAVE (8) | SUB WAVE (9) |
| FM / NOISE | FM LEVEL (10) | FM INDEX (11) | NOISE LEVEL (12) | Unassigned |

Both morph controls are continuous. The glyph samples the compact oscillator's existing interpolation at a fixed phase with note-dependent antialiasing disabled. No waveform ordinal is inferred from factory storage. The sub controls use the compact engine's actual one/two-octave and sine/square meanings. FM INDEX uses its unitless 0–15 range, and NOISE LEVEL uses the original 0–0.25 gain range. See the [authenticated inventory](../synthone-product-control-audit/README.md). Filter type, ADSR pitch tracking, and other independent filter controls remain separate. The existing FILTER row, LINK toggle and both ADSR pages stay in place.

## Persistence

The original 32-byte sound cell and project revision retain their sizes and field offsets. A new Synth One marker uses bit 22 of that cell's existing edit metadata. It owns an 11-byte record in each existing reserved sound row 6/7. Two bytes indicate which controls were authored, and nine bytes hold their positions. Any unmarked existing data in the owned extent causes the new edit to be rejected. A marked bad record is rejected by the project and note-lock validators. Legacy records with no marker are read without applying the new controls.

Edits apply at the audio boundary, under the current owner/preset revision. Project reload, typed history, preset replacement and sound reset carry the side record. A captured note lock carries both the independently authored FILTER ADSR/LINK state and its oscillator record. The compact wrapper stores exact factory targets for the nine controls so lock transitions can restore an unedited target without quantizing it. The existing host backend-union assertion passed with that added state. This is host evidence. Target layout, XIP and timing remain to be measured on the integrated build.

## Evidence

- [Red baseline](red.log) fails at readback of oscillator morph ID 0. [Green](green.log) passes 194 checks. Every control on both parts changed PCM during a held note. The test checks peer cells and side records, rejects a malformed project record, reloads the full sound object bit for bit, validates a saved note lock, preserves independent FILTER ADSR under LINK, and restores exact INIT PCM after preset replacement. PCM equivalence compares captures from freshly initialized DSP.
- [Factory editor](factory-editor.log) passes 28 checks using the real local authenticated pack. Both owners edit across real detents, save/reload preserves the original program numbers 129 and 694, and reselecting each program restores its exact original cell and morph value.
- [Factory non-regression](factory.log) compares untreated INIT and all 694 mapped original programs. The two compact builds emit identical 5,693,440-byte PCM streams (1,024 stereo frames per program/state). Local preset bytes and raw PCM are outside this handoff.
- Existing [compact LINK](link-compact.log) and [dry LINK](link-dry.log) each pass 40 checks. Both parts change PCM when linked and reproduce their prior independent PCM on unlink. [Compact ARP/SEQ](arp-compact.log) passes 126 existing checks.
- Real 240×240 host screens show [OSCILLATOR](panel/compact-oscillator.png), [OSC MIX](panel/compact-osc-mix.png), and [FM / NOISE](panel/compact-fm-noise.png). Edited positions are captured separately under `panel/`. The FM/NOISE bank has three assigned knobs.

## Reproduce

The patch is relative to the joined LINK-present source used for the [exact fit lane Hz host](../../perf-final-proof/exact-fit-joined/HANDOFF.md), formerly `/tmp/dn-fit-hz.e2mFXG`. [Source preimages](source-preimages.json) fingerprint every touched source. The previous source audit's stock commit has no LINK and is not the patch base. The isolated candidate was copied from the LINK-present source without mutating it.

Use a fresh source copy with its verified generated host inputs. Point `proof` to this evidence directory, `root` to the DnBOX workspace, and `out` to a fresh absolute temporary path. The actual factory checks expect the already authorized local pack with SHA-256 `c6c39e8425e51ede8e46b94b7272a7c96e2987894f2e9c0b598d312c7fe04d44`.

```sh
git apply --check "$proof/synthone-compact-editor.patch"
git apply "$proof/synthone-compact-editor.patch"
export X0X_SUPERJ6_FACTORY=1 X0X_S1C=1 PYTHONDONTWRITEBYTECODE=1
export X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'
export SMC_UI_TEST_FILE=tests/host/ui_synthone_oscillator_editor_test.c SMC_UI_OUT="$out"
sh "$root/perf-final-proof/sj6-knob-persistence/host-with-existing-inputs.sh"
S1_EDITOR_TEST_PACK="$root/s1-audit-data/s1.pack" "$out/test"
```

The factory comparison driver accepts the unpatched base, patched source directory and local pack as arguments. It writes both PCM streams inside patched scratch and keeps only the digest/verification log as evidence. Earlier published fit and timing receipts apply to their exact base images. This is a local host-qualified source candidate for integration.
