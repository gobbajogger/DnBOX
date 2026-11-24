# Active DnBOX control-parity build: Init only

Owner scope update: remove **all** Synth One and SuperJ6 factory presets from the development firmware now and finish the two complete engines' controls first. DnBOX previously shipped 694 Synth One presets; the independent 906-row upload is not the shipped set. Neither set is in this profile. Factory-bank compression, import and residency are deferred, not prerequisites for any control edit. Existing private assets and saved-project data are not deleted or migrated; no release or hardware action.

Apply `init-only-control-parity.patch` to the composed f7d17a57 baseline (firmware source manifest `../composed-pat-wire/POST.sha256`), not directly to integration/main. Verify `FINAL-POST.sha256` after applying. In this tree the dual-engine Elite/Pro profile defaults to Init only (compact Synth One with its editor plus typed SuperJ6 with built-in Init). It doesn't compile or read S1PK and doesn't emit type-103 FDATA. The reserved address, protected maps, arp, S1/SJ6 settings, Korg banks and other engines remain intact. Existing old S1/SJ6 saved selectors are admitted and play built-in Init without changing saved project bytes. New requests offer only Init. Explicit environment override 0 is available to reproduce historical banked receipts, not the active control build.

## Measured

- Packaging/native-plan regression (both models): no S1 bank generation, no native S1 pack reader, no FDATA payload and unchanged flash reservation. This is omission, not an erase.
- Actual same-source host project save/reopen from the earlier profile on both part assignments: no writes during reload, exact saved object bytes and both engines render with Init after reload. No preset pack input.
- Physical UI arp/step handling: 126 checks on this composed host profile. Actual SuperJ6 OUTPUT K4 oscillator 2 level: 451 root checks, both parts, independent oscillator/sub and layer isolation, save/reload and a live sequence note-lock. The owner-facing control changes the source oscillator amplitude, not composite layer gain.
- Independent longer SuperJ6 fixture with four marked synthetic project wires proved the *previous* reader refuses loading and saving without writing. Full original AU fidelity, all controls and native timing are not established.
- Native links with the Init-only change before the new oscillator control demand 27,760 B too much on Elite and 27,748 B on Pro (1,668 B saved versus f7d17a57). No package, device or site change. The guarded diagnostic 909 companion linked/booted in both emulators separately, but that is not updater-safe or full heavy scene evidence and not this composed source's release package.

## Reproduce focused host evidence

From the patched source with pinned ordinary generated host asset headers in build/gen:

1. Run `PYTHONDONTWRITEBYTECODE=1 python tests/host/synth_controls_init_only_test.py`.
2. Derive the existing helper: `sed '/^int main(void)$/,$d' tests/host/ui_synthone_filter_mode_test.c > tests/host/synth_init_test_helpers.h`.
3. Use `sh tests/host/sj6-smc-ui.sh` with environments `X0X_SUPERJ6_FACTORY=1 X0X_S1C=1` and host defines `-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_S1C=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_ARP_HOOKS=1`. For the project fixture (`SMC_UI_TEST_FILE=tests/host/synth_controls_init_only_test.c`) first run without the Init-only C define to write the earlier test projects, then with `-DX0X_SYNTH_CONTROLS_INIT_ONLY=1` and the same temporary `S1_FILTER_PROJECTS` directory to mount them.
4. Use the Init-only C define for `tests/host/ui_arp_engine_test.c` and `tests/host/ui_superj6_osc2_level_test.c`; add `-Wl,--wrap=juno6_init` when running the oscillator fixture.
5. Use the guarded lane native harness and original flash geometry for timing. Do not ship or run on a physical controller on the strength of these host or isolated linker results.

Next: complete source-backed SuperJ6 AMP1/AMP2 link and remaining controls separately from AMP-to-filter LINK, and Synth One oscillator/LFO controls. Verify each physical page, actual voice behavior, independent owners, saved-project recall and locks. Keep measuring stock native fit; a read-only companion requires separate safe storage/rollback and strict heavy replay before any installable build.
