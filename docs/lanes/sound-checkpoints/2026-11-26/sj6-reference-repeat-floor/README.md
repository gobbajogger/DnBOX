# SuperJ6 original-to-original release variation

Run `python3 score.py SJ6-Three-Factory-Presets-original-AUv3.tgz`
with NumPy. Bundle SHA256:
`0aaf6375083dfb2bb1224fe690340ed2eff4ad978f878d099e2c19913af60603`.

The saved archive's top-level automation result reports an earlier failed
preflight; this test accepts the individual PCM/capture pairs only after
verifying PCM hashes, frame count, applied/read-back state, component
identity, identical MIDI schedule and source preset across both authored
takes. This does not certify the enclosing automation. No PCM arrays or
original app data are embedded here. Windows are anchored to MIDI release,
and stop at the following event, not at an arbitrary global tail split.

- Raw authored repeats exactly, including both releases. It can support
  strict signed checks for the same cue and original configuration.
- Brainwave authored repeat residual is −43 to −64 dBr across release
  windows with virtually unchanged RMS level.
- Solidify authored note-off is materially variable. At the final
  release the first 16 ms average −25.17 dBFS with original-to-original
  error −27.26 dBFS. From 16–50 ms, the repeats differ by 6.00 dB of
  level and −68.49 dBFS signed error. From 200–500 ms even a +7.35 dBr
  repeat ratio is absolute −130.89 dBFS.
- Only one dry take exists per preset in this bundle. Do not subtract
  authored variability off a dry mismatch, claim the compact phase
  matches, or change the preset. Compare absolute error plus signed
  attack, decay and spectral shape; use deterministic original cases
  and controlled multi-run energy comparisons separately.

The causes of original repeat variability are not assigned here. This
is a test of the reference limits, not a firmware qualification waiver.
