# DnBOX factory input and stereo diagnostic

At source commit `42ae56e1d83127fd9855fb8e86f8907af216b63a`, the optional SuperJ6 factory transaction admits 12 original parameter IDs. The 14 S4 programs have 1,926 authored controls total, with 114–167 per program. At least 1,758 lack a route through this transaction. It carries no original program UID, tuning metadata, wheel destination, keyboard octave or sequence tempo. The ordinary firmware build omits the guarded transaction.

The host probe selects stock DnBOX Juno preset 0 and feeds every submitted word to the production factory row reader and `engine_superj6_factory_load`. A one-frame silent engine render commits the transaction. The probe then reads the selected DnBOX bank and preset, sequencer clock, raw register bank, physical oscillator and AMP parameters, instrument master and chorus from the live owner. It sends the eight S4 MIDI events through `original_runtime_key` and captures `original_runtime_render` after the DnBOX chorus and master. It preserves the 128-frame block grid and splits at exact event frames.

A three-vector regression changed all 208 ignored identifiers in the 220-entry catalogue, including the original chorus, delay and compressor. The measured physical state, stored words and 132,300 stereo frames matched the baseline byte for byte. There were 82,794 frames with unequal channels. Changing only the admitted master in the second vector to zero then silenced both channels. The active base preset, clock and chorus are DnBOX's. In the hash-pinned factory metrics, eight selected programs enable the original delay, whose IDs 24–27 have no route here.

| Host result | SHA-256 of interleaved stereo binary32 |
| --- | --- |
| Synthetic baseline and changed ignored controls | `0e51ad4d45aa1c7ae8a3482ab1526819f5565776d6f827bb0fc16a08ed8c3b61` |
| Admitted master set to zero | `bacc5dea3c2a5becb4d28596f4f3590ddc4eeeec1712b1939465ba4f23630209` |

Run from the repository root on Linux with the repository host toolchain. Supply a fresh output directory.

```sh
case_dir=experiments/sound-hardware-v1/sj6-factory/candidate
sh "$case_dir/run.sh" /tmp/sj6-candidate --self-test
```

For private diagnostics on actual rows, use `--inputs /absolute/path/to/factory-matrix`, `--release-ipa /absolute/path/to/release.ipa`, or `--release-app /absolute/path/to/SuperJ6.app`. The release-app option also accepts an embedded AU `.appex` with its containing app and banks available. The driver verifies the source-file and complete-record digests before staging each record. It writes the partial host waveform, observed DnBOX state and digest locally with `qualified: false`. The synthetic runs read 12 stored registers after overwriting their input row. The physical-state and register reports for the baseline and ignored-control vectors had identical JSON SHA-256 `45d4cb3ad3ec831ef8adc91274b99028722dc0697dd97b48204f9aa70dadf50d`, including the measured PCM digest. The register report distinguishes stored source words from independently measured physical parameters. This experiment supplies zero S4 pairs. A qualifying candidate still needs production-image capture with the complete preset, authored effects and clock, and active program and control readbacks.
