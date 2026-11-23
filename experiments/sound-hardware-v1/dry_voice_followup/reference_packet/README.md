# Dry Synth One comparison contract

## Concrete next implementation test

**All 60 controlled recordings are recovered and independently verified.** Their original mono columns are in `reference_pcm`, covering the full 180-note physical-gain grid. `reference_pcm_manifest.json` records the archive identity, each member's compressed and paired hashes, and every extracted mono hash. The total mono payload is 63,498,240 bytes and contains zero nonfinite values. Extraction copies column-zero bits without any numeric transformation. The archive contains 153 entries, of which 138 are regular files.

Start with `physical-gain/100_2000_.9`. Its extracted original reference is `reference_pcm/100_2000_.9.original-mono.f32le`, **1,058,304 bytes**, SHA256 **`b02703f0893f9e7676e4db03048f9fefe8575ae1b1a1f44da1167253309a6cd2`**. Its source paired file is exactly 2,116,608 bytes, SHA256 `c1fe9f3357ed6dc3c66254f68d9d806e0f94e6c1ceb4303e3518eacef837728c`. The historical pair contains three consecutive notes, each 88,192 frames, with two float32 little-endian columns: original mono first, historical candidate second. Those columns are **not stereo**. Only original mono was extracted into this packet.

Implement and render the same one-oscillator dry scene at 44,100 Hz, velocity 100, cutoff 2,000 Hz, resonance 0.9, and filter-envelope amount 0. Produce one mono float32 little-endian file with the three notes in order 36, 60, 84. Its length must be **1,058,304 bytes**. All exact binary32 parameter values and the complete 84-file/252-note matrix are in `case_plan.json`.

```bash
python /workspace/scratch/e803955aec60/dry_reference_packet/compare_saved.py \
  --case physical-gain/100_2000_.9 \
  --candidate /ABSOLUTE/PATH/new-candidate-mono.f32le \
  --output /ABSOLUTE/NEW/PATH/independent-pairs.f32
```

The assembly utility selects and hash-verifies the recovered mono reference, writes the new comparison pair, and invokes the unchanged `metrics.py` with explicit comparison gain **1**. It does not render a synth, fit gain, align phase, resample, normalize, trim silence, or alter either input. Output paths must be new. For historical paired input supplied directly, add `--reference /PATH/FILE.f32.gz --reference-layout historical-pairs`. A separately captured mono reference requires `--reference-layout mono-capture --reference /PATH/FILE --reference-sha256 OWNER_RECORDED_SHA`.

## Exact stimulus

Each note starts from a new reference instance. Set all 150 control values from the numeric default vector plus the case overrides; render 2,757 blocks of 64 silent warm-up frames, then note-on at recorded frame zero. Render 1,378 blocks of 64 frames. Note-off occurs before block 1,033, at recorded frame **66,112**. Total recorded frames per note: **88,192**. The amp ADSR is A 0.05 s, D 0.005 s, S 0.8, R 0.05 s; the values in the JSON are the corresponding binary32 representations.

| MIDI note | Exact frequency, Hz | Binary32 bits |
| ---: | ---: | --- |
| 36 | 65.4063949584961 | `4282d013` |
| 60 | 261.6255798339844 | `4382d013` |
| 84 | 1046.5023193359375 | `4482d013` |

These values come from the unchanged fixture's tuning setup, checked against the historical fixture hash. They are not estimated from audio. The reference logs show settled cutoff `1999.99988` for a requested 2,000 Hz; preserve the requested physical setting and warm-up protocol. Do not silently replace the requested parameter with that log value.

The audible source is oscillator 1 at morph endpoint 1, level 1, balance 0.5. Oscillator 2, sub, FM, and noise are zero. Raw oscillator mode, LP, no active LFO routes, no glide, and arp off. Parameter smoothing half-time is 0.000001 s. This dry recording excludes master and post-voice FX. `case_plan.json` contains the full default vector so unstated defaults are not guesses.

## Unchanged metric and gates

`metrics.py` is copied byte-for-byte from the historical evidence. Its SHA256 is `bdb7cb1acab5a450a4f43dada364b78dc116d2bf6a7710ad78f2cfd788af96c1` (997 bytes).

For each note, the metric computes whole-record RMS and the square root of the average power spectrum across 2,048-sample Hann windows with a 512-sample hop. The spectral floor is the larger of 1e-4 times the reference spectrum's maximum and 1e-12. It compares the union of frequency bins above that floor in either signal and returns the root mean square of their dB ratios. All 88,192 frames remain included. The exact code controls any implementation detail.

Acceptance for this component screen is RMS error ≤ 1 dB, spectral error ≤ 3 dB, and zero nonfinite samples, at fixed gain 1. **Never omit the gain argument when invoking `metrics.py` directly:** its historical default fits gain from note 60. `compare_saved.py` always passes `1`.

