# SuperJ6 Rage standalone Apple compressor diagnostic

This measures the Apple Dynamics Processor itself with known synthetic input, not the actual post-Delay input or loaded original app output. Source trace: [original app signal path](../../../docs/sound/superj6-original-app-signal-path.md).

Run `python3 stimulus.py cue48` to generate 109,440 frames of 48 kHz stereo planar Float32LE. Input SHA-256: `c8b974621dbe41e0dd160f49d79937bb4533dc3bc589b7f08e86cfe6784dfa9a`. The manifest names staircase, left/right active transients over quiet opposite-channel tones, releases, and silence. It contains exact 128-frame and irregular 17/31/63/128/251-frame schedules. `--rate 44100` generates the independent 44.1 kHz input with the same time layout. Use the actual timestamps, not cuts inferred from final app PCM.

The isolated `aufx/dcmp/appl` active panel profile for normalized #17 = 0.0625 is threshold −18.125 dB, makeup +1.4375 dB; constructor shape uses headroom 5 dB, attack .001 s, release .05 s. Query and read back actual physical parameters, component version, stream formats, offline flag, latency and tail. Do not assume the constructor values are the only loaded-app state. Cold renders use independent instances and the same exact input and partitions. Wrapper dry is a direct copy, not an unverified Audio Unit bypass or a compressor with different settings.

`python3 score.py cue48 dry.f32le active.f32le active-repeat.f32le` requires matching byte-exact dry samples and repeated active outputs. It reports chronology without inferred delay or amplitude alignment. The scorer was exercised against identity, a known −6.0206 dB gain and mismatched dry/repeat negatives.

## Native acquisition status

The Swift acquisition bench compiled on the authorized Mac (macOS 27.2 build 26B5112a). In that execution session, wildcard AudioComponent discovery returned no components, including the Apple effects controls `dcmp`, `lmtr`, and `hpas`. Independent `auval -a` returned no registered component rows. The bench stops before audio in this context and no native PCM was captured. This is a limitation of the observed execution session, not a claim that the app has no compressor. The bench source and preflight logs are preserved separately (source capsule SHA-256 `c796712d20324e8e9153a76829c35b1bbb4d1d38dea855f05617396845ff7c71`). No app, project, device or settings changes.
