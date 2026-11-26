Warm guard donor Elite 39986b42b39fe3e300cdf8a7fb60996ef475580118c664f7efac4961d9d82918 APP +252
Warm guard donor Pro df5831a1a84f8b52198f9af9112ac44a97c12759743d710d4f2b35d215db4570 APP +260
Apply wet-lease.patch then table-share.patch to matched source donor. Append -DDNBOX_S1_WET_LEASE_LAB=1 -DDNBOX_S1_WET_LEASE_USE=0 to original strict warm PI32 wrapper CFLAGS; invoke cold-build.sh 2 elite/pro each. No EXP/reciprocal reapplication: already in baseline.
Result +632/+644 APP, +564 pool after protected 8192 B; same FDATA/FITTAB/BREAK and no globals freed. No activated delay, partial strip, timing, device or production qualification. Independent earlier same-value table/808 tests and bit-identical lease layout test do not replace whole timing testing.
