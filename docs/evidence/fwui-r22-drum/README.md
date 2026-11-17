# R22 drum source checkpoint evidence
Native input manifest and render archive describe actual frozen input bytes, not an inferred HEAD. Publication adds this evidence and lane status without changing measured product/build inputs. Native binaries/toolchains are not included. Review source hashes before reusing qualification.

Extract render-evidence.tar.gz and run the unmodified tests/ui_defects/first_slice/evaluate.py --render-dir render --output evaluation-recheck.json. It contains45 PNGs, actual traces/logs, source-closure.json and instrumented receipts. Limits remain70% panel/12% empty-rectangle; only45 drum states/checks1–4 are covered.

Fit is calculated independently from actual ELF _pool_end/_pool_start and image bytes: capacity344064 minus used335244 minus reserve8192 =628B. The inherited build log label 'pool margin:8820' is raw free, NOT accepted margin. No PSRAM grep gate was used.

Full Local gate is FAIL, not merge approval. Current isolated snapshot has no Git metadata; environment/setup and all-lane checks remain integration work. No hardware result, nightly visual baseline regeneration or full M2 completion is claimed.
