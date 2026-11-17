# CI

GitHub Actions runs the Local gate automatically:

- on every pull request into `integration/dnbox-consolidated`
- on every push to `integration/dnbox-consolidated`
- manually via `workflow_dispatch` (the Actions tab lists this only after the workflow exists on `main`)

The workflow is `.github/workflows/gates.yml`. It uses GitHub-hosted `ubuntu-latest` runners with `permissions: contents: read`. There are no secrets, no deploy steps, and no jobs that touch `main` or `gh-pages`.

The firmware job installs public inputs only:

- JieLi Linux toolchain via `tools/get_toolchain.sh` (https://pkgman.jieliapp.com/s/linux-toolchain)
- the three Apache-2.0 AC79 SDK files pinned in `tools/build.py`, from Gitee tag `AC79NN_SDK_V1.2.14_2026-10-13`
- lua5.1, Pillow, Node 22, Playwright Chromium
- the public schwung reference trees (9W9, 8W8, schwung-303, schwung-tb3po)

Dependencies are cached with `actions/cache`. If a public download fails, the log prints `needs X`, the Elite/Pro firmware build is skipped, and every other step (host build, host tests, Lua and smoke tests, UI defects) still runs.

A red X is a real Local-gate failure unless the log shows a CI setup problem.

Allowed 2026-11-17 by the User. Cursor cloud agents stay unused.
