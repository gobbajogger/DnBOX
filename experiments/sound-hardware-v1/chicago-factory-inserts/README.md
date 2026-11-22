# Chicago factory inserts on SMK-37 Elite and Pro

The ordinary Elite and Pro firmware uses the archived full-rate BPM Delay and floating-point Hall allpasses. The other profiles keep the smaller selected-effect history. Either bass part can select Chicago, and both can run separate effects simultaneously.

The replay reads every original factory preset and its note scenario. It renders the stored effect and bypass through the firmware, then renders the same input with the archived insert implementation. All 81 full stereo renders match sample-for-sample. All 77 active effects also match after bypass subtraction. Four bypass programs null on both channels. With the compact histories, 71 complete renders matched exactly and 80 met both the 1 dB level and 3 dB spectral limits. DLY BSS exceeded the spectral limit at 3.3135 dB. These measurements compare the port to its archived source. Wet recordings from the Korg AU are still required for an independent factory comparison.

From a Git checkout with the repository's host-render dependencies installed, run each arm into a new output directory. The reference reconstruction reads the archived headers from commit `bf48ce83bea6f8d332dddff11e5407c4fd744fc6`.

```sh
python experiments/sound-hardware-v1/chicago-factory-inserts/replay.py . /tmp/chicago-full --full-history
python experiments/sound-hardware-v1/chicago-factory-inserts/replay.py . /tmp/chicago-compact
```

The gzip JSON files contain both channel scores and the digests for the archived, firmware and bypass renders of every program. The shipping receipt identifies both native application packages, all six PCM digests, the measured callback peaks and post-boot health checks. Each board retains 16,100 free pool bytes, including 7,908 above reserve. Free application flash is 7,796 bytes on Elite and 7,804 on Pro.

[Full-history scores](full-history.json.gz) · [Compact scores](compact.json.gz) · [Shipping scenes](shipping-scenes.json)
