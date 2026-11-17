# Changelog

All notable published and integration-branch releases of the DnBOX firmware repository are listed here. Public packages stay on `v*` tags from `main`. Integration tags on `integration/dnbox-consolidated` are prereleases; see [docs/RELEASES.md](docs/RELEASES.md).

## Unreleased — `integration/dnbox-consolidated`

Work toward the next public release. See [docs/START-POINT.md](docs/START-POINT.md), [docs/DELIVERY-PLAN.md](docs/DELIVERY-PLAN.md), and [tracking issue #967](https://github.com/gobbajogger/DnBOX/issues/967).

## [dnbox-start-2026-11-17] — 2026-11-17 (prerelease)

Clean starting point on `integration/dnbox-consolidated` (Asia/Vladivostok, UTC+10).

- Snapshot SHA at the tag: `7b17a64e11731de50f4fe46ef7ebe3948a730a10` (merge of #962, truthful fit-test gate). Hygiene #966 later landed on the same branch.
- M0 start tag: `start-fw-2026-11-17` (`ea4d1d37edca8dd39469573107a49309d0ea68d7`).
- This is a prerelease marker, not a firmware download. Public packages remain at v0.0.25.1.

## [0.0.25.1] — 2026-11-13

Current public release. Firmware for the M-VAVE SMK-37 Elite and SMK-37 Pro.

- A chain slot now lasts for the longest playing part in the current pattern (909, 808, both 303s and BREAK), counting length and rate, rounded up to a whole 909 bar. Muted, part-off and empty parts do not extend the slot. Chaining stays live-only and is not saved.
- Installer: [GitHub Pages](https://gobbajogger.github.io/DnBOX/install/). Release: [v0.0.25.1](https://github.com/gobbajogger/DnBOX/releases/tag/v0.0.25.1).
- Packages: `dnbox-x0x37-0.0.25.1-elite.fwsc` and `dnbox-x0x37-0.0.25.1-pro.fwsc`. Source commit: `79280e763341d03ba43266d3e25dad66f12de860`.
- Test status: owner-reported test on an SMK-37 Elite, not a full hardware qualification. The Pro package has not been tested on a device. Software checks passed (emu360 timing gate and bundled Bluetooth controller check). The emulator result is not a hardware result.
