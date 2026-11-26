# Current SuperJ6 XY selectors and physical destination dispatch
The original current registered public parameters #203 and #204 select
destinations. Preset callback paths are .274c6c→.274cf0→.0f160c and
.274d20→.274da4→.0f1684. Executed setter bodies store different one-byte
enums at ivar globals .82f390 and .82f398; no DSP write or Objective-C
message. Finite Float32 truncates toward zero; tested valid enum 0–9
persists, invalid integers fall back to Pad1=4 and Pad2=2. The current
consumer copies these bytes to the two-pad UI object at .2b1dd0→.2b1de0
and .2b1e08→.2b1e18. The other consumer reads the pair resolver in
.2b2654/.2b2784. The verified packed low/high address bytes are:

| Destination enum | Low address | High address |
|---:|---|---|
| 0 | 144 Seq Gate | 66 Arp Interval |
| 1 | 26 Delay Feedback | 27 Delay Mix |
| 2 | 7 Physical Cutoff | 8 Resonance |
| 3 | 88 LFO1 Rate | 89 LFO1 Amount |
| 4 | 96 LFO2 Rate | 97 LFO2 Amount |
| 5 | 100 LFO3 Rate | 101 LFO3 Amount |
| 6 | 15 Reverb Mix | 13 Reverb Size |
| 7 | 72 L1 Semitone | 73 L2 Semitone |
| 8 | 104 Tremolo Rate | 103 Tremolo Depth |
| 9 | 61 Unison Stereo | 60 Unison Detune |

Filter reaches 7/8, not legacy 129/130. Packed byte ordering is not a
claim about coordinate-to-value conversion. Do not invent a continuous
parameter, infer automatic modulation on preset load, or assert actual
gesture-to-DSP timing from this witness. No firmware changed.

From the intact authorized source worktree, use the existing verified
current bench setup and run `python3 verify.py`. The script combines the
separately executed setter/resolver checks with assertions; it expects
the same pinned ARM64 donor as sj6-current-faders. No donor or audio
is bundled.
