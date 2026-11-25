# Synth One reverb bench → target fit

Apply only as an isolated, never-install trial against the exact
ca1dd548 paired guarded-fit source and wrapper. This is not in the
runtime candidate. Same delay arena and same reverb arena, 8-line
packed network for the reverb-on/delay-off path; untouched in other
authored FX setups. In the headless reference bench the Organ full-wet
last-second levels improved −4.697 to −0.049 dB and spectral distances
9.800/8.525 to 2.566/2.749 dB; remaining five wet captures unchanged.
The Organ isolated waveform/null test still fails outright parity.

Native XIP with first version: 1,112/1,100 B over. Reusing the existing
IMA constants and combined stereo image filter leaves 956/948 B over.
A small outline leaves 892/884 B over. These are actual pair links,
not inferred flash costs. No executable, device, release or timing test.
The source remains explicitly opt-in under the lab companion profile.

Deleting the dual topology in a separate all-room trial produces a
guarded pair with 128/136 B app space and 412 B above the base RAM pool
reserve, but the six matched bench programs reject that variant:
Sahara full-wet tail +3.84 dB and Atmostrings −3.78 dB, both worse than
the existing compact reference; the source is not approved for use.
The working recommendation is to keep the matched two-path experiment
isolated and find further safe flash capacity or a smaller common
implementation. Never modify the preset to compensate.
