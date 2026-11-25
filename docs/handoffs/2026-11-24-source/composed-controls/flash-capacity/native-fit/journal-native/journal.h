#ifndef DNBOX_VMJ_SPECIMEN_H
#define DNBOX_VMJ_SPECIMEN_H
#include <stdint.h>
enum {
    VMJ_SET_A=0x9d000, VMJ_SET_B=0x9e000,
    VMJ_PANEL_A=0xa5000, VMJ_PANEL_B=0xa6000, VMJ_OWNER=0xaf000,
    VMJ_PAGE=256, VMJ_TEMP=512, VMJ_HEADER=32, VMJ_BODY=224
};
typedef struct {
    uint8_t binding[16], len[4], data[160];
    uint32_t generation, seq[4];
} vmj_snapshot;
typedef int (*vmj_read)(void* ctx,uint32_t off,void *dst,unsigned n);
/* read callback: 0 ok, nonzero I/O. 0 legacy, 1 journal, -1 invalid, -2 I/O.
 * 32-byte header and 224-byte reusable scratch; caller supplies snapshot. */
int vmj_select(vmj_read,void*,vmj_snapshot*);
int vmj_encode(const vmj_snapshot*,uint8_t h[VMJ_HEADER],uint8_t body[VMJ_BODY]);
#endif
