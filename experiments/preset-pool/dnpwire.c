/* SPDX-License-Identifier: GPL-3.0-only */
#include "dnpwire.h"
static uint32_t w32(const uint8_t *p){return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void wput(uint8_t *p,uint32_t n){for(unsigned i=0;i<4;i++){p[i]=(uint8_t)n;n>>=8;}}
void dnp_wire_disconnect(dnpwire *w){if(w&&w->pool){dnp_abort(w->pool);w->token=0;}}
unsigned dnp_wire(dnpwire *w,const uint8_t *q,unsigned n,uint8_t *r,unsigned cap){
    if(!w||!w->pool||!q||!r||n<DNP_WIRE_HEADER||n>DNP_WIRE_MAX||cap<DNP_REPLY_HEADER)return 0;
    /* Request and reply must not overlap. All valid responses echo header. */
    for(unsigned j=0;j<DNP_WIRE_HEADER;j++)r[j]=q[j];
    uint32_t token=w32(q+2);unsigned op=q[1],a=n-DNP_WIRE_HEADER,used=0;const uint8_t *p=q+DNP_WIRE_HEADER;uint8_t *o=r+DNP_REPLY_HEADER;dnpool *pool=w->pool;dnp_info info;int rc=DNP_INVALID;
    if(q[0]!=DNP_WIRE_VERSION||!token)goto out;
    switch(op){
    case DNPW_INFO:
        if(a||cap<DNP_REPLY_HEADER+10)break;
        rc=dnp_get_info(pool,&info);if(!rc){wput(o,info.generation);o[4]=info.reserved;o[5]=info.banks;o[6]=info.bank_sectors;o[7]=info.free_sectors;o[8]=info.contiguous;o[9]=DNP_DATA;used=10;}break;
    case DNPW_LIST:{
        if(a!=5||cap<DNP_REPLY_HEADER+16)break;
        rc=dnp_get_info(pool,&info);if(rc)break;if(info.generation!=w32(p)){rc=DNP_STALE;break;}uint32_t size,crc;rc=dnp_bank_at(pool,p[4],o,&size,&crc);if(!rc){wput(o+8,size);wput(o+12,crc);used=16;}break;}
    case DNPW_READ:{
        if(a!=17)break;unsigned k=p[16]?p[16]:256u;if(cap<DNP_REPLY_HEADER+k)break;
        rc=dnp_bank_read(pool,p+4,w32(p),w32(p+12),o,k);if(!rc)used=k;break;}
    case DNPW_BEGIN:
        if(a!=20)break;if(w->token&&pool->active){rc=DNP_BUSY;break;}
        rc=dnp_begin(pool,p+4,w32(p+12),w32(p+16),w32(p));if(!rc)w->token=token;break;
    case DNPW_WRITE:
        if(a<5||a>260)break;if(token!=w->token||!pool->active){rc=DNP_STALE;break;}
        rc=dnp_write(pool,w32(p),p+4,a-4);if(!pool->active)w->token=0;break;
    case DNPW_END:
        if(a)break;if(token!=w->token||!pool->active){rc=DNP_STALE;break;}
        rc=dnp_end(pool);if(!pool->active)w->token=0;break;
    case DNPW_ABORT:
        if(a)break;if(token!=w->token||!pool->active){rc=DNP_STALE;break;}
        dnp_wire_disconnect(w);rc=0;break;
    case DNPW_REMOVE:
        if(a!=12)break;rc=dnp_remove(pool,p+4,w32(p));break;
    default:break;
    }
out:r[6]=(uint8_t)(-rc);return DNP_REPLY_HEADER+(rc?0:used);
}
