/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DN_KORG_FACTORY_H
#define DN_KORG_FACTORY_H
#include "dnbank.h"
/* Experimental bridge only. Bind validated, immutable banks in a quiescent
 * session. Unbind before ANY pool mutation; maps and names may not outlive it.
 * Legacy factory numbers are stable: record i must carry ID LE64(i+1).
 * This bridge does not authorize deletion or change the saved-project schema. */
int dn_korg_bind(unsigned engine,const dnbank *bank);
/* out is unpublished staging memory; discard on failure. */
int dn_korg_values(unsigned engine,unsigned factory,float *values);
const char *dn_korg_name(unsigned engine,unsigned factory);
#endif
