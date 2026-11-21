# Synth One factory render admission

The comparison contract requires a complete original program bound to the independently implemented candidate, unity gain, at most 1 dB absolute level error and at most 3 dB mean spectral error. No threshold can be evaluated for a program that cannot be loaded. The CSV leaves both metrics empty for those rows.

The compiler verified 12 original banks and emitted 144 ordered commands for each of 694 records. The production factory-name, count and cell constructor expose a single named INIT. Its index zero is the DnBOX default and has no binding to the first record of any original bank. All 694 original identities are unbound. Timing, finite-output and A/B results for complete programs are therefore unavailable. The separate oscillator and dry-note measurements concern controlled subsets.

Run the following from this directory against the source checkout. The report uses GCC with link-time optimization and compiles the actual production selector. The output CSV path must be new.

```sh
python3 report.py /tmp/factory-programs.csv
cmp programs.csv /tmp/factory-programs.csv
```

The generated command manifest has SHA-256 `25a9c436f4dc7c37517c77acb632ccdf0753a885ee474e1e8f93c02b0c099a2e`.
