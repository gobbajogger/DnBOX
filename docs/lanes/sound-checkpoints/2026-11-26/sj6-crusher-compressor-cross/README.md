# Matched original SuperJ6 Crusher and compressor startup states

The stage-local estimates remain different after physically forcing control
transitions, not just checking readback. This is a native Mac SuperJ6 test,
not a firmware gain change and not a full-preset level or sound pass.

Fresh exact Jupiter Sine, OSC2 fader .5, A4 fixed velocity 87, 44.1 kHz.
Cold state uses the earlier capture's normal writes. Forced zero cycles
public #107 through 1→0 and #17 through .02→0 before applying the compressor
target. Forced enabled has #107 on, Amount zero, with the same #17 cycle.
The initial state of the original contains a Booster default 1.3 even
while raw Enable is initialized to zero, and the original compressor
starts active; the forced writes remove these potential ambiguities.

| Native state | Compressor .02 on/off change | Bare input-equivalent peak |
| --- | ---: | ---: |
| Cold, Crusher off | -9.921 dB | .3901 |
| Forced Crusher off | -9.712 dB | .3810 |
| Forced Crusher off repeat | -10.024 dB | .3945 |
| Forced Crusher on, Amount zero | -14.260 dB | .6435 |

Enabling Crusher scales the independently estimated compressor inlet by
about 1.66×, consistent with the earlier 30% mixed response plus the single
Booster. Active native compressor H1 is .019038–.019043, nearly fixed
across these inputs; standalone Apple is .12431 in the saturated region.
The respective phase shifts on activation are +160.50° and +160.503°.
The native-to-bare active output scale .15314 therefore does not depend
on fitting a new free output level. It is close to the prior source
product .404 × .44618064 × .85 = .15322.

At OSC2 fader .5 the earlier independently replayed Crusher probe estimates
an input-equivalent peak .1395–.1573. Thus the conditional compressor
equivalent .381–.3945 is still materially higher, despite forced transitions.
The wet calibration's 1.66× scaling cross-predicts the independent Crusher
insertion. No specific source node accounting for the intervening scale
has yet been verified; this evidence must not be used to blindly boost
firmware or change presets rather than the relevant route.

`python3 score.py native ../sj6-auv3-compressor-inlet` recomputes the
measurements from the original signed stereo renders and the prior Apple
detector curve.
