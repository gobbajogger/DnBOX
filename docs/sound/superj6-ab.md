# SuperJ6 original-preset A/B

**S4 FAIL. Zero of 14 complete preset pairs were available for comparison.** The predeclared N03 limits per program are an absolute RMS-level difference of at most 1.0 dB and log-spectral distance of at most 3.0 dB over 20 Hz–16 kHz. N01 allows 14 original-app factory preset renders when the 14/56 comparison ZIP cannot be recovered. This report records the input check before a full-preset render. [Delivery plan, N01 and N03](../DELIVERY-PLAN.md)

The November 21 rerun on the recovered Linux bench reproduced seven original component captures byte for byte. Complete instrument initialization stopped at the first Swift Foundation UUID metadata request. The engine-only AU host, identities and digests for 14 required input records, authenticated release audit, and stereo scoring command are in the [factory bench](../../experiments/sound-hardware-v1/sj6-factory/README.md). The two arms still need complete-preset renders before any score can be reported.

The owner reports a subsequent original engine-only Linux run with 22 oscillator-layer comparisons and three programs stopped on omitted PWM. The compact loader now resolves those missing controls from the build-9 defaults. The corrected oscillator passed 298 controlled probes and eight measurements using the two authored layers of PWM String Pad against the archived analytic reference. This replay had two verified layer settings from the Remote. All 14 complete preset records and the original Linux capture packet were unavailable. The oscillator scores cover an isolated stage. The three-second stereo preset matrix still has zero complete original/firmware pairs. [Loader trace and oscillator comparison](../../experiments/sound-hardware-v1/sj6-factory/oscillator/README.md)

## Loading decoded controls in the engine-only build

Enable `X0X_SUPERJ6_FACTORY=1` for either native model or the Linux host. A factory-bank reader initializes one `sj6_factory_row` to zero per selected program. Feed the authored control IDs and their original binary32 words, in order, through `sj6_factory_add_word`. The first occurrence wins. The supported fields are master volume (0), two waveform selectors (82–83), AMP2 ADSR (119–122), PWM amounts (78–79) and PWM enables (210–211).

Call `engine_superj6_factory_load(owner, &row)` after selecting an idle Juno owner. A successful return stages all 11 resolved fields for the next audio block. Missing PWM values take binary32 positive zero. Authored signed zeros retain their representation. An absent selector, AMP2 value or master, an invalid value or a busy owner rejects the whole row. A new preset or voice that takes ownership before the audio boundary cancels the staged row. The opposite part keeps its own state. The application selects the compact renderer for the decoded program and restores the shipping renderer on ordinary preset adoption. [Decoder](../../firmware/src/app/superj6_factory.h) · [Application integration test](../../tests/host/superj6_factory_load_test.c)

The focused test runs from the repo root with `sh tests/host/superj6_factory_load_test.sh`. Passing a directory containing the authenticated export’s compiled `bank_data.inc` also exercises the complete bank through this entry point. The loader covers the original main-pass fields above. Live LFO destination resolution, remaining instrument and FX parameters, and full-preset stereo scoring have separate evidence requirements.

### Layer-one attack and dry chorus routing

The opt-in renderer can stage SuperJ6 Layer 1 attack (ID 36) for an idle owner. Admission and the audio boundary both check ownership before applying the existing physical attack transfer. A held note or arriving preset cancels the staged write. Ten endpoints across the two parts produced 2,560 stereo frames bit-exact against the direct physical setter.

Factory IDs 40 and 41 retain authored presence. The opt-in route bypasses chorus when both switches explicitly contain zero, including signed zero. The two-owner test matched 512 audible stereo frames bit for bit against the same dry render after master gain. A missing switch or active mode still selects the existing Juno chorus. The test verifies independent owners, rejection, and ordinary project save/reload. Active original chorus and complete original factory playback remain unqualified.

## Verified originals

| Input | Bytes | SHA-256 |
|---|---:|---|
| Original SuperJ6 IPA, version 0.999.8 build 9 | 57,135,325 | `dde4992f24f52d4390391069025b27092a2f327e3a53115bdd53a6ede0dffa8e` |
| Original AU extension ZIP | 24,716,801 | `abe2f2e089306770489fb30c8870f608dc2f2e0804541f624337523679638a28` |
| AU executable inside both containers | 9,282,464 | `a5243e50bda6bd44f463c6e63ea3137802142f55f3c1ea25a5a6e046acba81a1` |
| Standalone app executable inside the IPA | 9,805,344 | `931976820fc5f967a979661aac52e508277e9164fdc33e6b68b1573b69cf290a` |
| J6presets.zip, 640 actual programs in 12 banks | 451,262 | `ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8` |

