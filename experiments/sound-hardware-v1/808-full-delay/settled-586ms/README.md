# Private 808 delay at 586.304 ms

The settled-tap kernel uses a 26,112-word private history. At a settled delay of at least 16,384 samples, the fractional part falls on a 1/512-sample grid. The write pointer and both read pointers then advance together until a ring boundary. The original interpolator still handles live time changes and shorter taps.

In the matched-input comparison against original 8W8, all 257 in-range full mixes and all 234 energized isolated returns passed. A four-second repeated-hit stimulus followed by a four-second tail passed 68/68 full mixes, 48/48 isolated returns and 68/68 tails. With the private delay enabled at its maximum, the populated Chicago/Miami project reached 11,558 µs on Elite and 11,508 µs on Pro. With the ordinary shared send, Elite reached 12,214 µs and recorded four late transfers and four xruns. Seven of the eight candidate runs passed. The ordinary release retains its 429.57 ms history.

The retained kernel is the DSP function from the candidate. Its randomized regression compared both stereo outputs, every ring word, filter state and cursors against the scalar loop for 5,500 blocks. It passed at `-O0` and `-O2` with undefined-behavior checks. The packaging driver below reconstructs that comparison from checkout `e3ffb3d8`. The standalone packaging command still needs a clean replay.

```sh
python replay-settled.py /path/to/DnBOX-at-e3ffb3d8
```

[Measured results and retained PCM digests](retained-snapshot.json) · [DSP function](settled-fxbus.c.inc) · [Regression source](settled-exact.c)

The original-render harness accepts the 26,112-frame candidate checkout, an 8W8 checkout containing the pinned upstream commit and a fresh output directory. It records all 338 settings and scores the 257 within the candidate’s delay span. Add `--sustained` for the 68 repeated-hit and four-second-tail cases. A compile-time check rejects a different ring size. The packaged invocation still needs a clean replay.

```sh
python original/delay_voices.py /path/to/586ms-candidate /path/to/8W8 /tmp/808-standard
python original/delay_voices.py /path/to/586ms-candidate /path/to/8W8 /tmp/808-tails --sustained
```
