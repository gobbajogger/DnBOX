# Isolated S1 voice-split experiment
The preserved 3+3 prototype from before the runner reset was not available; this reconstructs the same class of shortcut on e00f689d. This is evidence, not code for Nightly. Apply `combined-worker-experiment.patch` alone to e00f689d. It includes the Room/voice optimizations from the previous proof. Arm with `-DDNBOX_S1_WORK_SPLIT=1`; `-DDNBOX_S1_SPLIT_START=2` chooses 2+4. No new audio-pool allocation, but uses the otherwise unused 5,120 B independent B workspace. Not eligible when the other work owns core 1; current scope is stable six-voice poly (no audio-rate voice LFO selections and zero noise, exact smoothed voice parameters). If it is ineligible, the complete serial path remains.

The second core gets a private voice/control snapshot and publishes float terms. The first core retains ordered sums, the actual shared effects, and live voices. If a worker fails, the primary replays the upper voices from untouched original state before committing results. Host test covers failed submissions and injected faults after an 8-frame tile. Hardware publication would also need dispatch-tail and watchdog qualification under realistic scheduling.

## Pro strict Synth One B, Chicago A inactive, six MIDI notes, no drums

| Work placement | Worst completed block, µs | Last completed block, µs | Completed halves | XIP spare |
|---|---:|---:|---:|---:|
| Codec + identical ingress + held envelope, serial | 23,902 | 21,584 | 346 | 2,336 B |
| 3+3 | 17,220 | 16,035 | 352 | 1,276 B |
| 2+4 | 16,493 | 14,201 | 352 | 1,260 B |
| 2+4, voice-major worker tiles | 16,460 | 14,088 | 352 | 1,244 B |
| 1+5, 112-frame work tile | 16,443 | 12,764 | 352 | 1,292 B |

The 1+5 arm uses `-DDNBOX_S1_SPLIT_START=1 -DS1C_SPLIT_SPAN=112u`. The voice-major arm is opt-in with `-DDNBOX_S1_VOICE_MAJOR=1`.

The split replays have both full worker scratch output and six active Synth One gates, no mailbox fallbacks and no skipped cycles. Their PCM captures were identical to each other. The parity counter sees 38 late callbacks, not a real count of all drops. The current build misses the 11,610 µs deadline despite not exhibiting long exact raw repetitions in this waveform. Idle synthesis owner A completed 391 callback halves in the same length control; that is a separate unloaded comparator. Do not extrapolate to drum-load or owner A.

Both unforced Elite and Pro linked the bit-exact serial Room/voice candidate. Elite A 24,315 → 23,908 worst and 24,169 → 21,593 last, 340 → 346 completed; spare XIP 2,324 B. Those readings are from a saved project with actual six gates and no flash initialization. An erased NOR seed without the family marker boots a different project and is invalid even if the six USB sends appear in the log. `seed.sh` preserves the model's native companions and fixture-family marker. Run `build-project.sh` with SYNTH_OWNER=B for the B test.
