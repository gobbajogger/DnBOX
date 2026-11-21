/* SPDX-License-Identifier: GPL-3.0-only */
#include "fxbus.h"
#include "fxbus_delay_store.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { SHARED = 79200, BLOCK = 128, FRAMES = 44100 * 6 };
static fxbus_t raw_bus, packed_bus, private_bus, isolated_bus;
static int16_t raw_ring[SHARED], private_ring[FX_808_DELAY_FRAMES], isolated_ring[FX_808_DELAY_FRAMES];
static struct {
    unsigned before;
    int16_t data[(FX_DELAY_BYTES(SHARED)+1)/2];
    unsigned after;
} packed_ring;
static float zero[BLOCK];

static void shared_settings(fxbus_t *bus, int mode)
{
    fxbus_set(bus,FX_VOLUME,127);
    fxbus_set(bus,FX_DL_TYPE,mode&1);
    fxbus_set(bus,FX_DL_PING,(mode>>1)&1);
    fxbus_set(bus,FX_DL_WEAR,127);
    fxbus_set(bus,FX_DL_FDBK,127);
    fxbus_set(bus,FX_DL_LEVEL,127);
    fxbus_set(bus,FX_DL_TIME,mode&4 ? 0 : 12);
    fxbus_set(bus,FX_DL_TONE,mode&8 ? 0 : 127);
    fxbus_set_bpm(bus,mode&4 ? 240.f : 45.f);
}

static void private_settings(fxbus_t *bus, int mode)
{
    fxbus_set(bus,FX_DL_LEVEL,127);
    fxbus_set(bus,FX_DL_TONE,(mode&4)?0:127);
    fxbus_set(bus,FX_DL_FDBK,127);
    fxbus_set(bus,FX_DL_TIME,(mode&4)?0:12);
    fxbus_set_bpm(bus,(mode&4)?240.f:45.f);
}

static float noise(unsigned *seed)
{
    *seed ^= *seed << 13;
    *seed ^= *seed >> 17;
    *seed ^= *seed << 5;
    return (float)(int32_t)*seed*(1.f/2147483648.f);
}

/* Including a partial last block and revisiting a partially updated block
 * checks both representable extremes during the pack/decode round trip. */
static void packed_extremes(void)
{
    enum { LEN=197 };
    int16_t store[(FX_DELAY_BYTES(LEN)+1)/2]={0};
    store[0]=store[1]=-1;
    for(int i=0;i<LEN;++i) fx10_write(store,LEN,i,(i&1)?32767:-32767);
    for(int k=0;k<3;++k) {
        for(int i=0;i<LEN;++i) assert(fx10_read(store,LEN,i)==((i&1)?32767:-32767));
        fx10_write(store,LEN,k,(k&1)?32767:-32767);
        fx10_write(store,LEN,LEN-1,-32767);
    }
}

int main(int argc,char **argv)
{
    assert(argc==3);
    packed_extremes();
    int mode=atoi(argv[1]);
    FILE *out=fopen(argv[2],"wbx");
    assert(out);
    fxbus_init(&raw_bus,raw_ring,SHARED);
    packed_ring.before=packed_ring.after=0xa8910d30u;
    fxbus_init_packed(&packed_bus,packed_ring.data,SHARED);
    fxbus_init_808_fx(&private_bus,private_ring);
    fxbus_init_808_fx(&isolated_bus,isolated_ring);
    shared_settings(&raw_bus,mode);shared_settings(&packed_bus,mode);
    private_settings(&private_bus,mode);private_settings(&isolated_bus,mode);
    unsigned seed=0x1ab79cdu,seed8=0x808u;
    int tail9=0,tail8=0;
    for (int pos=0;pos<FRAMES;pos+=BLOCK) {
        int n=FRAMES-pos<BLOCK?FRAMES-pos:BLOCK;
        float source9[BLOCK],source8[BLOCK],a[BLOCK],b[BLOCK],c[BLOCK],d[BLOCK];
        float left[BLOCK]={0},right[BLOCK]={0},iso_l[BLOCK]={0},iso_r[BLOCK]={0};
        for(int i=0;i<n;++i) {
            int t=pos+i;
            float n9=noise(&seed),n8=noise(&seed8);
            int hot=(t<44100*2)&&(t%5873<1400);
            source9[i]=hot ? 1.9f*n9+.45f*sinf((float)t*.031f) : 0;
            source8[i]=hot ? .63f*n8+.81f*sinf((float)t*.013f) : 0;
        }
        if(pos==65536) {
            int div=mode&4?12:0;
            fxbus_set(&raw_bus,FX_DL_TIME,div);fxbus_set(&packed_bus,FX_DL_TIME,div);
            fxbus_set(&private_bus,FX_DL_TIME,div);fxbus_set(&isolated_bus,FX_DL_TIME,div);
        }
        int closed=pos>=44100*2;
        fxbus_808_delay(&isolated_bus,closed?0:source8,iso_l,iso_r,n,1);
        fxbus_process_st(&raw_bus,zero,zero,zero,source9,a,b,n);
        fxbus_process_st(&packed_bus,zero,zero,zero,source9,c,d,n);
        if (pos == 0) assert(!fxbus_can_stream_idle_delay(&packed_bus,zero,zero,(unsigned)n));
        fxbus_808_delay(&private_bus,closed?0:source8,left,right,n,1);
        assert(!memcmp(left,iso_l,(unsigned)n*sizeof(float)) && !memcmp(right,iso_r,(unsigned)n*sizeof(float)));
        for(int i=0;i<n;i++) {
            float data[]={a[i],b[i],c[i],d[i]};
            for(unsigned j=0;j<4;j++)assert(isfinite(data[j]));
            if(closed){tail8|=left[i]!=0;tail9|=c[i]!=0||d[i]!=0;}
            assert(fwrite(data,sizeof data,1,out)==1);
        }
    }
    assert(!memcmp(private_ring,isolated_ring,sizeof private_ring));
    assert(tail8 && tail9);
    assert(packed_ring.before==0xa8910d30u && packed_ring.after==0xa8910d30u);
    return fclose(out)!=0;
}

