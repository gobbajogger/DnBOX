# Exact local flash fit synthesis

Both models must have zero or greater XIP free; no extra 1 KiB. Pool minimum remains 512 B outside the 8,192 B reserve. No hardware/emulator, installer, publication, flash-map edit or protected write.

| Actual, unchanged physical layout | Elite XIP free B | Pro XIP free B | Pool safe margin each |
|---|---:|---:|---:|
| Recovered seven-control baseline | -8,492 | -8,480 | 512 |
| Joined seven controls + ADSR/FILTER LINK + LEVEL + four lossless changes | **-9,508** | **-9,500** | 512 |
| Previous + experimental exact dense dictionary | -9,392 | -9,380 | 512 |
| Original joined + exact Hz clock source (without dense) | **-12,160** | **-12,152** | 512 |

Map and native link log results, not sums. Fixed app capacity 552,672 B. Pool used 335,360 B and raw free 8,704 B. Four retained reductions: ARC, Chicago E mixed headers, shared Chicago/Miami exact dictionary, shared UI page emitter. Exact joined host: compositor 1,186,859; OSC/PWM 73; ARP 126 + 66; ADSR+LINK 312; LEVEL 69; S1 LINK 40. Exact Chicago oracle checked all 32,770 values with sanitizers; preset dictionary exact 2,578 checks + corruption mutant. Hz exact focused host passed 316. No audible LFO graph or native timing claim.

## Disposable external placement accounting, never a device layout

The previous 7,539 B group is now 7,119 live bytes because the Chicago dictionary was removed by the separately measured exact sharing. This must not count twice.

| Disposable fixture | .lab_cold B with modeled metadata | Actual net app bytes removed Elite / Pro | XIP free Elite / Pro |
|---|---:|---:|---:|
| Prior live immutable set only | 7,135 | 7,120 / 7,120 | -2,388 / -2,380 |
| Above + passive arc/pictogram/clip/Chicago offsets | 8,866 | 8,848 / 8,852 | -660 / -648 |
| Above + passive display APC/pad colors, drum geometry/labels, clip mode and panel titles | 9,704 | 9,684 / 9,688 | **+176 / +188** |
| Last overlay on the exact Hz source | 9,704 | 9,684 / 9,688 | **-2,476 / -2,464** |

The app retains its actual hard limits; the isolated linker routes designated readonly inputs to a throwaway separate logical address for accounting. No device layout or asset package exists for this variant. FONT_T remains app-internal at 1,103 B and XDATA 7,775 B throughout; no recovery resource moved. Real net deltas account for app code/link layout, not just payload; the external section includes a 16-byte placeholder header and actual linker padding. There is no validation, package, driver, restore implementation or guard code in that budget. An explicitly approved address, app+asset identity checks before first use, full readback, power-loss recovery and paired rollback are required before deployment.

## Further exact stock-layout experiment

The separate dense dictionary stores 24-bit tails +3-bit top-byte tags (seven exact prefixes) with old slot identities and exact values. Static 1,728 B becomes nominal 1,466 B; decoder/link reduce net gain to 116/120 B. All 111 Chicago/Miami prepared snapshots and names directly matched, unchanged metadata oracle and mutant passed. Not mixed into the primary or Hz arms, and remains >9 KiB short. Do not add its savings to the fixture because the fixture moves that dictionary. Existing 55,275 B 909 sample data is not a quick safe replacement: exact independently addressed 2 KiB chunks save at best 3,090 B before decoder/scratch, unbounded xz saves gross 6,612 but cannot substitute for bounded real-time playback. No hot repack promoted.

## Recover after /tmp reset

Persisted source patch chain is in source/; candidate and oracle digests and a disposable linker-only helper accompany it. No raw private payloads, ELF or installable bytes saved in this packet.

1. Copy persistent sj6-knob-persistence/checkout; restore expired symlinks as local regular files from unchanged corresponding full-fw-next, full-fw-integ, fw, and xip-lossless/rec-fw files. Supply BLE from fw and tools/mboot from full-fw-next. Restore generated dirs, then use matching exact-ble-cold-fw felucca_font.h in model and host GEN dirs (FONT_XDATA_LEN 7759, not 8140). Restore revision-pinned Chicago test oracle/Python from full-fw-integ and composed, arc generator and original program tests from fw. Apply only tools/gen_chicago_compact.py hunk of sj6-boot-safe-assets/engine-and-exact-table-source.patch to the full-fw-next generator for the exact A-only baseline; regenerate Chicago headers. Require the previous normal-map deficits before comparisons.
2. Apply source/superj6-envelope-panel.patch, source/superj6-master-panel.patch, source/chicago-e-mixed.patch, **source/chicago-e-generator.patch**, source/verified-smc-arc.patch, source/program-dict-union.patch, source/page-emit.patch. Do not patch through a symlink. Regenerate Chicago headers for both native models and the local common program metadata header. Add the source/superj6-hz-source.patch + optional host-test patch only in a separate Hz arm. The source/program-dense-tag.patch is an isolated different arm; regenerate the dictionary after it.
3. Native normal link uses sibling native-link-only.py + pinned SDK/toolchain and X0X_SUPERJ6_FACTORY=1 X0X_SUPERJ6_BUILTIN_INIT=1 X0X_ARP_HOOKS=1 X0X_SYNTHONE_PATCH=1 X0X_S1C=1. Host uses standard driver with -DX0X_CHICAGO_COMPACT_HEADERS=1 -Ibuild/smk37-elite/gen. For disposable links only use cold-link-fixture.py in same environment, FIT_LAB_OUTPUT writable disposable location, FIT_LAB_PROFILE prior/passive/display. It stops before packing and never writes the original linker script.
