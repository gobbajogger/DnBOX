# Native firmware qualification

The six application links cover stock SMK-37 Elite and Pro, and both boards with the Sound provider's Synth One patch disabled and enabled. Each linker produced its ELF and map together. The build then packaged that ELF through the normal board build. [Fit, artifacts and hashes](fit.json)

To replay the stock pair, use the published commit recorded in `fit.json`, the recorded toolchain and SDK inputs, and the ten original data inputs checked by `tools/check_original_inputs.py`.

```sh
sh build.sh --board smk37 --model elite
sh build.sh --board smk37 --model pro
```

To replay the Sound composition and its four board links, run the qualifier at the recorded commit in a repository that contains both cited lane revisions.

```sh
python3 tests/host/qualify_fwui_s1.py build/fwui-sound-qualification --boards
```

For an exact link-map comparison, add the recorded `-Map` argument to the application linker invocation. The raw maps record local LTO temporary paths, so repeated runs can differ in those paths while producing the same ELF and package. [Six linker commands](link-commands.json.gz) · [Build logs](build-logs.json.gz) · [Input Git blob IDs](source-manifest.json.gz)
