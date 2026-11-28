# 64-row display: stopped-project Save at the live tail

Start from the marked-stack 64-row source described in
`canvas-64-stack-tail.md` (including `canvas-64-stack-witness.patch`); this is an extra simulator-only witness, not
part of the normal display implementation. Inject the same manual Save
handler from the real LCD input pump with either live tail owner selected
(owner 1: main painter, owner 2: bass footer). Use disposable virtual
NOR. Disable the boot-time Synth One selection and automatic loaded
transport fixture so the legacy footer and true immediate Save are reachable.
Set a stopped project's live tempo, then mark the project dirty; setting
only the cached storage field is not sufficient because project_save
captures the runtime tempo. Reboot against the same NOR and compare the
loaded marker before issuing any second test save. Draw input is not
disabled. Use `read-barriers.py save RUN` to read the witness. A memory
watermark records written bytes, not every SP low water. This test does
not qualify the entire call graph, LCD timing or a release.

Use the original steady test build recipe, but the shorter `--shot 3000`;
copy the generated flash image from the first run to a fresh emulator
working directory for reboot. Do not use a hardware unit. Apply
`canvas-stopped-save-witness.patch` with owner 2 for legacy
footer; switch `BENCH_SAVE_OWNER` to 1 for the main owner. A first-run
witness state of 3 reports that the handler returned; dirty/save-failed/
queued fields must all be zero. State 4 after reboot proves it loaded
the stored tempo (146.2 BPM for main, 146.3 BPM for footer).
In the Elite runs both paths saved and reloaded successfully, with
0 oversized requests, 0 skipped emulator cycles, and 1,884 untouched
marked stack bytes (8,612 of 10,496 seen written). The reboot did no
flash writes. This closes the initial stopped-Save concern in the tested
paths; it still does not prove actual lowest SP or other nested routes.
Preserve the 69-row preferred branch until the remaining display guards
are satisfied.

Pro independently reproduced both routes and reload markers, the same
1,884 B untouched on actual Save, zero canvas overflow and zero writes
on reboot. All four saves used the real project writer and all four fresh
boots used the ordinary reader. These witnesses borrow reserved stack
rather than reducing total working RAM; they do not qualify the 64-row
build as a release.
