# SuperJ6 LFO 2 PWM2 receiver checkpoint

Apply superj6-pwm2-lfo2-source.patch after the PWM1 source packet, or to the fit worker's joined PWM1 tree. Both forward checks and the candidate reverse check passed. The host-test patch updates the PWM1 packet's shared test fixture. The incremental patch has no asset or map changes.

The shared LFO2 PWM page selects OFF, PWM 1 or PWM 2 on K3. K4 follows the selected base, including a base turn before the selected route has committed. With OFF, K4 still adjusts PWM1 and K2 retains its depth. The two square-wave gates remain on PULSE WIDTH. PWM1 projects keep byte 21 and bit 27 unchanged. PWM2 adds route bit 28 to the same 32-byte cell. Valid OFF and PWM1 encodings round trip exactly. Both route bits set is invalid. No new renderer or state array is required. The host juno6_t remains 2,936 bytes, aligned to eight bytes.

The 2,074-check replay includes 384 packed route/depth settings, malformed markers and an invalid combined route, the 8,192-frame independent transfer calculation for each receiver, exact bypass at depth zero and the strict .5 PWM gate, and the PWM2 clamp on three control ticks. Controller detents during held notes changed 6,666 of 8,192 frames on bass A and 6,649 on bass B for each route. In the dry peer channels there were zero changes. Both routes and owners passed exact active and OFF project save/reload with retained depth, base alias ownership and preset isolation. The prior PWM1 and current PWM2 candidate legacy fixtures matched 49,152 stereo frames bit for bit across both owners and Juno, Synth One, Chicago and Miami.

The packet uses the existing tests/host/sj6-smc-ui.sh and PWM1 packet's build flags. The two fixture names are ui_superj6_pwm1_lfo2_test.c and ui_superj6_legacy_pcm_test.c. Capture legacy.pcm in a temporary output directory and compare with the corresponding PWM1 legacy fixture output. Tests and receipts use host code. Native fit and strict timing remain with the fit worker.

BUILD-9-PWM2-RECEIVER.md records the receiver law and the distinct original depth, bases and selectors. Original selector lookup, the second simultaneous route and the full control inventory remain open.

The Hz-rate, oscillator and PWM, envelope and LINK, LEVEL, ARP SEQ keyboard and Synth One LINK regressions pass on the same source. The Hz replay matched all 3,840 authored rate and mode words across 640 authenticated records. See regressions.log.
