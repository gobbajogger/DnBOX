# Reusable sound reference testbenches

## Synth One: original stereo reference

- [Linux runner](../../tests/host/run_synthone_factory.py), [fixture and commands](../../tests/fixtures/synthone-factory/README.md), [verified packaged build inputs](../../assets/test-inputs/synthone/README.md).
- [Three-second full original graph and comparison recipe](https://github.com/gobbajogger/DnBOX/blob/36601a5855fd64fe9262bf9eb2db1b68643a5d17/experiments/sound-hardware-v1/synthone-factory-projection/README.md). Reference digest source: [694 per-preset records at 36601a58](https://github.com/gobbajogger/DnBOX/blob/36601a5855fd64fe9262bf9eb2db1b68643a5d17/experiments/sound-hardware-v1/synthone-factory-projection/rows-35.jsonl.gz).
- The output-only bank was reproduced from the original engine with authored effects: 694 presets, 12 banks, 132,300 interleaved Float32 little-endian stereo frames per preset at 44,100 Hz, 734,529,600 PCM bytes. All 694 clip SHA-256 values matched the pinned records, and all entries were read back and checked inside the archive.
- Archive identity: `synthone-factory-original-694.tar.zst`, 617,766,687 bytes, SHA-256 `67d6002ff2dc78a97bf0131705c8eebfbe28c5f84d817d26f3e8c69b5d9fae9d`; internal `manifest.json` SHA-256 `49f51b867d555649880bd9db54d05ab33a29454b7fe249b6d311ab2867974493`. [Download the verified 26 parts and checksums](https://github.com/gobbajogger/DnBOX/tree/159de9c9a02539d09821e4e408d051c1f327de95/assets/test-inputs/synthone/reference-audio). This is a dedicated asset branch, not a release. All parts were downloaded back from this exact commit, checked individually, then reassembled to the archive SHA-256 above.
- For an optimization, reuse a verified original capture. Render only the candidate with the same notes, sample clock, and output length. Keep level and spectral scores separate from a bit-exact regression check; do not normalize away level differences. A verified original capture is not evidence that the candidate matches it.

## SuperJ6: Linux ARM64 component bench

- [Original component replay and inputs](../../experiments/sound-hardware-v1/sj6-factory/original/README.md), [runner](../../experiments/sound-hardware-v1/sj6-factory/original/replay_reference.py), [factory/S4 stimulus and scoring](../../experiments/sound-hardware-v1/sj6-factory/README.md).
- Replayed on Linux with Unicorn 2.1.4: seven original synth, sampler and chorus clips matched the expected digests without Apple runtimes. The pinned AU and the two private input archives are named in the instructions; no original installer belongs in Git.
- This is a verified component bench, not a complete preset renderer. The target remains 640 full factory captures and 14 separately scored S4 stereo pairs. Do not count component clips or setter traces as full presets.

## Firmware optimization loop

Reuse the same saved project, event script, stock profile, model and emulator build for baseline and candidate. Start with a short deterministic comparison of the changed workload; record firmware and input hashes, peak render time and margin, late transfers/xruns, stock memory budget, and the applicable audio identity or fidelity score. Retain changes only when its intended metric improves and its guards still pass. Use the existing strict full-length Elite/Pro scenarios for final qualification, not for every candidate. A clean short run is not a release gate.
