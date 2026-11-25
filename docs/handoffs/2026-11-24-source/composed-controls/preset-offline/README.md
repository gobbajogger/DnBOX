# Exact preset packing trials
These are offline experiments, not device formats, firmware assets, or tools
for flashing a unit. No presets belong in the application partition.
 
In this directory, place the unmodified `source/` helper folder from the
cab25d44 native checkpoint next to the scripts, including its private catalogue.
Keep the two original preset ZIPs and the helper catalogue out of Git.
Python 3 with `zstandard` is required. Supply the two original ZIPs:
 
```sh
python shared_page_trial.py /path/to/J6presets.zip '/path/to/Synth one.zip'
python escape_lanes.py /path/to/J6presets.zip '/path/to/Synth one.zip' --journal
python escape_lanes.py /path/to/J6presets.zip '/path/to/Synth one.zip' --journal --dense-head
```
 
The successive lossless images used 46, 45, and 44 data sectors. The best
passed equality of all 1,546 original records (including 221,013 parameter
words, exact stable IDs, positions and name bytes), an independently changed
bank with a previously unknown value and missing raw position, bank removal
without touching other banks, and 4,097 simulated directory-write interruptions.
This is 176 KiB of data (184 KiB including both directories) versus 112 KiB
of usable pool. Replacement used 196 KiB transient data space in this scenario.
There is no bounded on-device decoder, physical NOR migration or transactional
garbage collection for this experiment. It is not a usable Elite/Pro build.

The same dense bank trial with `DNBOX_TRIAL_HISTORY=4096` used 178,948 B
before alignment, only 10 B more than the 64 KiB trial, and still used 44
data sectors. The history parameter alone does not bound the decoder's total RAM.
`COLUMN-TRIAL.md` describes a smaller alternative that encodes the full library
as one stream and therefore cannot independently replace or remove a bank.
