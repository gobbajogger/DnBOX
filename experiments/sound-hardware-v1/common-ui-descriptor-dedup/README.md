# Shared const UI descriptor data, Elite + Pro

Independent UI-only source patch. Reuse existing OFF/ON; share identical
FREE/SYNC labels, identical Synth One/SuperJ6 SEQ ON descriptor, and the
four identical Master and Wave bindings. No DSP source, parameter values,
default, order, labels, or renderer mechanics intentionally changed.

Guarded vendor-native links under shared flock. Unmodified donor build
reproduces exact previously reported hashes; patched +76 B APP on both,
624/636 to 700/712 B, pool unchanged (4480 above 8192, 3968 above 8704).
Only ui.c differs from donor. Source check resolves text labels and
descriptors; ELF check confirms old duplicates absent. Full archived
host Master/Wave and arp fixtures did not build on reconstructed tree;
the selected Master fixture references absent mix_gr_meter and existing
host API hooks. Do not treat source comparison as a screenshot pass.
No device, replay, gate, Git, or parity claim.
