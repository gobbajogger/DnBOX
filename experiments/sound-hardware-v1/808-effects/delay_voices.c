/* SPDX-License-Identifier: GPL-3.0-only */
#include "sc808_engine.h"
#include "fxbus.h"
#include "drum808.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

extern void sc808_capture(float *, float *, float *);
static drum808_t kit;
static fxbus_t verb;
static int16_t ring[FX_808_DELAY_FRAMES];

static void pot(sc808_engine_t *e, const char *key, int value)
{
    char temp[16];
    snprintf(temp,sizeof temp,"%d",value);
    assert(sc808_set_param(e,key,temp));
}

int main(int argc, char **argv)
{
    assert(argc == 12);
    int voice=atoi(argv[1]), rv=atoi(argv[2]), mode=atoi(argv[3]), glue=atoi(argv[4]);
    sc808_engine_t *ref=sc808_create(44100);
    assert(ref);
    drum808_init(&kit);
    drum808_set(&kit,D8_KIT,0,127);
    drum808_set(&kit,D8_KIT,D8_KIT_RV_BUS,1);
    drum808_set(&kit,D8_KIT,D8_KIT_DL_BUS,1);
    drum808_set(&kit,D8_KIT,D8_KIT_DIST,mode);
    drum808_set(&kit,D8_KIT,D8_KIT_DRIVE,64);
    drum808_set(&kit,D8_KIT,D8_KIT_GLUE,glue);
    fxbus_init_808_fx(&verb,ring);
    const char *rkeys[]={"rev_decay","rev_tone","rev_hpf","rev_level"};
    const int rp[]={70,60,55,93};
    for (int i=0;i<4;++i) {pot(ref,rkeys[i],rp[i]);fxbus_set(&verb,FX_RV_DECAY+i,rp[i]);}
    const char *keys[]={"dly_time","dly_fdbk","dly_tone","dly_level","dly_hpf"};
    for (int i=0;i<5;++i) {
        int value=atoi(argv[5+i]);
        pot(ref,keys[i],value);
        fxbus_set(&verb,FX_DL_TIME+i,value);
    }
    int bpm=atoi(argv[10]);
    pot(ref,"dly_bpm",bpm);
    fxbus_set_bpm(&verb,(float)bpm);
    pot(ref,"volume",127);
    pot(ref,"master_dist",mode);
    pot(ref,"master_drive",64);
    pot(ref,"comp",glue);
    for (int v=0;v<SC808_NUM_VOICES;++v) {
        if (v != SC808_BD && (voice==-1 || voice==v)) {
            char k[32];
            snprintf(k,sizeof k,"%s_rev",sc808_voice_id(v));pot(ref,k,rv?127:0);
            snprintf(k,sizeof k,"%s_dly",sc808_voice_id(v));pot(ref,k,127);
        }
    }
    FILE *file=fopen(argv[11],"wbx");
    assert(file);
    for (int pos=0;pos<88200;pos+=128) {
        int n=88200-pos<128 ? 88200-pos : 128;
        float x[128],rv[128],dl[128],original[128],left[128],right[128];
        if (pos==0||pos==3072||pos==12288||pos==20480)
            for(int v=0;v<SC808_NUM_VOICES;++v) if(voice==-1||voice==v)sc808_trigger(ref,v,118);
        sc808_capture(x,rv,dl);
        sc808_render(ref,original,n);
        sc808_capture(0,0,0);
        for(int i=0;i<n;++i)left[i]=right[i]=x[i];
        fxbus_808_reverb(&verb,rv,left,right,n,1);
        fxbus_808_delay(&verb,dl,left,right,n,1);
        for(int i=0;i<n;++i)rv[i]=dl[i]=0;
        drum808_kit_process(&kit,left,right,rv,dl,n);
        for(int i=0;i<n;++i) {
            float words[]={x[i],original[i],left[i],right[i]};
            assert(fwrite(words,sizeof words,1,file)==1);
        }
    }
    sc808_destroy(ref);
    return fclose(file)!=0;
}
