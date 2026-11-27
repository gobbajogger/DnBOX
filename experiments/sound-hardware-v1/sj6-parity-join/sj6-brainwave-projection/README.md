# Brainwave authored-control projection and signed A/B

Test-only driver from the guarded combined host donor. No firmware or preset
asset changed. Use the already saved Brainwave authored/Room-off fixture and
its exact 148,684-frame, 1,164-block, eight-event cue. This does not relabel
a partial projection as the complete authored patch.

From the workspace root:

    python sj6-parity-suite/prep.py
    python sj6-brainwave-projection/prep.py
    bash sj6-brainwave-projection/build.sh firmware-fit-next
    for a in 0 1; do
      for p in 0 1 2 3; do
        sj6-brainwave-projection/render "$a" "$p" \
          >"sj6-brainwave-projection/private/$a-$p.f32" \
          2>"sj6-brainwave-projection/private/$a-$p.log" || exit 1
      done
    done
    python sj6-brainwave-projection/score.py >sj6-brainwave-projection/result.json

Mode 0 is Init and the room-only pilot. Mode 1 applies selected authored
Brainwave values through existing runtime/DSP receivers, including exact
master, cutoff, oscillators/PWM, range, AMP2, vibrato and first AMP attack;
16 raw word readbacks agree. Additional ADSR, Room and velocity detents
are quantized, not asserted to match exact playback. Mode 2 also applies
the authored Unison controls through the incumbent compact receiver.
Mode 3 repeats mode 1 with unity master only as an ablation, not a fix.
This uses the same compact host donor for the four variants, not an AU
installer, native PI32, the separate full-rate reference checkout, or a
product change. No per-preset gain fitting or time alignment.

Room-off remains ~30.47 dB too quiet in mode 1, ~37.11–37.79 dB too quiet
when incumbent Unison is active. Isolating master for mode 1 yields
−17.91 dB uniformly, but the unity ablation is still 12.57 dB too quiet.
This confirms present authored master is not a sufficient correction.
Room-off is not all FX-off. These numbers apply only to this partial
candidate; missing inputs, output gain law differences elsewhere,
and app-vs-host receiver differences are not promoted to root causes.
Next comparison should isolate the same-input oscillator summer and
Unison dry/wet sum before Room/master, not fit a terminal gain.
