# Compact J60 bridge's reachable arithmetic clock

This is a source-only arithmetic check, **not** a native or typed-audio
counter. It extracts the continuous, initially reset bridge recurrence
from the [archived compact implementation](../../../handoffs/2026-11-24-source/composed-controls/firmware/src/dsp/juno6_j60_native.inc).
It assumes no mode changes, bypass or intervening resets. Compare its
tick count to an exact rational integrator that advances after each
published host step. The original captured resampler uses its own
startup indexing; equal nominal rates do not imply equal startup counts
or paired signed sample positions.

At 44.1 kHz the compact counter advances 323,665 times over 148,684 host
steps and 28,800,000 over five minutes, equal to the exact rational
count endpoint at both. Its first per-host-step count discrepancy occurs
at 91,270 steps; after one hour it is 11 internal samples ahead. The
48 kHz case has exact 2:1 ticks at every tested step. This confirms that
the float recurrence is designed for nominal **96 kHz**, not 44.1 or
88.2 kHz. It does not show which runtime path is actually reached,
whether the old and new bridges enter the same samples, or any audio
quality, APP use, or dropout timing. An independent composed-route
counter and signed wet comparison remain needed.

    cc -std=c99 -O2 -Wall -Wextra -Werror -ffp-contract=off bound.c -o j60-bound
    ./j60-bound
