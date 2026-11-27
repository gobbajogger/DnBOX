# Build configuration and native controls check

Pin the six actively qualified Part Sound switches and the host C definitions in the runtime attestation. The focused unit test proves toggling any of them changes its recorded inputs. A stock gate receipt must not satisfy a different build/visual profile. No release default or cache relaxation.

Additional focused control checks against the held-note policy and input path:

- 1,250 physical key checks passed, including all 37 stopped and playing recording keys, actual pitches 36–72, FIFO/alias and release ownership.
- Dedicated Pattern knob does not change engine presets, K6 still does. Empty automation lanes do not veto choices; current engine automation survives preset changes and parks/restores independently per engine. These targeted binaries passed with ASan/UBSan, with LSan off in the diagnostic rerun because this runner blocks it.
- Broader release qualification remains pending. The last official attempt used an older test tree and exposed environment constraints; do not treat passing isolated tests as official gate success.
