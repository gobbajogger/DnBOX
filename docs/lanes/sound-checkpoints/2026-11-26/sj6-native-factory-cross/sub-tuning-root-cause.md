# Original Raw Sub note-specific tuning

Root cause: the old detached replay supplied an empty per-note tuning map. The actual original compiled Sub path consults a separate descriptor indexed from the physical ranged note, and converts its cents field with sign inversion. It is independent from the visible 440 Hz tuning control, and it does not alter the Main faders. Positive/fallback original receiver tests distinguish this from a guessed pitch change. No original calibration table, audio or binary is part of this checkpoint.

An independent local interpolation of the pinned genuine source assets reduced the worst three relative held H1 slopes from about 0.0209 Hz to 0.00119 Hz when driven by the actual mapping. Reversing the mapping's sign raised it to about 0.0425 Hz. This is an independent numerical check, not a compact-sub or original sampler waveform pass.

On the same compiled original reference engine, same chronological Raw cue and same existing conditional branch normalization, restoring only the authentic Sub tuning lookup gives:

| Signed residual | Previous | Correct Sub note map |
|---|---:|---:|
| Dry whole | −23.595 dBr | −62.482 dBr |
| Chorus I whole | −23.053 dBr | −61.146 dBr |
| First release owner, dry / chorus | −24.621 / −18.209 | −53.679 / −53.533 |
| Final release owner, dry / chorus | −17.320 / −18.054 | −69.735 / −68.252 |
| After final owner, dry / chorus | −12.481 / −13.265 | −42.531 / −50.176 |

Attack first64 remains close but gets slightly worse. Original Main samples and gain are unchanged. This is a cause-verified Raw reference, not firmware parity; recorded Sub waves are still forbidden in the shipping engine. A separate proposed source-only input for a validated independently supplied behavioral profile keeps the model's recorded donor assets out of Git. There is no production gain calibration, fit, release or device operation.
