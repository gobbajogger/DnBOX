# Small Synth One and SuperJ6 effect parity benches

Keep a separate short case for each effect. These benches are for diagnosing
sound, not certifying firmware fit, live timing, USB, or the release.

For each case keep four stereo captures: original dry, original wet, compact
dry, compact wet. Match actual program settings, input MIDI or input signal,
key release, rate, gain, preroll, state reset and duration. Bypass and wet
must start with the same state and timing; leave the capture long enough to
include the decay. Isolated kernel probes can feed the same kept-dry sample
signal to both engines, but cannot prove the whole preset effects graph.
Give transport warm-up/latency an explicit measured alignment if needed.
Do not estimate a shift, gain-match takes, convert mono to dual mono, or
substitute a Python lookalike for an unavailable original.

## Small starter battery
- **Synth One**: Dry note and chord, Pad delay only, Dark Theater Organ
  reverb only, and those two patches with all original effects enabled.
  Use six original preset pairs for the broader final comparison, not just
  the two problematic cases. The archived preview-gain-calibrated wet
  reference does not stand in for a true independent dry original.
- **SuperJ6**: Dry note or identical input kernel pulse, 500 ms feedback
  delay and a long delay, then the original room and chorus/widen paths
  as verified original renderers become available. Original kernel evidence
  and full instrument A/B are distinct; never call a delay oracle a full AU.

The score separates dry error, complete wet error, and the effect-added
difference `wet - dry`. The last is diagnostic even for nonlinear effects,
not a claim that the engine internally exposes an independent wet bus.
Inspect early/late energy and frequency bands, fit decay and stereo
correlation where signals support it. A deep null is useful for a
phase-deterministic kernel; when an effect is modulated or random and
sources cannot be phase-locked, use repeatable timing and spectral metrics
as evidence, not an invented sample-nulling sign-off. Silence or clipped
captures need a different stimulus. The scorer never rescales levels.

## Run

Requires Python 3 and NumPy. Render locally from the relevant engine
adapter into a private directory. Copy `matrix.example.json` and replace
capture paths, windows, and provenance with the actual render settings.
The example is deliberately not runnable without authorized captures.

```sh
python3 compare.py --self-test
python3 score_matrix.py /path/to/capture-matrix.json /tmp/new-fx-scores
```

Outputs JSON scores and input SHA-256, not rendered audio. Set explicit
thresholds only when agreed for that benchmark; otherwise treat the output
as diagnostic. Keep preset bytes, original binaries, and render audio out
of Git. Candidate comparison comes after the full controls are wired and
the selected exact firmware fits, not as a reason to defer wiring.

## Included backends and validation

- `s1-fx-parity/`: real upstream AudioKit Synth One + Soundpipe renderer
  and separate DnBOX stereo renderer. Its old-stock validation generated
  all four capture arms and ran six cases. See `s1-stock-diagnostic.json`;
  the dry comparison is not assumed exact, and Organ delay is a negative
  control because the original program leaves it off.
- `sj6-fx-bench/`: isolated C bridge into actual compact effects and
  the prior ARM64-verified stereo delay reference. The older restored
  compact source was used to smoke-test the bridge on the 250 ms case;
  that result is not the current full engine. The J60 and raw RevSC
  adapters need Unicorn and the private AU. Original complete Room and
  Widen graph outputs are not yet comparable, so they are not marked pass.

The self-tests exercise the harness only. Neither bench ran against the
new combined controls candidate or certifies a parity result for it.
