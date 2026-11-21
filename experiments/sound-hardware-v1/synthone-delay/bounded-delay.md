# Four bounded delay histories per Synth One owner

[Download the source, replay commands and measured results](bounded-delay-proof.tar.gz). Run the commands below from the extracted packet.

A 2,748-byte per-owner implementation fits the 2,764 bytes conditionally left by dry INIT in its selected stock backend union. Four independent signed 8-bit block-mean curves track the two five-second and two 2.5-second paths at 180 Hz. Each sample is centered in a 245-frame boxcar, and the moving tap interpolates adjacent means. The four channels retain their own feedback value. The same PI32v2 compiler used for the stock links measured 2,748 bytes of state and 422 bytes of standalone object code. The 16-byte headroom is conditional on the current dry INIT layout. This private host experiment adds no production code.

The two original factory restores, note streams, feedback-zero controls, smoothing, four-delay topology, final reverb and master processing run through the same comparison harness. The original peak, RMS, counts and oldest lag matched the established authored witness byte for byte. The two runs received identical per-owner left delay inputs (full capture digests in `input-identity.txt`). All 3,158 selected frames per program were contiguous at frames 261,442–264,599. Original and candidate outputs were finite and all left-tap reads were nonzero.

| Program | Left tap level / spectrum (dB) | Final left level / spectrum (dB) | Final right level / spectrum (dB) |
| --- | ---: | ---: | ---: |
| Dat Bomb Bass, Yo | −6.738 / 21.020 | −6.720 / 20.957 | −5.485 / 18.473 |
| June Oh 06 Sqr | −18.499 / 28.775 | −18.735 / 27.596 | −13.309 / 25.294 |

At gain 1, all six comparisons exceeded the predeclared 1 dB RMS level or 3 dB spectral tolerance in `contract.json`. An independent positive control ran eight different 2–9 Hz signals through all paths for 15 seconds. After ring wraparound, the normalized RMS waveform error was 0.8–2.3% across the eight curves. The musical result rejects this rate, quantizer and reconstruction. A finite-state dry-engine replay and other signal representations require their own resource, timing and musical evidence.

With the factory harness built according to the parent delay report, run `python3 replay.py /abs/factory-build /abs/authored_histories.cpp /abs/new-output`. It recompiles the original authored harness with instrumented graph calls, confirms exact original receipts, independently renders both approaches and computes all metrics. Run `cc -O2 -std=c11 -ffp-contract=off slow_control.c -lm -o /tmp/slow-control && /tmp/slow-control` for the positive controls. `target-abi.txt` records the target symbol sizes from compiling `abi.c` with the project PI32v2 options and `-Oz`. No firmware was flashed and callback execution time was not measured for this rejected design.
