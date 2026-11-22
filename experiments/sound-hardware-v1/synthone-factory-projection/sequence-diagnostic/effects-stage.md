# Original effects after sequenced firmware voices

This host diagnostic applies the pinned original effects to the firmware's captured mono post-filter voice sum for the nine preceding sequence cases. It compares both direct fixture notes and original emitted events. Each source render reproduced the retained original PCM, original dry PCM, and emitted events bit for bit. Each firmware tap retained the existing 40-control readbacks and untouched final PCM in both schedules. The original effect omissions affect comparison-only inputs. They leave all other stages active. No generated original source or PCM is included here.

On #139, the firmware voice estimates closely match the original dry voices under matched events. The firmware final output then fails level by +2.703/+2.721 dB. The exact original graph on those same firmware voices yields −0.908/−0.843 dB level and 2.345/2.337 dB spectral error, a stereo pass. Bypassing the reverb crossfade changes this to +2.738/+2.756 dB and 4.307/4.307 dB. Keeping its authored dry attenuation and muting the wet return still passes at −0.954/−0.935 dB and 2.399/2.392 dB. Removing delay leaves the complete-effects bytes unchanged. Applying the original effects on the direct notes fails #139 at −2.576/−2.530 dB, which shows the interaction with the sequence.

On matched events, the original effects also recover #664 and guards #10/#221, including when the reverb wet return is muted. Guards #213/#481 still fail, as does #482 with the complete effects. The result isolates a contributor to the residual and does not qualify a production route.

Reproduce from the repository root after the retained sequence and dry-tap comparisons. The retained-input directory must contain `factory`, `nine`, `dry` and `panel` outputs, matching the restored verified fixture.

```sh
python experiments/sound-hardware-v1/synthone-factory-projection/sequence-diagnostic/effects_stage.py . RETAINED_INPUTS FRESH_OUTPUT
```
