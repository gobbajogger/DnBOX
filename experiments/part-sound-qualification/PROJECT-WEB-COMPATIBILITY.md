# Current project exports vs. the deployed Projects parser

A real firmware C export, not a relabeled format header, generated stream 13 and 16 with sound revision 13. Stream 16 includes valid automation parked for the other engine and emitted files in both project formats.

The deployed [Projects parser](https://github.com/gobbajogger/DnBOX-nightly/blob/7fcf17b953a34952f3c677dafd515744cd2dc289/projects/index.html#L1172) rejected all three selected exports at its schema check: format 1/stream 13, format 1/stream 16 and format 2/stream 16. Current source accepted all three and still rejected unsupported stream 17. This is a browser Projects backup compatibility gap, not a raw installer sample-slot backup failure.

The expanded firmware C fixture passed 271 checks. The current JavaScript companion suite passed 72 tests, including real parked-automation archive and restore byte equality and downgrade refusal before any sample or commit writes. These are host/companion results, not a live browser or device qualification. No production website, installer or device was changed.
