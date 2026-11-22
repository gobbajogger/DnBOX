# Three histories for the original Synth One stereo delay

The original right fill-in line and the right line after the long delay have identical time, interpolation and feedback controls. Their outputs add. One short line can instead take the sum of their original inputs. The left and right long histories stay independent and each Synth One retains its own three histories. The maximum base time is 2.5 s. At 44,100 Hz the two long histories each need 220,500 samples and the short one needs 110,250, plus four guard samples per history.

| History format | Bytes per owner | Factory final stereo | Isolated delay stereo | Worst final spectrum |
| --- | ---: | ---: | ---: | ---: |
| Three binary32 histories | 2,205,048 | 694/694 | 694/694 | 0.0235 dB |
| Three binary16 histories | 1,102,524 | 692/694 | 694/694 | 9.404 dB |

Every program ran its 144 ordered original setter calls and the same three-second note, overlap and release schedule on all three arms. The original frontend, voices and downstream effects remained active. The probe checks identical left and right samples at the delay input before comparing isolated returns and final output. Of 694 original programs, 525 have the delay on with positive mix. The two final binary16 failures were *Theoretical Evolution Pad* and *Awakening Drone*. Limits were 1 dB level and 3 dB spectral RMS at unity gain. The original four-line constructor uses 7,056,016 bytes per owner. Both smaller designs still exceed the native pool. [Summary and inputs](summary.json) · [Every program, stereo scores and PCM hashes](rows.jsonl.gz)

A separate 15-second two-owner run exercised independent inputs, the maximum five-second long tap, continuously swept time, high feedback, changes while signals were silent, ring wraps and independently timed resets. All twelve owner/time intervals passed on both channels with both formats. [Live control results](live-scores.json)

Build the pinned Synth One factory harness first. From this directory run `python3 prepare.py /path/to/DnBOX /path/to/factory-build /tmp/folded`, `python3 music.py /path/to/factory-build /tmp/folded/probe /tmp/folded/rows` and `python3 live.py /path/to/factory-build /tmp/folded/live-sweep`. The host replay uses Linux in-memory files for temporary PCM. [Original factory harness](../../../../tests/host/run_synthone_factory.py)
