# Corrected same-tree Classic I + existing mode transitions

Apply this exact four-file delta on the provisional Classic I 809 source.
It includes authenticated wrapper replacement (multiplicative signed raw
feedback) and the exact separately fenced existing I/Both/II/OFF switch
bridge, no new controls or carrier bytes. The earlier additive feedback
implementation is superseded, not reused. CORE.sha256 pins corrected core.

Same real host tree: 809 both-order mixed, 119 scoped I, 156 existing mode
switch and retained state, exact 763 reader refused b27–29 before writes and
admits positive control with only those bytes cleared. Original 26,577
bit-identical graph-oracle frames in sj6-classic-modes-oracle, not claimed
as full original AU app or target/native parity. No II/Both physical knob
extension, cold alternate-mode authored state or release admission.

Replay from pinned source: bash replay.sh [exact-source-mod-wheels-directory].