Both Info.plist files identify iPhoneOS 15.0 or later. Both Mach-O executables target ARM64, LC_BUILD_VERSION platform 2 (iOS). The AU identifies the instrument as `aumu/AuPr/jay6`. The assigned Linux x86_64 executor has no iOS AUv3 host, Simulator, xcrun or corresponding Apple audio runtime. The surviving ARM64 component harness executes selected DSP routines with explicit host substitutions. It does not instantiate the app's bank loader and complete instrument graph. [IPA](https://chatgpt.com/library?file_id=libfile_fde3_f-e74b92cda8fc8191953c4cf5e5f148b5_2021479872048191acbfdca1f870a539&account_id=personal) · [AU archive](https://chatgpt.com/library?file_id=libfile_fde3_f-dbfdf52305a4819196715dea559db8da_2021479872048191acbfdca1f870a539&account_id=personal) · [Banks](https://chatgpt.com/library?file_id=libfile_fde3_f-6ff4bb216f9c8191bb54bd7c912e4624_2021479872048191acbfdca1f870a539&account_id=personal)

Targeted searches of the authorized project files and Library for SuperJ6, 14/56, N01 and original factory preset renders found source, banks and component captures. The project delivery ledger identifies the earlier 56-preset/14-over-unity material as Synth One. No identified archive provided a complete original SuperJ6 factory-preset stereo render with its stimulus. Three visually similar results were checked directly.

| Inspected recording package | What the bytes represent |
|---|---|
| Broad source/reference recovery, `26e07453fe79ff404e4ea92b1a104dfa9f1ef50a9492f958a380ba54d0f69b36` | Updated original sampler-control fixture with 13 original PCM files and a musical fixture with 10. The earlier control ZIP contains 12. |
| Corrective native/source package, `48bca78f9188addb1d9ce53676b4ad3cd66a5489c7eec0d32003ac11091285c5` | 56 WAV files. Five are kit/BREAK assets. The remaining 51 cover 12 sample notes and five oscillator/noise primitives, each with original, port and difference audio. |
| Private model/source continuation, `2bf91cf7a85cb2a0fd1322acd67f07cd2a63b4fd17127c8070d36ae94b1437f6` | Five WAV files, all kit/BREAK assets. Its independent source review also records the missing full instrument graph. |

The next capture needs either the authenticated comparison ZIP or stereo output from the original AU/app for 14 identified factory presets with the actual sample rate, note/velocity/release events, reset, transport and capture length. The candidate must load those same programs and replay those events. One original record is already pinned for a first comparison: *Medieval Sky*, UID `87E999B2-447E-4B2E-9EA9-AE53F4DEE44B` in Sound Test Room, with 114 fields. Its enabled LFO2 has a nonzero amount and destination-index 25. The two installers and original JSON provide the input data; the missing input to the bench is the output of the complete original preset graph and the corresponding qualified candidate preset path. [Original graph and selected record evidence](https://chatgpt.com/library?file_id=libfile_fde3_f-997eee43234c81918afec06ba3d46632_2021479872048191acbfdca1f870a539&account_id=personal)

## Authenticated 640-program replay

The recovered export matched its pinned SHA-256. Both selector paths returned all 640 programs in 12 banks and preserved all 90,549 ordered binary32 values. The real firmware loader accepted every program and verified 4,580 authored envelope, waveform, PWM and master words, 640 authored register words and 2,460 PWM defaults. The archive still contains legacy fields with unresolved sounding behavior. Complete active FX and preset execution remain open. [Bank verifier and fixture](../../experiments/sound-hardware-v1/sj6-factory/README.md).

## Next non-envelope controls supported by the original evidence

The next group comprises waveform IDs 82/83 and per-layer PWM amount/enable IDs 78/79 and 210/211. The original setters select waveform by truncating three times the incoming binary32 normalized value after promotion to double. The four stops are sine, triangle, saw and square. Enable uses a strict threshold above 0.5. A square with PWM enabled uses duty `0.5 − 0.47 × amount`; the other waveform states use 0.5. All 640 programs contain both waveform values. Twenty-five contain the newer per-layer PWM settings. Qualifying pre-note selection and the enable threshold can proceed from these observations. Switching a waveform under held notes also invokes detune/retrigger behavior that still needs separate verification. [Clean control catalog](../../experiments/sound-hardware-v1/mapping/results/implementer_brief/public_control_catalog.json) · [Original route audit](https://chatgpt.com/library?file_id=libfile_fde3_f-997eee43234c81918afec06ba3d46632_2021479872048191acbfdca1f870a539&account_id=personal)
