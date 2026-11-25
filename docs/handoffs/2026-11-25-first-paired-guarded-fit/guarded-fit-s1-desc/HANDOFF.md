# Exact follow-on: XDATA tail and compact S1 descriptors

Complete huff-tanh-slot3-fit first. At its enclosing directory apply
xdata-runtime-guard.patch at -p1 to add boot-side exact XDATA tail CRC.
On the protected isolated source tree apply s1-descriptor-fit's
compact-s1-descriptors.patch at -p1. The companion map change intentionally
drops removed .rodata.extra_parameters and follows the successor immutable
.rodata.patch_parameters (208→496 B). Apply xdata-and-s1-placement.patch
to the enclosing three-way linker wrapper, i.e. not separately to the stock
wrapper. Use replay-maps.sh with all four diagnostic flags.

First actual XDATA-only join measured APP excess Elite 3,416 B and Pro
3,408 B. Compact S1 descriptor join now measures 2,400 B Elite / 2,388 B
Pro. Both still hard reject. XDATA holds exact 480 B two immutable owned
descriptors and existing unchanged font prefix, 12 B free. FDATA 0xee28;
Slot3 0x7f9c, 100 B free; FITTAB 0xf73; POOL 8,604 B free including the
reserve; BSS 172 B free. See compact map receipt. Actual mapper enforced
range/pair checks; no linked byte seals, runtime/timing or gate tests,
packages, device, physical layout edits, or release.