Numeric plumbing verification is complete: comparing the reference with itself gives exact zero RMS/spectral error for all three notes; a synthetic half-amplitude copy fails all three notes with 6.020599913279624 dB RMS error while gain stays 1. These checks verify reference selection and gain handling only. They are not a new synthesizer comparison. The small receipts are in `utility_verification`; synthetic PCM was removed.

This is one component screen and covers zero intact factory programs. The historical 180/180 and 72/72 outcomes describe a source-aware prototype, not new code.

## Recording recovery status

The archive `sound-pcm-preservation.tar` is **recovered and independently hash-verified**, 211,988,480 bytes, SHA256 `d5d11628a390f19d99783b1fb805979d371f5971efa01737c00bbc1e359b5403`. Its exact identity is in `reference_recovery.json`. All 60 physical-gain gzip sizes/hashes and decompressed paired sizes/hashes match the historical manifest and per-case reports. The initial inventory had no local PCM; that finding is preserved as history, and it is no longer the current blocker.

The dynamic-filter set consists of 24 files, three notes per file. All 24 uncompressed hashes and metric reports are recovered. Their documented last location is `/tmp/sound-filter-modulation`, which is absent. **No dynamic-filter recording is present in the recovered archive.** A narrow title search found no separate modulation PCM archive. This does not prove that no differently named archive contains them. The 72 active-filter note cases remain unavailable for audio comparison.

The small source/evidence archives were inspected as archive inventories only: `sound-hardware-core-spike-v4.tar.gz` has no PCM members, and `sound-classic-handoff-delta.tar.gz` has no PCM members. The latter records that the failed first modulation run's PCM was overwritten by its final rerun. Do not claim that failed PCM has been recovered.

## Host replay fallback, reference owner only

The historical dry executable is absent. It is no longer needed to compare the recovered 180 physical cases. Its observed interfaces, retained for the still-missing active-filter captures, are:

```text
physical-gain/dry-reference TABLES OUT.f32 CUTOFF RESONANCE VELOCITY
filter-modulation/dry-reference TABLES OUT.f32 CUTOFF RESONANCE VELOCITY DEPTH
```

The smallest selected historical invocation is `physical-gain/dry-reference TABLES OUT.f32 2000 .9 100`. It records the three-note scene above. Its source harness identity is `51cdbae86dcb75bcb57720ca4471b3a0807e51253388f55ea64070bd0dad4653`. The public-table input identity is `0d35b5422cb5d3e0e349a66c9e0a50f2980b6287e68f8d94f55640d3f8168bd5`, 852,176 bytes. No public waveform assets are included in this packet.

The complete historical build command arrays are in `reference_owner_historical_build_commands.json`; their recorded dependency hashes and observed local matches are in the two `reference_owner_*dependency*.json` files. They are build metadata, not an implementation source handoff. The historical harness linked a prior source-aware candidate beside the original. An independent implementation must not use that code. A fresh reference-only wrapper, if needed, belongs to the reference owner and returns only PCM and numeric metadata to the implementer.

The historical build is not immediately replayable unchanged: its two generated dry/candidate object files are absent, one retained scheduler object has a different hash, and three required header identities could not be matched in the supplied reconstructed trees. Exact matches exist for **34 of 35** retained reference dependency objects. The inventory records every required path and hash; do not substitute a nonmatching object silently. Existing host dependency ELF comments report GCC `(Debian 14.2.0-19) 14.2.0`; the exact compiler executable used for the missing dry object was not separately recorded.

The historical physical runner used a **15-second per-file timeout** and completed the 60-file matrix in 35.373608112335205 seconds. No per-file time was recorded. One selected capture would process 793,920 total reference frames, of which 529,344 are silent warm-up and 264,576 are retained. A mono-only reference capture would write 1,058,304 bytes. Those figures are workload bounds, not a new timing measurement.

The present `factory-test` executable is not a substitute: inspected harness glue fixes a 48 kHz, 96,000-frame preset scene with changing notes and common FX. It cannot produce the specified dry capture through its existing interface.

## Historical failures and filter isolation

Historical initial dry failure and the first modulation range failure are preserved as separate JSON files. The latter accepted 54/72 cases, with maximum RMS 5.383274629847556 dB and maximum spectral error 8.895468323672016 dB; the failing group used depth 1.2 through an unsuitable normalized setter. Later historical pass summaries remain separate. No thresholds or outcomes were rewritten.

No filter-only impulse, sine, or level-sweep recordings or matching replay harness were located in the provided owner evidence/reconstructed-reference inventories. The only impulse/tonal report matches concerned an unrelated Hall experiment. Therefore these dry-voice cases cannot establish filter transfer independently of oscillator phase. A filter-only numeric recording remains an additional missing input.

No compiler, original renderer, native link, emulator, full suite, timing run, RDC operation, or GitHub operation was executed by this recovery task. Original DSP algorithms and historical candidate implementation contents were not opened or copied. The final archive work consists only of read-only hash verification and original-column numeric extraction authorized by root.
