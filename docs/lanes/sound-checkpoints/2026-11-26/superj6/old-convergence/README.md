# Original old SuperJ6: oscillator-convergence trim

Run from workspace root: set PYTHONPATH to the authorized old bench and Unicorn packages, set SJ6_BIN to the original pinned binary, then run python sj6-old-convergence/probe.py.

The harness executes the original resonance callback and real sampler and Swift mixer property implementations without needing the older full World wrapper. The final AVAudioMixer setOutputVolume call is captured, not substituted with a claimed original PCM rendering. The mocked two-element instrument sampler wiring contains an oscillator and Sub stand-in; the real DSP receives both messages.

For all 128 control-grid words plus Brainwave's authored Float32 resonance in each mode, the observed endpoint is exactly Float32(1 - k * pow(Double(raw),1.5)), with Normal k=0.49881276637272776 and OTA k=0.4.

At Brainwave's 0.4141378700733185, Normal fan-in trim is 0.8670601844787598 and OTA is 0.8933950662612915. Both sampler resonance values are 7.86447811126709. Thus mode changes this trim by only +0.260 dB at the authored setting, and even the full range never imposes 12 dB reduction. Strength/ADSR/tracking writes are not mixer-volume writes. Genuine dependent packets are recorded, including OTA cutoff's additional sustain update.

Absence or one duplicate application of this old authenticated trim cannot explain the newer AU gap. No fitted gain, preset edit, current-AU routing claim, native build, or production change.
