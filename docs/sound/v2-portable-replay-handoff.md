# Managed replay / artifact handoff, not a second lane writer

Standing owner permission (copy verbatim into a delegated task):

"I authorize these GitHub actions in gobbajogger/X0X37-for-Loopy-Pro and gobbajogger/x0x37-for-Loopy-Pro_nightly from now on, without asking: create and push lane/*, integration/* and backup/* branches; open draft and ready PRs into integration/dnbox-consolidated and merge them after the three checks in RULES.md rule 9; comment on PRs; fast-forward gh-pages after the Local gate; create archive/<branch> tags and then delete that branch or close its PR as RULES.md rules 9-10 allow. Never force-push, never push or merge to main, never change repo settings."

Task scope is narrower than that standing permission: read-only exact-checkout
replay and local archive verification only until root explicitly starts the LFS
transfer phase. Do not write refs, commits, PRs, branches or repository settings.
Do not initiate an LFS upload yet. Sound remains the sole lane writer. Do not
retry or route around the separate Library401 failure.

Root supplies the immutable runner commit. Check out that exact commit in an
isolated worktree, not main and not a moving lane head. Read AGENTS.md, RULES.md,
CHECKLISTS.md and docs/lanes/sound.md. Use the portable restore/replay commands in
assets/test-inputs/synthone/README.md. Never reuse retained objects or mark
missing audio/device/native/xrun tests PASS. Keep source and output manifests,
compiler versions, all logs and outputs. Do not modify the fixture to obtain a
matching hash or drop programs that fail.

Compare every recreated output to docs/sound/v2-synthone-factory-pcm-manifest.json.
If all match, run the deterministic USTAR packager. Expected artifact:

- Size:533934080bytes.
- LFS SHA256 OID:4e80387b4fc714c33b7239e27e0497c76c5cd15f2e496dbfaf45d2c709fdaa9e.
- Per-output manifest SHA256:68c7f2d14fdcd1df74aa2888bb0b2d9e3bf77e500232a8cc4ecdff45bcb3b508.
- Recoverable files:1388 (694original/compact pairs), hardlinked only where exact.

If any output or archive hash differs, stop that artifact's transfer and report
both hashes, compiler/libm environment and exact failing filename. A different
recreation is not the preserved original. Keep both sets. Do not call an LFS
upload successful from a dry-run, pointer creation or local object existence.

After root separately starts upload: upload only the verified LFS object and
return OID, byte count and server/readback result. Do not advance any Git ref.
Sound then commits the pointer and LFS attributes through its existing branch,
only after the object is verified remotely. No duplicate PR or competing writer.

These outputs are deterministic test intermediates, not unique microphone
captures or finished-lane deliverables. They remain preserved under rule13;
this document does not silently exempt them or authorize deletion. Root's
managed environment has authenticated Git and git-lfs; actual LFS authorization
and cross-environment bit reproducibility are not yet verified.
