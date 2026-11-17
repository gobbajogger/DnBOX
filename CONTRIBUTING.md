# Contributing

Thanks for your interest in DnBOX. By taking part in this repository you agree to follow the [Code of Conduct](CODE_OF_CONDUCT.md).

## Using the firmware

Install from the [web installer](https://gobbajogger.github.io/DnBOX/install/) or the [latest GitHub release](https://github.com/gobbajogger/DnBOX/releases/latest) (v0.0.25.1). Pick the Elite or Pro package that matches your keyboard, back up sample slots first, and do not unplug USB while writing. See the [README](README.md) and the [manual](https://gobbajogger.github.io/DnBOX/manual/).

## Reporting a problem or idea

Please use the issue forms:

- Bug report: keyboard model (Elite / Pro), firmware version, steps to reproduce, and whether you backed up sample slots
- Feature request

Do not attach private backups, credentials, or raw captures that you do not want public.

## Code and documentation changes

1. Branch from `integration/dnbox-consolidated` and open a pull request into that branch (not `main`). `main` is reserved for the owner's final sign-off. Follow the pull request template.
2. Keep changes focused. Do not commit secrets, firmware images (`.fwsc`, `.bin`, `.elf`, `.img`), logs, dumps, or installer zip archives.
3. Follow the licence in [LICENSE](LICENSE) and the third-party notices in [LICENSING.md](LICENSING.md) and [CREDITS.md](CREDITS.md).
4. Run the local gate and paste its final line in the pull request:

   `scripts/gate.sh <lane>`

   It wraps `tools/dnbox-gate.sh` and prints `DNBOX-GATE: PASS <sha>` or `DNBOX-GATE: FAIL <sha> <failed checks>`. Name the lane (`sound`, `fwui`, or `integ`) and list the done checks the PR covers. Build and test guidance is also in [BUILDING.md](BUILDING.md).
5. Do not change GitHub workflow files under `.github/workflows/` unless a maintainer asks you to. GitHub Actions are off on this repository; checks run locally.

## Releases

Integration-branch tags for finished delivery-plan moves are described in [docs/RELEASES.md](docs/RELEASES.md). Public package releases stay on `v*` tags and `main`.

## Security

See [SECURITY.md](SECURITY.md). Do not file a public issue for a vulnerability.
