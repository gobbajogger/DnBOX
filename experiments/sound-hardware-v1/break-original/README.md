# BREAK straight playback against BB Gen

The shipping DnBOX BREAK renderer and mestela's original BB Gen plugin played the eight slices of five supplied loops at 44.1 kHz and unity playback rate. With identical decoded source audio, all 694,381 compared PCM16 words matched. The original stereo channels were identical for each mono loop.

The separate WAV comparison includes the loss from DnBOX's 4-bit IMA encoding. Both original WAVs available in the checkout met the sound lane's 1 dB level and 3 dB spectral limits in every slice.

| Supplied loop | Matching words from shared source PCM | Largest slice boundary difference | Worst absolute level difference from source WAV | Worst spectral difference from source WAV |
| --- | ---: | ---: | ---: | ---: |
| CRAMEN | 68,834 | 4 frames | Original WAV unavailable | Original WAV unavailable |
| TASTY | 104,081 | 2 frames | Original WAV unavailable | Original WAV unavailable |
| HOTPANT | 83,970 | 4 frames | Original WAV unavailable | Original WAV unavailable |
| 909FL | 218,748 | 3 frames | 0.03067 dB | 1.10763 dB |
| 909GR | 218,748 | 3 frames | 0.03058 dB | 0.83723 dB |

CRAMEN, TASTY and HOTPANT belong to the designated three-loop pack. Their decoded WAV inputs were reconstructed using the host IMA decoder and verified against all three published PCM SHA-256 digests. The two 909 loops are separate test assets. Loading matched sources isolates the DSP comparison from pack installation and sample provenance. The per-slice scores, counts, predetermined displacements and paired PCM hashes are in [results.json](results.json).

The reference host uses the unmodified plugin API, audio renderer, WAV loader and slice selector at upstream commit `c3ea85da9025a3694c167e096cb09d0efb60420c`. It sets the tempo and complexity, starts transport, then plays each slice through MIDI note 36–43 or the first transport trigger. The candidate calls the actual firmware bank loader, transport, slice-selection and rendering functions. Both receive an eight-slice mono loop, zero complexity and no retrigger. An integer-scale conversion expresses the float candidate output in the plugin's PCM16 format.

Each capture begins at the selected slice and stops 128 frames before the next trigger. DnBOX ramps gate and voice gain over 88 increments. The test retains that onset in the PCM capture and scores from frame 89, after both binary32 gains reach unity. DnBOX distributes the remainder from dividing a loop into eight among the slice boundaries. BB Gen leaves that remainder at the end. The test calculates their relative start positions directly from the source length before comparing the overlapping samples. No gain or delay is fitted to the audio. Playback at fractional speeds, random sequencing, reverse, stutter, mixed output and native timing require their own tests.

To reproduce, run from the firmware root on Linux with NumPy and a C compiler. Supply a BB Gen checkout containing the pinned commit and a fresh output directory.

```sh
python3 experiments/sound-hardware-v1/break-original/replay.py /path/to/schwung-breakbeat /tmp/break-original
```
