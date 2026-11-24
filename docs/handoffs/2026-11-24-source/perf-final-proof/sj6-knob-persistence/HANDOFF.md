# SuperJ6 oscillator and PWM panel proof

The isolated candidate starts at 1cb4b0af6361e3d1a1821db87650c20a79ab7270. The exact pre-existing engine worktree diff and keyboard fixture are retained as the first two patches. Apply the following in order to a clean checkout of that commit.

1. engine-first-input.patch
2. engine-first-keyboard-fixture.patch
3. superj6-oscillator-knobs.patch
4. synthone-filter-link.patch
5. arp-gate-fixture.patch
6. existing-panel-fixtures.patch

The SuperJ6 patch replaces the compact oscillator macro row in the engine-first profile with two pages for the seven build-9 IDs in BUILD-9-CONTROLS.md. Waveform, range and PWM changes apply at the audio boundary. They save with the 32-byte sound cell and reset on a new preset. The proof uses the actual UI input path and the actual engine, and saves and reloads through the project's host storage implementation. The latest dry and compact host logs each record 73 oscillator and persistence checks plus 126 existing arp checks. On each owner and renderer, every tested control changed at least 8,174 of 8,192 post-detent PCM samples on a sounding note. Saved and reloaded PCM matched exactly.

The Synth One patch adds FILTER page knob 4 LINK while keeping the two existing ADSR pages. Both host renderers passed 40 checks across the two parts. On the compact renderer, 8,014 of 8,192 samples differed per owner. The retained FILTER values survived LINK on, reload and LINK off. The unlinked output matched the initial PCM bit for bit. The separate SuperJ6 DSP-only fixture matched the explicit linked ADSR for 6,144 frames and restored the independent contour exactly when set off.

The combined host ARP patch is a regression fixture only. Its 66 checks include the pad-plus-key alias, cancellation on page change, both backends, and GATE on knob 3 written to the actual ARP block. The stock gesture path came from the recovered worktree. These checks use the production host input hooks and renderers. Native electrical behavior and original-app ARP/SEQ parity remain open.

Both normal-link native profiles retained the existing engines. The oscillator-only candidate overflowed XIP by 7,972 bytes on Elite and 7,960 on Pro. The combined source overflowed by 8,492 and 8,480 bytes respectively. Link logs and maps are included. The full strict timing run requires a linked candidate on both models and remains unrun. Prior laboratory slot 3 receipts describe a different source and layout. No device, installer, release or main changes were made.

For focused host replay, run the standard `tests/host/smc_ui_test.sh` from the checkout with `X0X_SUPERJ6_FACTORY=1`, `X0X_S1C=1` and `X0X_HOST_DEFS='-DX0X_SUPERJ6_FACTORY=1 -DX0X_SUPERJ6_BUILTIN_INIT=1 -DX0X_ARP_HOOKS=1 -DX0X_SYNTHONE_PATCH=1 -DX0X_S1C=1'`. Set `SMC_UI_TEST_FILE` to `tests/host/ui_superj6_knobs_test.c`, `tests/host/ui_synthone_filter_link_test.c` or `tests/host/ui_arp_keyboard_test.c`. The supplied host-with-existing-inputs.sh uses the already generated headers and was the replay driver in this checkout.

Remaining implementation includes the reconciled 64-control inventory, SuperJ6 panel AMP and FILTER ADSR with LINK, original modulation and active FX, original sequencer behavior, and the full 640-program selector and playback. Both bank sources were kept intact. Continue with authorized, version-pinned behavior evidence and the original timing gate.

The updated existing Synth One panel fixture passed 1,577 checks, and the engine fixture passed all 13 factory control sweeps of 128 positions plus note locks, preset replacement and project reload. Stock ARP passed 336 checks, including its original 16 macro positions. Engine-first ARP passed its 126 transport and pad checks with the new oscillator fixture covering the replaced panel. The original typed factory lifecycle passed 32 synthetic applications, 288 authored-format words and 64 PWM defaults with its stock-selection profile.
