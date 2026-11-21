# SuperJ6 original-preset A/B

**S4 FAIL. Zero of 14 complete preset pairs were available for comparison.** The predeclared N03 limits per program are an absolute RMS-level difference of at most 1.0 dB and log-spectral distance of at most 3.0 dB over 20 Hz–16 kHz. N01 allows 14 original-app factory preset renders when the 14/56 comparison ZIP cannot be recovered. This report records the input check before a full-preset render. [Delivery plan, N01 and N03](../DELIVERY-PLAN.md)

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

## Next non-envelope controls supported by the original evidence

The next group comprises waveform IDs 82/83 and per-layer PWM amount/enable IDs 78/79 and 210/211. The original setters select waveform by truncating three times the incoming binary32 normalized value after promotion to double. The four stops are sine, triangle, saw and square. Enable uses a strict threshold above 0.5. A square with PWM enabled uses duty `0.5 − 0.47 × amount`; the other waveform states use 0.5. All 640 programs contain both waveform values. Twenty-five contain the newer per-layer PWM settings. Qualifying pre-note selection and the enable threshold can proceed from these observations. Switching a waveform under held notes also invokes detune/retrigger behavior that still needs separate verification. [Clean control catalog](../../experiments/sound-hardware-v1/mapping/results/implementer_brief/public_control_catalog.json) · [Original route audit](https://chatgpt.com/library?file_id=libfile_fde3_f-997eee43234c81918afec06ba3d46632_2021479872048191acbfdca1f870a539&account_id=personal)
