/* SPDX-License-Identifier: GPL-3.0-only */
/* Tests the actual existing ldr_session over the simulated NOR and transport. */
#include <assert.h>
#define main stock_harness_main
#include "loader_harness.inc"
#undef main

static uint8_t saved_head[0x4000], saved_slots1_2[T_FD_LO-0xC2000], saved_slot3[0x8000];
static uint8_t *replay, *original;
static uint32_t fdo, fdl, app, interruption;

static void update_type(uint8_t *lg, unsigned type, unsigned rename, int payload_changed, uint32_t payload_size)
{
    uint8_t head[64];
    memcpy(head,lg,64);ota_jl_enc(head,64);
    unsigned n=ota_rd16(head+8), seen=0;
    assert(n>0 && n<(0x400-64)/0x50);
    for(unsigned i=0;i<n;i++) {
        uint8_t *e=lg+64+0x50u*i;
        ota_jl_enc(e,0x50);
        if(ota_rd16(e)==type){
            unsigned off=ota_rd32(e+8),z=ota_rd32(e+12);
            assert(++seen==1 && off+z<=logical_len);
            if(payload_changed){
                assert(z==payload_size);
                ota_wr16(e+4,ota_crc16(lg+off,z,0));
            }
            ota_wr16(e,rename);
        }
        ota_jl_enc(e,0x50);
    }
    assert(seen==1);
    ota_wr16(head+2,ota_crc16(lg+64,0x50u*n,0));
    ota_wr16(head,ota_crc16(head+2,62,0));
    ota_jl_enc(head,64);memcpy(lg,head,64);
}
static void invariant(void) {
    assert(!bad_range && protect_ok());
    assert(!memcmp(nor,saved_head,sizeof saved_head));
    assert(!memcmp(nor+0xB3000,saved_slot3,sizeof saved_slot3));
    assert(!memcmp(nor+0xC2000,saved_slots1_2,sizeof saved_slots1_2));
}
static int run(uint8_t *buf,unsigned fault) {
    logical=buf; interruption=fault; repeat_fd_fault=fault;
    put_record_at(T_REC);
    unsigned count=record_cleared;
    int rc=ldr_session();
    if(fault){
        assert(rc==-10 && repeat_fd_fault==0);
        assert(record_cleared==count && nor[T_REC+6]==0x41);
    }else{
        assert(!rc && record_cleared==count+1 && nor[T_REC+6]==255);
    }
    invariant();
    return rc;
}
static void app_eq(const uint8_t *stream) {
    assert(!memcmp(nor+0x4000,stream+app+0x4000,T_APP_HI-0x4000));
}
int main(int argc,char **argv) {
    assert(argc==2);
    original=load_logical(argv[1],&logical_len);
    assert(original);
    replay=malloc(logical_len);uint8_t *app_only=malloc(logical_len);
    assert(replay && app_only);
    memcpy(replay,original,logical_len);memcpy(app_only,original,logical_len);
    app=flash_off(original);fdo=entry_of(original,103,&fdl);
    assert(app && fdo && fdl==T_FD_HI-T_FD_LO && !memcmp(original+fdo,"S1PK",4));
    assert(flash_off(replay)==app);
    /* Nonbootable, in-memory test stream. The changed app byte is only to
       force an APP-sector update before FDATA, not to create a candidate app. */
    replay[app+0x8823]^=1u;
    update_type(replay,0,0,0,0);
    for(unsigned i=0;i<fdl;i++) replay[fdo+i]=(uint8_t)(73u+i*19u);
    memcpy(replay+fdo,"D9RD",4);
    update_type(replay,103,103,1,fdl);
    update_type(app_only,103,999,0,0);

    unsigned cases=0;
    for(unsigned i=0;i<15;i++){
        memset(nor,255,sizeof nor);memcpy(nor,original+app,T_APP_HI);
        memcpy(nor+T_FD_LO,original+fdo,fdl);
        for(unsigned k=0;k<sizeof saved_slot3;k++)nor[0xB3000+k]=(uint8_t)(155u+k*11u);
        for(unsigned k=0;k<sizeof saved_slots1_2;k++)nor[0xC2000+k]=(uint8_t)(11u+k*29u);
        memcpy(saved_slot3,nor+0xB3000,sizeof saved_slot3);
        memcpy(saved_slots1_2,nor+0xC2000,sizeof saved_slots1_2);
        memcpy(saved_head,nor,sizeof saved_head);
        protect_fill();
        const unsigned fault=T_FD_LO+i*4096u+256u;
        assert(run(replay,fault)==-10);app_eq(replay);
        assert(memcmp(nor+T_FD_LO,replay+fdo,fdl));
        /* Existing active record is retained; continue the same staged bytes. */
        unsigned clear=record_cleared;
        assert(!ldr_session());assert(record_cleared==clear+1);
        app_eq(replay);assert(!memcmp(nor+T_FD_LO,replay+fdo,fdl));invariant();
        assert(!run(app_only,0));app_eq(original);
        assert(!memcmp(nor+T_FD_LO,replay+fdo,fdl)); /* APP-only cannot roll back */
        assert(memcmp(nor+T_FD_LO,original+fdo,fdl));
        assert(run(original,fault)==-10);app_eq(original);
        clear=record_cleared;
        assert(!ldr_session());assert(record_cleared==clear+1);
        app_eq(original);assert(!memcmp(nor+T_FD_LO,original+fdo,fdl));invariant();
        erases=0; assert(!ldr_session() && erases==0);
        cases++;
    }
    free(replay);free(app_only);free(original);
    printf("PASS %u paired forward and %u restore interruption cases; real loader accepts exact full original APP+FDATA; APP-only leaves FDATA unchanged\n",cases,cases);
    return 0;
}
