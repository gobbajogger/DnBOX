# SuperJ6 J60 Post-Step: native guarded link and independent holdouts
Run from the workspace root:
```sh
bash sj6-j60-wet-native/run.sh elite >sj6-j60-wet-native/elite.log 2>&1
bash sj6-j60-wet-native/run.sh pro >sj6-j60-wet-native/pro.log 2>&1
python3 sj6-j60-wet-native/verify.py
bash sj6-j60-wet-native/fixture.sh
```
The donor is the immutable sj6-hp-exact-exp-fit/kept-{elite,pro}-1 guarded source and build snapshots. The exact off-arm ELF reproduces both retained donor hashes. On-arm swap is by explicit macro DNBX_EXPERIMENT_J60_POST only. The paired real PI32v2 ELF/read-back comparator checks equal section metadata/pool symbols and exactly one F32 code literal change at 0x0206651c inside the actual linked juno6_j60_process. The only other differing bytes are linker-generated common image IDs in six header locations. Runtime selector original_runtime.c uses this function if the typed J60 owner is selected, and the previous companion Raw mode-I typed run proved actual invocation. Four link fixtures intentionally end at never-install ELF and no firmware install/package.
 
Independent exact signed known-input audio fixture enters original ARM64 at 96k in modes II and Both, preserving both stereo channels and start/drain. Compact comparison compiles the same paired source under host for isolated middle, not another native stage timing measurement. Both modes improve attack/held/tail. Bypass preserves all 38,400 source words per channel on both macro arms and the separate stopped original. Sound for the whole terminal/production path and target playback are not qualified, even though this particular code substitution adds exactly zero APP and zero pool memory in the measured donor.
