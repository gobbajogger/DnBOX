# Wet-zero invariant and future activation test

Original established S1 wrapper (fw/tests/fixtures/synthone-factory/fullfx/
source_fx.hpp:222–258) computes high pass, compressors and sp_revsc_compute
before multiplying reverb mix, enable, and LFO and crossfading. Zero mix
and enable-off are output controls; neither suspends nor resets RevSC
history. Zero current wet output is not permission to bypass the network:
doing so could regress a later wet reveal. Native compact RV4 and RV8
also compute feedback upstream of wet gain.

Dry Init has DLON=0, RVON=1, RVMIX=0. Current RV8 adoption checks DLON
and RVON, not whether the patch starts with audible room. It replaces
the warm *algorithm* for Init, not just the CPU cost of an intentionally
warm effect. Final exact Elite normal states show old mode fb=0.5 and
new RV8 fb=-2 while both have RVMIX=0. Do not graft this variant.

Narrow future experimental policy, not shipped: choose RV4 or RV8 once
at explicit program install/arena bind, before either history runs. Dry
Init stays RV4; an initially authored wet, delay-disabled reverb-only
program may choose RV8. Keep that kernel running through wet and delay
toggle edits. Do not reset a live history or use RVMIX as loop bypass.
This leaves an explicit fidelity limitation: a dry-start user patch
remains on the warmed RV4 when wet is raised later. Upgrading at the
first audible wet frame needs exact fast in-place history migration or
one fixed quality-preserving kernel; do not wipe existing state.

Test each initial program class using sustained deterministic input at
mix zero with Reverb Enable on. At known audio boundaries raise mix,
toggle Delay on/off while the room plays, keep mix zero for a feedback
cycle and reveal it again, then repeat with Reverb Enable off/on. For
each reveal compare the wet component to a shadow run of the same
selected kernel whose history was never reset/skipped. Hold input,
delay mix, smoothing and routing identical during each comparison.
Require unchanged room mode, preserved local state/stride, no empty
first revealed block after warm input, matched closed/reopened histories
and no owner bleed. Check dry Init against RV4 output and CPU. Both
loaded native and six genuine host pairs still gate any integration.
