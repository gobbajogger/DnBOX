# Synth One Filter Envelope LFO ID56: STOP, no firmware patch

Tested on exact current composed FW manifest `02069b8e9ebdc013e8fdbe7d79e8508245aba57d1008b21182d8d20f81bd47b6`. No product, record, UI, codec, Git, native or device writes.

Pinned original `s1-audit-data/original-inputs/closure/master/AudioKitSynthOne/DSP/{S1Parameter.h,Kernel/S1DSPKernel.hpp}` defines ID56 OFF/LFO1/LFO2/BOTH and matching 64..22050-Hz cutoff bound. Original `closure/experiment/probe/portable_voice.cpp:316–342` advances filter ADSR, scales FILTER ADSR MIX by descending LFO1/LFO2/arithmetic mean or OFF unity, computes `clamp(cut - cut * mix * (1 - fenv))`, and sends the new cutoff to all three filters for each rendered voice sample.

Compact `firmware/src/dsp/s1c/s1c.c:396–433,452–459` advances AMP and filter envelopes each sample but only consumes new envelope and LFO in cutoff when the voice countdown ticks. Its nominally similar formula is actually eight-sample sampled/held in the shipping configuration (`s1c_config.h:25`, not the unshipped KR1 branch). This temporal mismatch affects positive presets during note onset and release, not just bulk oscillator/EQ differences. No assertion that full original waveform matches.

`cutoff-cadence-probe.c` uses the genuine compiled compact voice/latch and settled authenticated program words. To isolate the discrepancy, identical current device values feed independent original scalar algebra, so any differences from original waveform, upstream envelope math or coefficient approximations cannot explain diagnostic mismatches. No production instrumentation or code modifications. Genuine low-pass factory ordinal 81 produced:
- Each of LFO1, LFO2, BOTH: actual compact coefficient equals scalar-current cutoff at every 512/512 control ticks. The held coefficient instead corresponds to previous cutoff at every 3584/3584 off-ticks; of these 1064 were in real filter ATTACK and 1792 RELEASE. Total max 40.906 Hz. First LFO1 attack off-tick held 3230.954590 vs current 3236.856934 Hz.
- Subtracting an OFF oracle at the same sample and same state leaves the isolated ID56 contribution stale on 3584/3584 off-ticks. Max isolated drift was 11.505/11.805/11.655 Hz for LFO1/LFO2/BOTH respectively.
- Real zero FILTER ADSR MIX ordinal 542 gives 0/4096 route-dependent differences in all three modes. Generic base cutoff smoothing can still advance between device ticks; it is not an ID56 signal.

A third five-state radix digit would fit, without growing the codec, and FILTER MODE K4 is available. Neither was claimed. To revisit, demonstrate exact original-equivalent note-phase application between control ticks or explicitly agree to the measured approximation; do not blindly switch all voice coefficient work to per-sample at the current hard timing gate. The previously verified FM, AMP Decay, Legato, and all pages were not edited.

Reproduce with `./replay.sh EXACT_COMPOSED_AMP_DECAY_SOURCE AUTHENTICATED_S1_PACK`. `probe.log` is the archived observed quantitative output; the tool exits 0 when the mismatch and control case are successfully detected, **not** when the candidate is equivalent.
