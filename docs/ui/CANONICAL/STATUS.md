# Canonical gate: state at the freeze (2026-11-16 14:2x UTC+10)

Work frozen by the user's decision (Dot owns all DnBOX work). Done and not done:

Done
- 777 canonical states enumerated from the firmware (`tests/host/ui_visual_gate/catalog.py`, `scenes.json`).
- Per state: `golden.png` (host render at 94197fc8), `target.png` (R22 gallery view), status + deviations (`index.json`, `CANONICAL.md`).
- Pixel-diff gate `tests/host/ui_visual_gate/gate.py` (tolerance 0, coverage re-enumeration, `--accept --who --words` writes the approval note,
  `--conformance` non-blocking report). Host sim: `uistate` command and `X0X_HOST_FIXED_CLOCK=1` (host/x0x_host.c only).
- Green run at base: `proofs/green-94197fc8.log` (777/777 pass).

Not done (FAIL until done)
- Red proof (deliberate 1-px change must fail) — not recorded.
- Anti-revert guard `tests/host/no_revert_check`, its red/green proofs, `MERGE-GATE.md` — not written.
- Registration in `tests/run_tests.sh` — not done. CI: no host-test workflow exists on this branch (removed in 0032dabd / ed49a00f / bc502b6b).
- `NON-CANONICAL.md` index of old captures — not written.
- Full host suite and Elite/Pro build-delta proof on this commit — not run.
- SMK-37-only screens (clip grid, WAITING FOR LOOPY PRO, pad calibration) and the SMC screen are not rendered by the FM-1 host sim: not covered.
- Target mapping outside the hand-mapped R22 views is auto-nearest (distance in index.json) and needs review.
