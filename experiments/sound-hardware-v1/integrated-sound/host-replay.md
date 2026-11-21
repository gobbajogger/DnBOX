# Replay the original-engine host checks

The commands in all-engines.json.gz use POSIX shell environment variables. Use detached worktrees for the before and joined source arms. The pre-merge measurements used tree 0f637e27f2ec1e359deed9fa5c6e2bc9316c453c. Public commit 063f2306fef6e575067b8fc62714fec5d5f55222 reproduces its code with only documentation and header-comment differences. Joined source 5837c73f754a667c526edd420aefbe7cf3a071bf and report 2fe5930c161a30c907a4b91991ab3c9724380b10 have identical tested firmware, assets and test harnesses.

Set BEFORE and JOINED to the source worktrees. Set OUT to a new output directory. Set EIGHTW8_DIR and NINEW9_DIR to the original athousanddetails/schwung-8W8 and athousanddetails/schwung-9W9 checkouts, and BBGEN to mestela/schwung-breakbeat. Set REFERENCE_REPO to a DnBOX checkout containing the archived sampler commit, PWM_FIELD_REPO to the nightly reference checkout containing the verified preset fields and S1_ORIGINAL_INPUTS to a new directory for the authenticated Synth One fixture. The exact commit IDs and fixture digests for all inputs are in the receipt.

~~~sh
export BEFORE JOINED OUT EIGHTW8_DIR NINEW9_DIR BBGEN REFERENCE_REPO PWM_FIELD_REPO S1_ORIGINAL_INPUTS
export OPENBLAS_NUM_THREADS=1 OMP_NUM_THREADS=1 ASAN_OPTIONS=detect_leaks=0
mkdir -p "$OUT/before" "$OUT/joined"
(cd "$JOINED" && sh host/build_host.sh --generate-only)
(cd "$BEFORE" && sh host/build_host.sh --generate-only)
(cd "$JOINED" && python3 tools/restore-synthone-test-inputs.py --bundle assets/test-inputs/synthone --output "$S1_ORIGINAL_INPUTS")
~~~

For the before arm, restore the instrumentation listed under premerge_oracle_files.sha256 from qualification_pr985 into the isolated worktree. Those files contain original AU measurements, a renderer and comparison harnesses. They leave the firmware source intact. To reproduce the old release defect, use the shipping 808 wrapper specified by the 808-production-original command. The older default 808 wrapper disables the shipping tail cutoff and passes.

Read the command and cwd fields with Python's gzip and json modules. Run one command at a time from its cwd. Commands that expect a fresh directory need a new OUT for a second run. The receipts distinguish the 909 original-WAV level and spectrum goals from its tighter pointwise test. The historical and shipping pointwise counts are both 133.

## Miami Stereo and historical Bass Split

With the host-only test patch applied, run the following from the current repository root. The first command selects the current public Stereo control, ID 15. The second selects the archived WBL1 Bass Split contract using its own implementation and matching headers. The last command verifies that the current test rejects a mono render. It requires the GNU linker on Linux. A successful negative probe exits zero only after the dispatcher rejects the mutant with the expected stereo diagnostic.

~~~sh
sh tests/run_tests.sh miami-stereo
legacy=$(mktemp -d)
git archive 0c228a97e67a73c0b377f62728603d94fb1bec66 -- tests/host/miami_stereo_split_test.c experiments/wobble/blep.h experiments/wobble/wobble.c experiments/wobble/wobble.h firmware/src/dsp/fastmath.h | tar -x -C "$legacy"
sh tests/host/run_miami_stereo_split.sh --legacy "$legacy"
sh "$EVIDENCE_DIR/miami-mono-negative.sh"
~~~

Set EVIDENCE_DIR to the directory containing this replay file. The negative test writes its deliberately failing binary and log under build/host/miami-stereo. Run the normal dispatched test again to restore a passing binary.
