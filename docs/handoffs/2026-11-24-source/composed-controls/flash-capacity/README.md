# Preset capacity: both sides of the ledger
Offline source checkpoint only. Do not build these trials into a release or use them to write a unit.

## Current physical budget
Both `all-controls-fit-s1-wheel-noise` links used the same complete-controls source. The APP
cap is 552,672 B; Elite uses 552,408 B and Pro 552,400 B. The diagnostic
companions remain occupied; these are not free space and are not production slot migrations.

Slot 2's original range `[0xD6000,0xF4000)` is 120 KiB. The high 60 KiB, including the
old experimental pool's A and B directory sectors, contains D9RD in these
diagnostics (61,434 B including its header on both models). Rehousing two fully
separate directory sectors in the lower 60 KiB gives an **unimplemented 52 KiB**
ceiling before BREAK extents and update work space. The two existing slot-2 default
loops require 50,061 B with their slot header, or all 13 data sectors. The diagnostics
also occupy 32,764 B of the 32 KiB user Slot 3. Preserve projects, user slots,
calibration, BLE, OTA, rollback and the atomic backup guarantee.

## Measured reclaim without re-encoding any sound
`stream_storage.py` operates on aggregate link data or existing pinned default-BREAK
inputs. The canonical codes include their per-asset models, restart index, headers
and raw block fallbacks. Reconstruction compares every original ADPCM byte.

- 909 crash/ride/hi-hat: at **256-byte** restarts, byte codes plus already-resident
  non-sample data used **57,120 B**, vs 61,434 B now. That fits in 14 sectors instead
  of 15 with just 224 B spare in those sectors. Both board binaries returned
  identical measurements. The 512-byte restart case used 56,859 B for the same
  sectors. This is 4 KiB potentially adjacent to the writable range if FDATA's
  lower boundary is changed with a matching package, the exact checked decoder
  and proper migration.
- Default slot 2: 256-byte restarts used **46,037 B** including the slot header,
  vs 50,061 B now, or **12 sectors** instead of 13. Slot 1 also saved one sector,
  but that is not credited to the Slot 2 shared pool and is not permission to alter
  Slot 1's capacity.
- Thus at best these two independent byte savings provide **8 KiB for preset
  extents** in Slot 2 with the two defaults retained and separate A/B directories,
  before allocator replacement reserve. This is a geometric offline result, not
  deployable or qualified. It is not a claim that all 176 KiB of bank data can fit.
- A separate simulated design stages the spare directory in the existing Slot 2
  backup under a strictly exclusive transaction. It survived prefix interruptions
  in the model only. Do **not** count its 4 KiB in the supported design without
  production boot recovery and installer/rollback qualification. Prefer keeping
  independent directory and sample backups unless that extra sector is essential.

No decoder has been admitted to the hard real-time path. The 909 path already has
dropouts in the combined native heavy scene. The byte savings must not be traded for
audio stalls: qualify native decoder footprint, worst restart and reverse/seek
cost, writes that tear, and dual-model strict playback before any promotion.
 
Reproduce on an exact diagnostic link directory containing `lab.map` and `lab_fdata.bin`:
 
```sh
python stream_storage.py /path/to/elite/linked/files
python stream_storage.py /path/to/pro/linked/files
python stream_storage.py --default-breaks /path/to/firmware/assets/default-breaks
python shadow_model.py
```
 
No original WAVs/preset data or binaries are produced or committed.
