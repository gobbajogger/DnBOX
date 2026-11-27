# Synth One semitone and shared meter combined loaded check

The exact standalone semitone lookup and meter maximum both cleared isolated tests. Both combined guarded diagnostic builds linked, but the combined arm did not clear the loaded goal:

| Board | Free XIP | Worst block | XRUNs | Baseline fixed-AB |
|---|---:|---:|---:|---:|
| Elite | 716 B | 12529 µs | 7 | 12155 µs / 7 |
| Pro | 724 B | 12370 µs | 10 | 12351 µs / 8 |

Raw audio pool 8756 B, both 319488 rendered frames, 5 settled ownership checks and unchanged virtual NOR. The same 922-event tape does not imply exactly aligned event application: Elite readback timestamps differed, trace PCM began diverging at frame 97859. Therefore this is not a differential timing or sonic attribution. Do not promote, and do not count the isolated improvements as integrated headroom.

ELFs: Elite db60af96df108e44d6d03cba4b42fce43aa9e6f8c5ab7e6c48fc796b750e296f; Pro b149f494915e00c8ee332552117d1fdda416703c49cb5d26c105a0b7627216a8. Retain the existing diagnostic baseline. Keep the 10809 µs/0 dropout requirement.
