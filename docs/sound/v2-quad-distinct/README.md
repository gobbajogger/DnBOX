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

## Frozen 3e41 qualification evidence
Managed default ASan/UBSan/LSan completes 44,313,152 comparisons with exit0,
options unset. Exact 3,094-byte receipt and 271-byte full log are recovered and
independently verified against the published source. Passive tracer samples are
not continuous trace-state proof. Scope is reader memory safety only.

Complete 38-case source sequence suite and original frontend/12-scene renderer
pass. Complete 694-program source/common-FX replay has 66,624,000 frames,
0 differing words and 0 nonfinite words. Independent output audit verifies the
exact 1,388 filenames, lengths and hashes against retained manifest68c7f2d.
This does not supply an independent full-FX/app oracle or long-tail coverage.

All deadline runs still FAIL. Same-host d9ae(A)/3e41(B) ABBA records wall/CPU
misses A11/8, B28/28, B11/9, A2/2. Every run has 6,000 callbacks, 768,000 frames,
tracer0, 0 nonfinite and matching per-program diagnostic word digests. A separate
fixed-process-affinity diagnostic on allowed CPU8 records A28/26, B155/145,
B246/245, A83/83. It does not isolate/reserve a CPU and is NOT qualification.
No warmup, outlier or failed run was removed. Neither causal speedup nor causal
regression is established by these variable-host samples; affinity is not a fix.
All raw traces remain preserved locally under the stopped publication boundary.
