# SuperJ6 Raw source Sub: second independent mismatch at OTA

Using the authenticated current receiver and the corrected physical-note pitch sender, the full source Sub witness reproduced terminal digest c303865350c83e58de687ee06b1d5ced6495f4757cf9101296826b2314a5dd30. Then each voice was captured at the current original source's exact pre- and post-OTA instructions, before AMP, without local time/phase or gain matching. The same source input was fed into the real compact OTA and the independently reconstructed physical OTA already tested on Main. The actual filter control words were constant and feedback zero for the four notes under test.

| Physical note | Input frames | Compact OTA signed residual | Physical OTA signed residual |
|---|---:|---:|---:|
| 36 | 22,610 | −10.01 dBr | −141.26 dBr |
| 48 | 33,635 | −14.33 dBr | −141.29 dBr |
| 52 | 33,635 | −14.95 dBr | −141.31 dBr |
| 55 | 33,635 | −15.22 dBr | −141.35 dBr |

A perfect carrier would not remove the measured compact OTA error. The physical leaf works on both measured source types at this cutoff and feedback, not just Main. This does not qualify a firmware replacement, other filter controls, release envelopes, or whole output matching. Preserve independent carrier and filter changes when testing, do not use master gain to approximate either. No original recording, parameters, proprietary executable, or product change here.

Metrics file SHA-256 `4cb0d82c1fb2cb600df0c3e226ce239e85e2bf87a40ddb8d3df3f3b30ea0007d`.
