# Typed Synth One-only: Elite and Pro real link and symbol audit

Start from the earlier RAMTEXT guarded source bundle. In a copied donor,
apply build-admission.patch from the donor root and then the three
s1-carrier patches from the donor's tree/ directory. Use the included
s1-only.flags as compose-flags.txt and run the patched run.sh under the
native build lock, with the original SDK/toolchain and original donor
generated assets. Then run measure.py against the paired
tree/joint-diagnostic-later outputs and the engine_profile.py audit helper.

The typed Synth One controls now compile under independently disabled
SuperJ6; the build includes the expanded Synth One control path, not the
reduced link pilot. The audit finds Synth One and no new SuperJ6 DSP or
object files, on both models. Legacy stock Juno remains intentionally.
The link measures 74,380 / 74,388 bytes of free XIP and 17,832 B of
audio pool beyond 8,192+512 B reserve on both. Neither sound parity nor
dropouts have been checked in these binaries; typed Remote is still off.
The diagnostic wrapper explicitly stops after ELF and map, before packaging.
The scratch union assertion is widened only for measurement; neither the
layout assertions nor saved-project handling are certified for production.
The split refuses explicit SuperJ6 typed NOTE137 locks, but absence
detection for all older/ambiguous shared sidecars remains unproven.
