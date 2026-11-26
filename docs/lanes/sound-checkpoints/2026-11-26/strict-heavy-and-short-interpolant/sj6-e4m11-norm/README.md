# Exact unscaled short E4M11 interpolant
Single diagnostic change after the no-copy carrier: remove the exponent-only
times 32768 per operand and divide 32768 after the add. Keep identical linear
weights, taps, clocks and disallow FMA by same native compile. Installer lane's
independent finite-word test covered 11,179,648 signed cases. No tuning changes
or additional lab variants. Exact two-owner native short fixture, unchanged
no-copy source owner and signed pins, no full heavy timing or release claims.

Relative to prior no-copy, both boards +32 bytes APP free (2860/2872,
112 bytes still more than Q15), +564 pool unchanged with identical ring/owner
sizes. Reused original stock/copy and no-copy baseline sealed measurements.
Four native two-owner 256-frame timings improve 22.93–22.94 us to 723.79–725.95
us, all output signatures exactly equal original E4M11, all inputs verified.
Still about +185–191 us vs stock Q15 on the isolated short load; not release fit.
Zero emulated NOR writes, no physical device or deployment.
