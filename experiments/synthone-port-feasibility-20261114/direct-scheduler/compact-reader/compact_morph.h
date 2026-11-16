// Isolated reader backend bridge; original interpolation/phase body is unchanged.
#ifndef S1C_MORPH_H
#define S1C_MORPH_H
#include "soundpipe.h"
#include "soundpipeextension.h"
#include "compact.h"
/* PRECONDITIONS, not implemented hostile-object admission:
 * sp/osc and all reachable metadata must already be initialized, aligned,
 * mapped, live and readable under the accepted original 4096-sample,
 * 4-waveform/13-band/24-bit-phase descriptor contract. Frequencies/phase
 * increments and interpolation inputs must remain in that admitted domain.
 * osc->tbl points to 52 live descriptor pointers with valid original table
 * metadata; descriptor payload pointers may be NULL. fbl is 13 live original
 * increasing finite band values. Binding was fully admitted with s1c_bind.
 * osc and out are writable and mutually disjoint. BOTH writable spans must be
 * disjoint from sp, binding, entire packed payload, table-pointer array,
 * descriptors and bands; readonly spans may share. This bridge DOES NOT check
 * all these aliases, metadata spans, mappedness or lifetime itself.
 * All readonly metadata/payload is immutable for the call and no state races.
 * in is unused, as in the original primitive.
 *
 * SP_NOT_OK preserves output until all four reads succeed. It does NOT promise
 * full oscillator rollback: wtpos/inc may already change before a read fails.
 * No full-Scheduler-owner protection or metadata-only Scheduler admission is
 * implied; those require a separate admission/span contract and exact tests.
 */
int s1c_runtime_morph_compute(sp_data*,sp_oscmorph2d*,SPFLOAT*,SPFLOAT*,const s1c_binding*);
#endif
