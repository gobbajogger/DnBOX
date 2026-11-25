# Original Synth One SEQ and SuperJ6 OTA wheel control corrections

Checkpoint patches only (not an integration merge or release). After the earlier Mod Wheel routing patch, apply in the listed order: master-table (compacts cold control-row validation), s1-delay-resonance (upstream edit detent is preserved, but upstream delay input fixes audible resonance to zero), s1-separate-seq (separate Synth One arpeggiation direction, SEQ selection and master ARP, with legacy import), sj6-ota-wheel (destination 5 uses the original OTA transfer when OTA is selected).

All discoverable corrections in this checkpoint were wired before the paired original-flag native object compile. Exact compile-only receipt, before fit or playback was started on this source:
```
75991720c9b736c723fe8d56ac85fcef957afe6e42f24cee3247a7738b30d4fe
compiled-controls elite
compiled-controls pro
```
Do not substitute the prior strict results for this image. Do not publish the guarded companion layout to real devices. Rejected experimental scheduler/reverb candidates are not part of these source changes.
