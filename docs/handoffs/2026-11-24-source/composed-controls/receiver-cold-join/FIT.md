# Exact cold-data, receiver sharing, UI memory follow-up
Base: isolated clone of all-controls-remote-join/cold-data-fit/tree with identical guarded table profile. Patched typed-receivers, typed-owner-sharing and typed-carrier-sharing in order at zero fuzz. Receiver file was c14ab1e6… before and ae1d3faf… after. Reused the existing guarded tool/link script, modifying only the shell runner's paths relative to isolated location.
 
| Auditable native link variant | Elite XIP overflow | Pro XIP overflow | POOL free (both) |
| --- | ---: | ---: | ---: |
| Input cold-data | 25,248 B | 25,240 B | 8,224 B |
| Cold-data plus all three receiver patches | 14,496 B | 14,488 B | 8,224 B |
| Add Undo-span then compact UI title patches | 14,164 B | 14,152 B | 8,620 B |
 
Both paired phases map .lab_fdata 0xef90, .lab_slot3 0x7388, .lab_fittab 0xf73. The last pair maps .pool 0x51e54, which is 428 B above the required 8192 B reserve. Ordinary BSS ends at 0x01c1ff54, 172 B before 0x01c20000 in each map. These are rejected link measurements (hard XIP), not runnable test firmware or sufficient package/companion verification.
 
Exact inputs and outputs in FINGERPRINTS.sha256. All work in receiver-cold-join; no edits to the original tree, devices, Git, installation, runtime probes, or publishing.
