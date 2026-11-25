# Combined strict guarded dual-room diagnostic
Base: exact loaded-perf-undo-caption/tree (APP +920/+932 B and preserved
typed dispatch/Undo safety). Apply loaded-perf-s1-rv8-shared/dual-room.patch
at that tree root with patch --batch -F0 -p1. Only s1c.c and revsc-proto.inc
change relative to that donor. Both parent patches and resulting touched
files are pinned by FIT.sha256.
    
Reproduce from this directory:
    
    DNBOX_CHICAGO_DRIVE_COMPACT=1 DNBOX_CHICAGO_MOD_SPEED_COMPACT=1 bash replay-near-maps.sh

Actual strict paired links: Elite +8 B / Pro +20 B APP, each +564 B over
the 8 KiB RAM pool reserve; read-only FDATA 61432 B, slot3 32764 B (4
spare), FITTAB 4072 B each. Combined flash cost exactly 912 B on each.
The complete guarded link log prints both SHA256 and diagnostic header
identities, and the sealed checkpoint manifest covers both no-install
ELFs, linker scripts/maps and all companion segments.

Independent host component from combined source (private original bank
fixture; does not change native INIT-only configuration):
    
    DNBOX_AB_WORKSPACE=<workspace> DNBOX_AB_LATEST=<this-dir>/tree bash host/build-run.sh

All six dry + six authored wet F32LE outputs byte-identical to the prior
shared dual-room component, as verified with cmp in host/compare.txt.
The earlier comparison to the original Synth One reference is unchanged:
Organ last-second error -4.697→-0.049 dB, spectral L/R errors
9.800/8.525→2.568/2.748 dB; full-waveform residual +1.263 dB relative.
Other five wet and all six dry outputs continue to match the old compact
room baseline exactly. No parity sign-off implied. No native emulator,
hardware, image deployment, source graft or release. Diagnostic no-save
ELFs contain guarded layout only and must not be installed.
