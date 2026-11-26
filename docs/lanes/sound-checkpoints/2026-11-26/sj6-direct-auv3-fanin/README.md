# Direct SuperJ6 AUv3: is the dry oscillator mix additive?

The original Mac SuperJ6 AUv3 (0.999.8 build 9) ran directly inside the
AppKit/AVAudioEngine host. No DAW, standby app, or USB audio device was used.
The same warm Brainwave cue was rendered eight times: all three oscillator
paths active, each path isolated, and a repeat of each. The capture pins the
binary, preset identity, frame-exact MIDI and live parameters; Crusher and
the other named downstream controls are off. This is not a full-FX test.

The unweighted stereo sum of the three isolated renders matched the all-on
render to −65.39 dBr residual on the held note, −56.95 during the pending
chord transition and −72.11 at commit. Level differences are within
0.0011 dB over all measured windows. The dry mix is effectively additive
for this cue. A common gain before or after the mixer is **not** ruled out;
overall compact engine fidelity remains open.

Run `python3 score_fanin.py .` with NumPy installed from this directory to
check the original sample hashes, MIDI schedule, and pinned controls and
recompute the measurements. No install is required to score existing PCM.
The eight renders are duplicated in ChatGPT Library; see the Library index.
