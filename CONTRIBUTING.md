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

1. Coordinate with the lane owner, branch from `integration/dnbox-consolidated`, and open a draft pull request targeting that branch for code or documentation. Do not push or merge to `main`; final sign-off belongs to the User. Follow the pull request template.
2. Keep changes focused. Include or link the single required nonsecret gate log for the exact PR head, following [docs/DELIVERY-PLAN.md](docs/DELIVERY-PLAN.md). Do not commit firmware images (`.fwsc`, `.bin`, `.elf`, `.img`), unrelated logs, private/raw captures, dumps, credentials, SDK/toolchain binaries, or zip archives.
3. Follow the licence in [LICENSE](LICENSE) and the third-party notices in [LICENSING.md](LICENSING.md) and [CREDITS.md](CREDITS.md).
4. Build and test guidance is in [BUILDING.md](BUILDING.md).
5. Do not use GitHub Actions or add or change GitHub workflow files under `.github/workflows/`.


## Branches

Max 15 branches per repo; every branch needs an open PR; delete the branch right after merge/close, after the archive/<branch> tag.

Protected branches (`main`, `gh-pages`, `nightly` where it exists, and `integration/dnbox-consolidated`) count toward the cap. Keep at most two `backup/*` branches. Work that is closed or merged is preserved by the `archive/<branch>` tag, then the branch is deleted.
## Releases

Follow [docs/DELIVERY-PLAN.md](docs/DELIVERY-PLAN.md) and current owner instructions for qualification, integration and delivery. A source PR does not authorize a release, publication, a device write or a merge to `main`.

## Security

See [SECURITY.md](SECURITY.md). Do not file a public issue for a vulnerability.
