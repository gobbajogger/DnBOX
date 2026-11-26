# SuperJ6 Main OTA: transposed fixed all-pass state preflight

An independently implemented one-state first-order all-pass is equivalent
over exact arithmetic with fixed coefficients. It replaces each direct-form
two-section history with two transposed states. The exact archived packed
reference uses 64 B; this uses 52 B, theoretically 72 B less for six owners,
before stride, storage ownership and lifecycle accounting. The variant
changes Float32 association, doubles fused operations in the first form,
and is not approved as bit exact or realtime-safe.

Run `bash run.sh` with NumPy. Across 128 signed synthetic input/profile
combinations per arm (1,048,576 samples each), the worst whole-output
residual was −127.98 dBr fused and −124.59 dBr with the two history
updates using separate multiply/subtract. Synthetic all-pass coefficients
are not authentic captures. Exact original all-pass constants are resolved
privately by the existing signed leaf bench; the source file has no
constants or production modification. Measure authentic cues, reset and
holdouts, and native code and timing before considering adoption.
