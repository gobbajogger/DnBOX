/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNPWIRE_H
#define DNPWIRE_H
#include "dnpool.h"
/* Proposed ED_PRESET_POOL subprotocol, carried by existing SysEx pack7.
 * All integers LE; raw binary frame before/after pack7. Serialized foreground.
 * A nonzero transaction ID identifies an upload and is echoed in every reply.
 * Transport must acquire exclusive maintenance before accepting mutations. */
#define DNP_WIRE_VERSION 1u
#define DNP_WIRE_MAX 272u
#define DNP_WIRE_HEADER 6u
#define DNP_REPLY_HEADER 7u
/* Reconnect after timeout: query INFO + LIST before assuming END failed.
 * Wire state is RAM only. Host must never replay a commit blindly. */
enum { DNPW_INFO=0,DNPW_LIST=1,DNPW_READ=2,DNPW_BEGIN=3,DNPW_WRITE=4,DNPW_END=5,DNPW_ABORT=6,DNPW_REMOVE=7 };
typedef struct {dnpool *pool;uint32_t token;} dnpwire;
/* No sample reservation or raw flash writes via this protocol. Sample upload
 * calls reserve/release from its existing trusted device-side path. */
unsigned dnp_wire(dnpwire *,const uint8_t *req,unsigned n,uint8_t *reply,unsigned cap);
void dnp_wire_disconnect(dnpwire *);
#endif
