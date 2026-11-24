/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBANK_H
#define DNBANK_H
#include <stdint.h>
#include <stddef.h>
typedef struct {
 const uint8_t *pack,*sector_map;
 uint16_t off[7],programs,ng,nf,nl;
 uint8_t engine,gb;
} dnbank;
typedef void (*dnbank_emit)(void *,unsigned,uint32_t);
int dnbank_open(dnbank *,const uint8_t *,size_t);
int dnbank_open_mapped(dnbank *,const uint8_t *,size_t,const uint8_t *,unsigned,size_t);
int dnbank_read(const dnbank *,unsigned,char name[48],dnbank_emit,void *);
int dnbank_validate(const dnbank *);
int dnbank_preset_id(const dnbank *,unsigned,uint8_t id[8]);
#endif
