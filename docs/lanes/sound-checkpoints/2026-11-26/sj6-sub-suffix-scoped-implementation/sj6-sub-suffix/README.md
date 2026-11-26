# Actual scoped compact dry suffix diagnostic

Stock compact raw cutoff #28=0 produces no HP; inactive compact Crusher
returns without a DC effect; compact per-instrument render ends directly
after master, before any separate global bus. This scoped host fixture has
the previously opt-in exact-zero path disabled. The sole chronological
cascade implements proven HP10 and exact zero-distortion DC in the stock
no-op block, and the separate terminal .999 DC after unchanged master.
No duplicate filter, no alternate level scalar or copied sample data.

Strict readback admission requires #28, #107/108/109 and all other competing
routing controls to be present and zero. Brainwave source-isolation's
actual forced zero scope passes. Cross-program dry input only Raw passes:
Brainwave leaves #108 nonzero and Unison enabled, Solidify leaves #108 nonzero,
and Raw authored keeps Chorus I (in a different compact position from actual
source). The five refused original programs compare byte-for-byte unchanged
to prior Range outputs. Not production mode or complete graph.

Synthetic independent signed transfer error −104.52/−111.63 dBr at nominal
32.70/97.999 Hz relative to chronological F64 physics and partition invariant.
Brainwave first held H3 phase −179.95→−23.60°, native −18.71°; later
−179.55→+101.54°, native +148.06°. Separately absolute signed held and pending
improve .239 and .160 dB; first held level deficit grows −26.31→−29.25 dB,
and release signed residual worsens .044. Raw dry signed full worsens
−.15560→−.14837 dBr, level deficit −29.764→−30.428. Correct phase transfer
does not qualify carrier, absolute stage levels, or the full original synth.

Use route.py to create a copied donor runtime, given exact retained pin and
range-corrected dnbx_core from previous proof. Run existing build.sh with
SJ6_RUNTIME_OVERRIDE (copied route), SJ6_DNBX_CORE_OVERRIDE (range code) and
SJ6_RENDER_SOURCE (compact.c). Reuse prior signed receipts/cues, and build
synthetic.c independently with -ffp-contract=off. Scripts report compiled
comparisons, no private waveform/control values in the sealed archive.
