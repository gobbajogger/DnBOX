# Exact no-shadow/no-Fold active quiet-Sub eligibility probe
Scope: instrumented S1 owner-A diagnostic, not a speed benchmark. Enabling
only DNBOX_S1_SUB_ADMISSION_METER in the copied exact source applies the
provided patch without fuzzy matching. No DSP skip or sound changes.
Patched APP free +1168/+1180, pool +564. Probe is 24 resident bytes, with
diagnostic instructions in the active selected voice path. Both artifacts
verified with the original seal/emulator/tape and exact SRAM, actual nonzero
mix and no virtual flash writes, no physical operation.

Both 1MiB dumps have all six meter words zero at the exact linked symbol
including stamped identity and call count. Positive owner endpoint answers
and a mixed PCM peak do not establish a call to the particular S1 voice path.
The probe disassembly contains the exact original_bass address check and
s1_sub_meter store. No active SUBV or eligibility number can be inferred.
No quiet-Sub candidate adopted. The original strict parser rejects an empty
meter; audit-meter.py preserves this targeted no-evidence verdict without
converting zero words into a numeric eligibility pass.
