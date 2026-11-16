# Explicit representation copies, bounded candidate

The four float representation copies in compact morph now use explicit compiler
builtin memcpy with unchanged addresses, widths, word order, interpolation and
phase arithmetic. This retains alias-safe copying; it does not reinterpret a
uint32_t pointer as float. Default `-fno-builtin`, heap interception, floating-point
flags and all original tests remain in force.

The retained baseline produced 98,304 external copy calls over 24,576 tested
frames; the same real-reader/state/output test now produces zero such calls and
zero differences. This is operation-count evidence, not a timing qualification.
The runner stages the pinned baseline source/header closure and current reader,
records the exact compiler and verifies actual local compiler dependencies.
Its system headers/toolchain remain host dependencies, not a hermetic toolchain.

Fresh original-source comparison: 12 scenes / 576,000 frames / zero differing
channel words / zero render heap calls. Output SHA256 is unchanged:
`4b32746b36590998cf7c215ec07d5890d394414e8b2702b7a1a5e847c8ed6b34`.
Frontend: 6,385 changes / 957,750 target-word comparisons / 4,096 frames / zero
differences or control heap calls. Both actual quad-route finite-wrong-word and
oracle-window negatives fail as expected. Original oracle compiler warning
remains visible in the log, not suppressed.

Pinned native component compilation: compact morph executable section 598 to
556 bytes (-42); every other allocated section unchanged. Owner remains 7,972
bytes, alignment 4. This is not a combined application link, full stack bound,
stock-memory admission or device measurement. No full 694 replay or deadline
qualification for this candidate yet. Earlier callback failures remain failures.
Independent source/runner review accepted the narrow compiler route and freshly replayed the 24,576-frame test with zero differences/copy calls, verifying all staged hashes. No S1–S7 completion claim.

Original complete 38-case sequence replay also passes: 114,196 frames, 120,289 state snapshots, 1,081 events, zero failures/runtime heap calls. This still does not qualify the complete app or deadline performance.

Native observed prologue scratch for morph drops 44 to 28 bytes, saved 13 registers unchanged. With unchanged quad/physical-row frames, the observed three-frame prefix is 316 rather than 332 bytes. This excludes deeper callees, callers, interrupts and runtime/library frames and is not a full stack bound.

## Frozen d9ae corpus and deadline characterization
All 694 programs / 66,624,000 frames have zero differing or nonfinite words. Every one of 1,388 PCM lengths/hashes equals retained manifest68c7f2d. This is source/common-FX evidence, not independent app/FX/tail qualification.

Same-host ABBA on eight programs remains FAIL in every run: A10/10, B3/3, B2/2, A1/1 wall/CPU misses. Each records6,000callbacks,768,000frames,benchmark tracer0 and0nonfinite. A is retained a803, B is d9ae; all bound inputs match except morph source. No samples, warmup or outliers were discarded. This does not establish a causal speedup or zero-xrun qualification. Raw traces remain preserved locally under the stopped trace-publication boundary.
