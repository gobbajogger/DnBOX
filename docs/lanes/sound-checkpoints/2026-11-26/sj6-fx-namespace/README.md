# Crusher namespace and source law, not firmware admission

Build9 ARM64 SHA 6d163d79…, genuine aumu/j6ct/AuPr parameter tree and
separately loaded aufx/dist/appl leaf are distinct namespaces. The checker
verifies actual active parameter IDs/names, Objective-C selector references,
pow and objc_msgSend imports, and independent direct leaf sender/readback.

| Namespace | Control | Confirmed route |
|---|---|---|
| J6 #107 neoncrushEnable | gate | >0 sets byte, wet .3 / Booster 1.3; otherwise wet 0 / Booster 1; not a graph detach |
| J6 #108 neoncrushAmount | wet | cache Float32 raw; convert to Double; setMix(pow(raw,1.5)); Booster untouched |
| J6 #109 neoncrushBitRate | cadence | setDecimation(.0606*pow(raw,1.23)); setRounding(0) |
| Apple aufx/dist/appl #3 | Decimation % | public native percent; sender bypasses the J6 law |
| Apple #4 / #5 / #15 | Rounding / Decimation Mix / Wet-Dry Mix | independent public AU addresses |

Source caller contains Enable, BitRate, Amount edges in that code order
(0x1002737c8, 0x10027387c, 0x100273970). This does not establish complete
callback history in an installed render. Direct Apple sender verifies the
leaf independently; it does not prove effective live installed #108 or
#109, or the AKDecimator normalized-to-Apple-percent hop. That requires
same installed node physical before/after capture of separate edits.
Legacy compact #108 cadence is not the Build9 wet mapping.

fx.h corrects earlier diagnostic's linear #108 model. It uses Double pow
as a host reference, not suitable as-is for native firmware. Dry predicate
unchanged. No new controls, production code, or push.
