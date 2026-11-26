# Result: shared shape improves, no signed whole parity and no fit

Parent reference archive SHA b6c454f6839703edc5ca8ee6dff2cadb76d99781ba836b4580a1d79897d762de.
Verified 26 capture PCM hashes and fixed profile maps. Per shape: one A4
anchor, one A4 repeat and 11 independent non-A4 keys, including six static
blend probes. No live bend in these particular AU captures. The independent
original-stage bend comparison is separate.

One source only, frozen H2–H18 normalized to H1, with known linear transfer
deembedding only; no compensating common gain or phase shift in signed test.
Relative prefilter harmonic-shape residual, phase-referred to independent H1:
N64 Sine −54.89…−83.04, Tri −43.66…−74.99 dBr.
N128 Sine −55.07…−95.20, Tri −44.37…−87.68 dBr.
These are relative complex harmonics, not a signed PCM cancellation metric.

Signed N128 held comparison with known source envelope settled and known
forward scoped suffix, exact original cue sample indexing, no inferred gain:
Sine −3.41…−2.53 dBr and absolute level deficit 9.77…11.93 dB.
Tri −3.24…−2.54 dBr and absolute level deficit 10.13…11.92 dB.
This is a constructed same-control stage diagnostic, not a full owner. It
does not simulate the original attack or release, or qualify sound parity.
Do not silently overwrite the remaining shared gain discrepancy.

Hard resource floor using current compact stock donor: it owns 4 float
oscillator history words per layer. Exact 4× requires 16, or +48 B/layer,
+576 per six-voice/two-layer part, +1152 for two parts, before other work.
Q16 data alone after exact duplicate-row reduction is 1,408 B (N64) or 2,816
(N128), excluding selector map, code and ownership. No attempt to link or
assert runtime performance; no status change for the default firmware.
