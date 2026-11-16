# Exact duplicate-coordinate reuse

The quad reader retains both row admissions, every final word-validity check and
its transactional output commit. It reuses a local uint32 word only when the
physical row and sample are exactly equal. No persistent cache, binding change,
owner growth, floating arithmetic or skipped distinct coordinate is introduced.

Real instrumented RED observed four physical evaluations for an aliased-row
case expecting two. GREEN observes 4/2/2/1 for distinct, aliased rows, repeated
positions and both. Both row admissions remain. All 44,313,152 comparisons with
the retained scalar reader pass, including invalid/alias boundaries and the
additional same-row late-failure no-mutation case. Independent source review
accepted this narrow change and its tests, not an independent 44M replay.

Fresh 12-scene/frontend original-source replay passes with unchanged output.
Native component reader executable section increases 3,142 to 3,184 bytes (+42),
other allocated sections and 7,972-byte/alignment-4 owner unchanged. This is not
combined fit or timing success. Default sanitizer and full corpus are pending;
prior failed deadlines remain failures. No S1–S7 completion is claimed.
