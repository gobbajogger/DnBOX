# SuperJ6 Raw: restored connected A/B, no production change

The recovered warm core (SHA-256 70d0aa0e…) and exact generated Fold header (dd7f52b2…) reproduced all four archived dry/authored baseline and Sub-muted PCM digests. The full previous report was byte-identical (7dd6bec959390a9d787a8fba9f3ed7ca500b2bbeb8a3552d433fb8eac8f538f8). Linux host only; these are not native link or release measurements.

## Connected tuning × even-edge factorial

The candidate changes only the synthetic Sub oscillator and leaves the real tuning inlet in control. The root-36 solo through frame 49,458, the full Sub-muted output, and event/AMP trace are bit-identical between edge-off and edge-on. Report SHA-256 1237a20f4a0dec8548ec7cb4672462fede82a8e00f496b45d24bf84b6c3ef2dd.

| Signed residual vs original, lower is better | Tune only | Tune and edge |
|---|---:|---:|
| Dry complete | −1.719 dBr | −1.751 dBr |
| Authored complete | +1.129 dBr | +1.125 dBr |
| Chorus contribution complete | +0.528 dBr | +0.520 dBr |
| Authored chord attack | +0.797 dBr | +0.825 dBr |
| Authored final tail | +1.029 dBr | +1.032 dBr |

The isolated sustained improvement does not transfer to whole-render parity. Keep this edge disabled.

## Actual first divergence: synthesized waveform before OTA

On the edge-free low solo, the actual compact Sub tap and its independently clocked tuned original reference have the same held fundamental, about 32.685 Hz, but differ by about −9.54 dB and −114.6° at H1. The H3, H5, and H7 phase errors do not follow H1 times harmonic number. The compact onset also first crosses 1e−6 two frames earlier. Correct tuning does not fix the waveform, and a single delay or a master gain change cannot fix these observed differences. The measured pre-OTA held residual is +1.29 dBr. Report SHA-256 c687b0d69668697f22a092bd4057328bb5cc72b36747d007cbb8927e8bffce01. The phase-aware four-odd-harmonic source-only ceiling improved seen roots but failed the unseen source zone, so it is not a candidate either.

The Main-dominant even-bin phase error is a separate issue. Substituting authentic pre-OTA Main left that error unchanged. In the controlled dry comparison, independently identified HP and zero-drive/terminal DC stages took H2 and H4 phase errors from −43.16°/−22.28° to +0.14°/+0.25°, with the remaining flat level gap intact. No gain change or audible match is claimed.

No audio clipping, derivative waveform table, original software, private source inputs, or production changes are included in this record.
