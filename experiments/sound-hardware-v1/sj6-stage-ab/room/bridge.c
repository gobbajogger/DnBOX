#include <string.h>
#include <math.h>
#include <stdint.h>
#include "dsp/juno6.h"
#include "app/original_runtime.h"
#include "dsp/sj6_native_room.inc"
float render_room(unsigned type,unsigned size,unsigned enabled,float *l,float *r,unsigned n) {
    original_runtime_part p={0};
    p.kind=ORIGINAL_SOUND_JUNO;p.status=ORIGINAL_RUNTIME_OK;
    uint8_t code[SJ6_ROOM_COUNT]={
      [SJ6_ROOM_ENABLE]=enabled,[SJ6_ROOM_TYPE]=type,
      [SJ6_ROOM_SIZE]=size,[SJ6_ROOM_MIX]=127,[SJ6_ROOM_LOCUT]=0
    };
    original_runtime_sj6_room(&p,code);sj6_room_process(&p.dsp.juno.room,l,r,n);
    return p.dsp.juno.room.fb;
}
float render_room_close(unsigned close,float *l,float *r,unsigned n) {
    original_runtime_part p={0};
    p.kind=ORIGINAL_SOUND_JUNO;p.status=ORIGINAL_RUNTIME_OK;
    uint8_t code[SJ6_ROOM_COUNT]={
      [SJ6_ROOM_ENABLE]=1,[SJ6_ROOM_TYPE]=1,
      [SJ6_ROOM_SIZE]=80,[SJ6_ROOM_MIX]=127,[SJ6_ROOM_LOCUT]=0
    };
    if(close>n)return -1.f;
    original_runtime_sj6_room(&p,code);
    sj6_room_process(&p.dsp.juno.room,l,r,close);
    code[SJ6_ROOM_ENABLE]=0;
    original_runtime_sj6_room(&p,code);
    sj6_room_process(&p.dsp.juno.room,l+close,r+close,n-close);
    return p.dsp.juno.room.fb;
}
int render_room_toggle(unsigned toggle,float *l,float *r,unsigned n) {
    static const unsigned boundary[]={0,6143,9971,14211,16607,24576};
    original_runtime_part p={0};p.kind=ORIGINAL_SOUND_JUNO;p.status=ORIGINAL_RUNTIME_OK;
    uint8_t code[SJ6_ROOM_COUNT]={
      [SJ6_ROOM_ENABLE]=1,[SJ6_ROOM_TYPE]=1,
      [SJ6_ROOM_SIZE]=80,[SJ6_ROOM_MIX]=127,[SJ6_ROOM_LOCUT]=0
    };
    if(toggle>1u || n!=boundary[5])return 0;
    for(unsigned i=0;i<5;i++){
      code[SJ6_ROOM_ENABLE]=(uint8_t)(toggle?!(i&1u):1u);
      original_runtime_sj6_room(&p,code);
      unsigned j=boundary[i];
      sj6_room_process(&p.dsp.juno.room,l+j,r+j,boundary[i+1]-j);
    }
    return 1;
}
