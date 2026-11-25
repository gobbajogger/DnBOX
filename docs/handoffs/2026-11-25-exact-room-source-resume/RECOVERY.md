# Byte-exact guarded recovery following runner rollback

This is source/recipe only. Do not commit generated private coefficients,
ELFs, companion images, maps, or recordings. The same private donor input
hashes, SDK/compiler and guards remain mandatory; no package/deployment.

Prerequisite workspace fixture: all-controls-remote-join/fast32-fit/tree
and its link-guarded-exact.py, all-controls-remote-join/compile-flags.txt,
prereqs/jieli-linux-toolchains-20260730.1 and prereqs/sdk. None of the earlier
directories were modified. Under rehydration root checkpoints/, export Git
docs/handoffs at these exact source commits (strip docs/handoffs/ prefix):
9bea695f, 5743178d, 352bb60b, the huff/tanh/Slot3 pack at a23920fa:
docs/handoffs/2026-11-25-huff-tanh-slot3-fit/, ca1dd548, 908878f4,
51116efa, cf33bb05, e2c8fdca.

1. Place restore-near.sh in rehydrated-reverb/ under that workspace. Run it
   then create sibling symlinks prereqs and all-controls-remote-join to the
   workspace originals, and guarded-fit-miami-{cap,decimator} to their
   original checkpoint directories. Run near-bound/replay-near-maps.sh.
   Verify every PAIRED-SECTION-HASHES.sha256 entry from near-bound/tree/labout.
2. Run restore-cold-room.sh <rehydration-root> <workspace-root> from this
   packet. It applies fold-sparse (confirmed exact historical source, not
   the unlinked bitmap arm), the newly preserved Chicago speed patch and
   generator, then cold, packed, typed UI and finally dual Room patches.
   Both matched native pairs use the same documented Drive/Speed flags.

Actual matched extents and historical full ELF SHA256:
- Near: Elite +32 B APP, Pro +40 B; +412 above 8 KiB reserve. ELF
  5bcd76a82e44a5ce135157c8ccac3ce21be212c3783d285772588a77585e5594 and
  7145d630918e221c6ef64d7da5d976a3126c42ab4cbd56b0012e1f2b3e77f394.
- Cold/typed: Elite +920 B APP, Pro +932 B; +564 POOL. ELF
  02ceb3c1c7a3b032e24a0a149daf3a00238f869c8628ba73975e8fe6815f1147 and
  c2e88ec1e2f8efb87933a7c7f1f3eb25b4e945a2747a3de750cd0af8c5bd1dfe.
- Dual Room: Elite +8 B APP, Pro +20 B; +564 POOL. ELF
  14560fd643bf571c2e36fdf5e5085d17cc3233b218d1a9bbbfe6b5f2befa2e43 and
  0612a1ae686b516bd42ffc93e681736d34917f91ba7810c10899261baf8888df.

Exact guarded link wrapper SHA 50ed3c11ad8f5997d57307e45c776168ed1efcaa8d5a6f026cfcd572742fe134.
Exact dual SHA: ui.c 7301a68765e66868d0e3fea6b7162fb42660e0b1398065ebd49fec30678c9059,
s1c.c b5afd6fc243d80fafaca15127cf592045587946fa308a1463145aeb5b332f4b6,
revsc-proto.inc aabe7d737212e811706f3f82e070d7e7754fb85c6c3e18d51a01190344da744d.
All historical Near image hashes, and all final dual binary segments and
ELFs, were rechecked (receipts enclosed). Raw map/script outputs contain
temporary object names/local paths and are not a binary-equivalence oracle.
No new playback, firmware write, migration, or release qualification.
