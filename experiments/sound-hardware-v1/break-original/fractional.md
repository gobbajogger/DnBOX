# BB Gen programmed rolls at fractional tempo

BB Gen v2 reads the sample at the integer part of its playhead during a roll. The production baseline interpolated between adjacent samples. The per-voice change selects the original read behavior when a programmed forward roll starts and keeps that decision through its outgoing crossfade.

| Matched render | Baseline | Corrected | Total |
| --- | ---: | ---: | ---: |
| Programmed 2×, 3×, 4× and 8× rolls | 52 | 84 | 84 |
| Dry fractional-speed controls | 14 | 14 | 21 |

Three playback speeds (0.75×, 1.25× and 1.5×) run across five authenticated decoded loops plus two untouched original WAVs. The harness checks the original status and final repeat count and confirms that both sides differ from the dry control when a roll is enabled. All 105 original PCM hashes reproduced independently across the two runs, and all 21 dry firmware hashes remained identical. The limits are 1 dB absolute level and 3 dB spectral RMS. The corrected rolls reached at most 0.167 dB and 2.504 dB. [Per-render levels, spectra and PCM digests](fractional-rolls.json.gz)

At unity speed, 28/28 programmed pairs and all 16 original-WAV straight slices passed. All 694,381 post-fade decoded PCM16 samples matched across 40 slices. The adaptive BREAK suite passed all 51,481 checks. [Reference stimulus and unity-speed interpretation](README.md)

On the exact per-voice candidate, Chicago/Miami peaked at 11,511 µs on Elite and 11,325 on Pro. Juno/Synth One reached 10,398 and 11,244 µs. All four had zero late transfers and zero xruns. The additional saved dual-LFO crush, phaser and master scene reached 10,893 µs on Elite and 11,640 on Pro. Pro recorded one late transfer and one xrun. All six post-boot stereo lengths and crash-handler checks passed. [Four ordinary captures](fractional-regular-timing.json) · [Additional wet captures](fractional-wet-timing.json)

The production implementation now masks the interpolation fraction per voice. It reproduced all 105 original/candidate pairs from the first per-voice implementation, and passed the six package-level scenes. The four standard E/P peaks were 11,534 / 11,366 µs for Chicago/Miami and 10,444 / 10,998 µs for Juno/Synth One. The additional two-LFO, phaser and master projects reached 11,040 / 11,320 µs. All six recorded zero late transfers, zero xruns and a complete post-boot stereo capture. [Shipping firmware identities and six receipts](shipping-rollmask.json)

To reproduce the comparison on the historical baseline, check out `bf48ce83` with the updated `straight.c` and `tempo.py` from this directory. Run `python3 tempo.py /path/to/schwung-breakbeat /path/to/new-output`. The baseline returns a failing status for the 32 out-of-limit rolls. Apply the candidate to a clean baseline and repeat in a fresh directory. [Per-voice source change](fractional-roll-head.patch)
