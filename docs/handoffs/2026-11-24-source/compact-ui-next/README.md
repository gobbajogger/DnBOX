# Typed ROM paging capacity on exact 602ed70c (parked, not release fit)

Input: `perf-final-proof/controls-init-only/source-s1-mono-sj6-id1`, source
manifest 602ed70c. `capacity.patch` is the only product-source proposal.
`host-adapters.patch` updates every direct private page/list consumer found in
the snapshot; this includes a generated fixture header that can alternatively
be regenerated from the adapted ui_synthone_filter_mode_test.c.
`synthetic-only.patch` is never a production patch.

The 15-page/64-row baseline becomes 23/92 using the same 700 bytes of POOL
arrays (444+256 → 424+276). Page titles are typed at compile time, immutable,
and legacy-truncated exactly. There is no runtime title-string lookup. ROM
descriptors use one-byte title IDs; SMC keeps its existing 36 B query and
exact string return. The transient SEL list packs refs to three bytes, with
lossless GEN aliases FE/FF; underlying four-byte prefs and DNR3 wire don't
change. Future refs beyond the bounded tuple domain are not admitted.
The shared MORE ref budget may reduce the attainable pages for a context.

Exact native links without geometry changes stopped at original ld:
| Model | Baseline XIP headroom | Typed candidate headroom | Delta | POOL used both |
| --- | ---: | ---: | ---: | ---: |
| Elite | −30,260 B | −30,612 B | −352 B | 0x51de4 |
| Pro | −30,248 B | −30,600 B | −352 B | 0x51de4 |

Both models preserve 8,732 B free POOL, 28 beyond the reserve. Both still
fail stock XIP; this is a POOL-neutral source candidate, not installable
or release-qualified. Neither package, native image, device nor Git changed.
No guarded diagnostic address/profile was enabled, and no emulator/latency
claim follows from these failed stock links. Before landing on a fit source,
repeat full native limits and broader ordinary host qualification.

From two disposable identical source copies: copy ui_page_budget_test.c
to tests/host in both, run run-page-test.sh with flag argument '' and a unique
output directory for baseline; apply both non-synthetic patches to candidate,
run with '-DCOMPACT_PAGE_BUDGET=1' and a unique output directory. Replay each
produced test binary with stdout/err separated. Each 1,456 checks, both
771 stdout records (including 102 framebuffer checksums) byte-identical.
For overflow replay apply synthetic-only.patch to candidate, and run with
'-DCOMPACT_PAGE_BUDGET=1 -DPAGE_OVERFLOW_FIXTURE=1'. PASS 35 checks: physical
page index 16, SMC 22 pages and exact selection, 85 SEL rows and physical
index 70, SOUND last, shared ceiling and three ordered MORE refs.

Native reproducer: existing sj6-knob-persistence/native-link-only.py with
same pinned SDK/toolchain and identical pre-generated locked stock assets
(the baseline Elite generated header directory, copied into both model
trees; model register/build selection still comes from --model). Setup:
X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_ARP_HOOKS=1
X0X_SYNTHONE_PATCH=1 X0X_S1C=1; --board smk37 --model elite/pro; set PYTHONPATH
to each source copy. Default output paths build/smk37-elite and build/smk37.
The driver disables packaging and bootloader and obtains maps even on overflow.
