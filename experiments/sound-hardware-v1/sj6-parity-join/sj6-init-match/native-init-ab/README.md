# Native Mac Init: dry velocity replay

This is a terminal A/B and input-path control, not a native keyboard build. It compares the native Mac AU's real Your Init Preset against compact's existing Init; only MIDI, clock and fixed/variable velocity mode are known to align. The compact Init parameter set is not the native Init's full parameter set.

The two source archives in Library are dnbox-native-j6-init-gain.tar.gz and dnbox-native-j6-init-velocity.tar.gz. Extract to one directory; do not use the similarly named Raw J106 pre-roll archive. The anchor is gain-master-0.5: its 170 readbacks are unchanged and it equals both velocity-on captures byte for byte. The native dry reference is 44.1 kHz, MIDI note 48, on 0 and off 22050, exact 132300-frame render partitions in cue.h. Use the detached compact guarded source and flags identified in host-source.sha256 and host.flags, the same donor as normal-stage:

    bash build.sh /path/to/firmware-fit-next
    for mode in 0 3 6; do
      for v in 32 112; do
        ./render "$v" "$mode" >"compact-$mode-$v.f32"
      done
    done
    python score.py /path/to/extracted-reference .

Configured fixed medium (3) emits identical renders for both input velocities as the original does. Configured variable medium (0) matches the original 32-to-112 ratio within 0.00001 dB. Using the unconfigured event path (6) incorrectly compared variable velocity at 32 to a fixed-velocity reference. The manual firmware input wrapper already calls the configured handler; no production velocity change was made. At fixed velocity, the compact Init is ~13.07 dB below the original in the held window. That remainder is real for these two Init renders but is not a proven mixer or master gain error. Native defaults and in-voice signal taps need to be aligned before attributing it. No master gain, sample alignment, normalization, waveform shape or DSP was changed.
