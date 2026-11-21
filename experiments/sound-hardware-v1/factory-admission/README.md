# Synth One factory render admission

The comparison contract requires a complete original program bound to the independently implemented candidate, unity gain, at most 1 dB absolute level error and at most 3 dB mean spectral error. No threshold can be evaluated for a program that cannot be loaded. The CSV leaves both metrics empty for those rows.

The compiler verified 12 original banks and emitted 144 ordered commands for each of 694 records. The production factory-name, count and cell constructor expose a single named INIT. Its index zero is the DnBOX default and has no binding to the first record of any original bank. All 694 original identities are unbound. Timing, finite-output and A/B results for complete programs are therefore unavailable. The separate oscillator and dry-note measurements concern controlled subsets.

Run the following from this directory against the source checkout. The report uses GCC with link-time optimization and compiles the actual production selector. The output CSV path must be new.

```sh
python3 report.py /tmp/factory-programs.csv
cmp programs.csv /tmp/factory-programs.csv
```

The generated command manifest has SHA-256 `25a9c436f4dc7c37517c77acb632ccdf0753a885ee474e1e8f93c02b0c099a2e`.

The [historical 694-row oscillator comparison](historical-oscillator-v3.json.gz) replayed both sides through the same original graph. It reported 492 bit-identical programs, 202 with differing words, maximum RMS error 0.000633 dB and maximum spectral error 0.048300 dB. Its per-program comparisons qualify the oscillator substitution in that graph. The production complete-program admission count remains 0/694.

## Factory selection in the reference comparison

The [C reader](packed/bank.c) selects all 694 programs by original bank name and stored position. Bonus starts at position 40. JEC's JSON array order differs from the stored positions of all 88 programs. Program UIDs identify only 670 distinct values, or 691 when paired with bank. All 694 bank/position pairs are unique. These addresses preserve the distinct records with duplicate UIDs.

The [independent mapping audit](audit_mapping.py) reads the original Swift save getters and step getters, takes numeric IDs from the original enum, and compiles the original kernel's binary32 defaults in array order. Across the 12 authenticated bank JSONs it verified all 99,936 pre-setter parameter IDs and physical words, including 2,105 default substitutions. The check reads the generator's actual binary output. Three negative controls changing the first ID, a value bit and the last value bit each failed. The two additional original headers under `original/` come from AudioKit/AudioKitSynthOne at `6466a3715c96b7ecf1dd255a218cbd571408a314`, under the [Synth One MIT license](../../../assets/factory_banks/synthone/LICENSE). [Counts by bank and field](reverse-mapping.json)

The [Linux playback test](packed/replay.py) selected each program from a read-only, column-dictionary bank with 73 encoded bytes per program plus shared dictionaries, addresses, and names. Its candidate reads its complete ordered stream through the C reader. All 66,624,000 stereo frames matched the original reference hashes, including all 694 names and both channels. Each program ran for 96,000 frames at 48 kHz. It uses the existing common original effects graph. In a separate PI32v2 compile the tables and strings occupied 102,976 bytes of read-only data and the reader used 206 bytes of text before linking. The ordinary stock INIT links have 6,156/6,164 bytes free for Elite/Pro. Production program selection and independent production effects remain open. [Playback receipts](packed/result.json)

Reproduce from the repository root with the previously documented restored test inputs. Use fresh output directories.

```sh
python3 experiments/sound-hardware-v1/factory-admission/audit_mapping.py
python3 tests/host/run_synthone_factory.py --source-build /tmp/s1-original-inputs --output /tmp/s1-compare --build-only
python3 experiments/sound-hardware-v1/factory-admission/packed/check.py /tmp/s1-compare/commands
python3 experiments/sound-hardware-v1/factory-admission/packed/replay.py /tmp/s1-compare /tmp/s1-packed
```
