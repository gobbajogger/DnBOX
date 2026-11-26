# Synth One packed source morph exact neighboring fold
Donor: retained latest table-share native compact source with actual KR path
and shared-tight morph tables. Exact source/asset pins in source-and-native.
This change reuses adjacent phase folding and sample sign within uninterrupted
monotonic rows only. Original folds apply at reflection/subsection corners,
array wrap, and other shapes. No modification to tables, interpolation, PCM
operation order, controls, arbiter or persistent state. No raw table or
firmware binary included.

Native strict PI32 isolated exercise is eight fixed cases ×8192 samples at
360 MHz, exact Fp flags with FNV over every Float32 output word and final
phase readback. Inputs cover all four static waveform endpoints, moving
morph both bands and band-zero, high-band in-between, and wrap above 1.
All outputs and phases identical. Measured leaf time savings 0.39–5.79%,
sweep 1.03%; payload 20725→20793 (+68 bytes) with same source table data
and no RAM. The isolated test measures source leaf only, not native
whole firmware timing, APP link margin, total launch CPU, or release
qualification. Legacy exact 906-case reference remains unchanged (not rerun).

To replay, create sibling fold-probe in the same workspace and copy the
five donor files from pinned table-share tree/dsp/s1c into base and src,
then apply fold.py only to src/morph-paired.inc. Follow run.sh from there.
Host check compiles decoder.c twice -Os -ffp-contract=off with
DNBX_MORPH_PAIR=1 and DNBX_MORPH_SAW_SHARE=1; prefix the reference
object's globals with ref_ via objcopy; link host-check.c with both. It
exhaustively tested all 212992 row/phase paired words plus 262144 varied
entire tick PCM/phase cases, all bit-identical.
