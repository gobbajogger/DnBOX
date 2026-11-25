# Independent eight-line room with live Delay (host)

Unlike the previous witness, this version packs eight histories into 1,724
bytes plus state, inside the 2,100-byte reverb allocation. Delay keeps its
own 2,152-byte history. The alternate room is chosen once when a reverb-on,
delay-off program is adopted; it stays selected when Delay toggles. Native
builds are deliberately barred pending timing, broader preset coverage,
and transition fidelity.

Organ: -0.496/+0.300 dB overall stereo, -0.184 dB body, +0.200 dB full
release, -0.049 dB last second, 2.568/2.748 dB spectral diagnostic. Raw
waveforms do not null. Other five static wet renders are bit-identical.
Result is 2/6 on the measured level/spectral criteria.

The host toggle fixture runs the actual DSP Delay control. Six zero-mix
toggles are bit-transparent against a second instance, guarded arenas
stay intact, and audible Delay changes 74,400 stereo frames. Selecting
the Pad restores ordinary ownership. This proves live host coexistence,
not click-free transitions or original software's automation behavior.

Apply independent-host.patch against the recovered ab-latest-headless
preimage. In the isolated host lab place the patched files in dsp/s1c with
its unchanged headers and compile with build.sh render. For the toggle
test, set S1_ROOM_STIMULUS to the absolute path of toggle.c and invoke
build.sh toggle, then pass the private original fdata.img to toggle.
Rerender 1, 62, 113, 128, 278, 435 into out/<id>.f32le (mode wet);
score.py uses the previously saved private calibration and reference data.
No recordings or binary inputs are included.
