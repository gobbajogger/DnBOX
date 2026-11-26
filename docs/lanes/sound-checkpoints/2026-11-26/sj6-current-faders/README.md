# Current Mac SuperJ6 actual instrument faders, shared trim and pan senders

Run from workspace root with the authorized newest executable pointer in
/tmp/dnbox-newest-sj6-oracle-location and Unicorn + original bench on
PYTHONPATH. The code verifies this Mach-O and the direct-AUv3 archive.
Run probe.py, stereo_trim.py, and pan_split.py; their outputs are hash-pinned.

Actual released Mac normalized control closures invoke the real copied guest
AudioKit property implementations. OSC1 (1), OSC2 (3), and Sub (202) send
Float32(.808 * Double(raw)); Noise (201) sends Float32(.707 * raw^2.5).
Actual AUv3 readback 1/1/.7269732952/0 therefore maps exactly to
.8080000281/.8080000281/.5873944163/0 in the actual AKSamplerDSP address0.
The observation-only object boundary is bridged to the actual guest DSP call.
This corroborates earlier physical values from original older app, not a
candidate gain selected to fit. A separate compact 7-bit route rounds the
Sub control to approximately 92/127 at this input; the possible adjacent
steps change ~0.09 dB or less, not 12 dB. Do not round a source oracle.

258 actual current resonance callbacks across both Normal and OTA reach
the Swift mixer and Apple output volume observation. Exact shared gain is
Float32(1 - k*raw^1.5), k=.49881276637272776 Normal or .4 OTA. Brainwave
Normal is .8670601845 (about -1.24 dB vs unity) and OTA .8933950663.
Old/max-law differences cannot explain the whole loss. Current dry native
fan-in measured by integration owner is additive independently.

The current five-output stereo spread caller and AKMixer pan setters reach
the respective AVAudioMixer pan properties with raw multipliers
[1, -.95, .5, -.59, .33]. Zero sends exact channel centers; this is not a
claim about stressed active Apple pan law, only captured real parameter sends.
The chapter's direct cue also disabled Unison and had stereo spread zero.
No audio, app body, speculative candidate gain, or production modification.
