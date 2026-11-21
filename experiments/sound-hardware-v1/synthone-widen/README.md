# Final stereo widening on Synth One

The 44.1 kHz original factory chain mixes its right channel with a zero-feedback 2,205-frame tap after the compressor. The left channel passes through. The candidate stores the high 24 bits of each input float, including its sign and exponent, and clocks the ring even at zero wet mix. It needs 6,615 bytes per selected part and preserves over-unity inputs.

| Factory comparison | Two-byte history | Three-byte history |
| --- | ---: | ---: |
| Complete program stimuli | 694 | 694 |
| Within 1 dB level and 3 dB spectral limits on both channels | 692 | 694 |
| Programs with different right-channel words | 214 | 214 |
| Largest absolute right-channel level difference | 0.025523 dB | 0.000099 dB |
| Largest right-channel spectral RMS difference | 4.654807 dB | 0.003267 dB |

Both arms processed 144 commands and 88,200 stereo frames for each program through the factory fixture. Every left-channel word matched. The two-byte history failed on *BASS - Dubchill* and *JEC Alarming!*. The three-byte result's largest level difference was on *After the storm* and its largest spectral difference was on *Funkularist Bass Arp*. The compressed JSONL files contain 694 original and candidate PCM hashes and scores per precision. This test substitutes the final widening tap only. It retains the original upstream effects histories for both renderers.

To reproduce, use the recovered factory build from the sound lane with its pinned command lists, closure, objects and original bank inputs. Each run needs a fresh output directory.

```sh
python3 replay.py "$FACTORY_BUILD" /tmp/s1-wide-24
python3 replay.py "$FACTORY_BUILD" /tmp/s1-wide-16 --history-bits 16
```

The two-byte run exits 1 for the two out-of-tolerance programs. [Application ownership and native builds](selected-ownership/README.md) exercise a separate integration in the real part renderer. Both selected parts retain distinct histories across switching and project reloads. Each borrows 4,095 bytes from its retired Chicago insert and 2,520 from its backend union. Both links add 368 flash bytes and retain 8,972 bytes of free pool. The multi-second feedback delay also needs owner storage. The combined graph has yet to qualify on either board.

The [editable FWUI widening prototype](fwui-reconstruction.md) adds a smoothed per-part target and preserves existing 13-slot snapshots and project cells. Its recovered source still needs native qualification.
