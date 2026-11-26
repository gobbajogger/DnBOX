#include "juno6.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
static juno6_t j, other;
static float l[256],r[256];
static unsigned pos;
static int16_t mode[1024];
static unsigned cursor,resets;
static uint32_t pcm;
static float gains[1024];
void diagnostic_mode(unsigned k,unsigned active,float g,const float *left,const float *right)
{
    assert(cursor<1024);mode[cursor]=(int16_t)active;gains[cursor++]=g;
    union {float f;uint32_t u;} x={.f=left[k]};pcm=(pcm^x.u)*16777619u;
    x.f=right[k];pcm=(pcm^x.u)*16777619u;
}
void diagnostic_reset(const dnbx_part *s)
{
   for(unsigned i=0;i<DNBX_VOICES;i++)for(unsigned l=0;l<2;l++){
       const dnbx_layer_voice *v=&s->voice[i].layer[l];
       assert(v->filter_first==0.f&&v->filter_second==0.f&&v->ota_tail[0]==0.f&&v->ota_tail[1]==0.f);
   }
   ++resets;
}
static uint32_t digest_unarmed(void)
{
    juno6_init(&j,44100);
    assert(dnbx_note_on(&j.core,57,87)>=0);
    uint32_t d=2166136261u;
    for(unsigned q=0;q<32;q++){
        if(q==8)juno6_sj6_set_ota(&j,1);
        if(q==22)juno6_sj6_set_ota(&j,0);
        if(q==23)dnbx_note_off(&j.core,57);
        assert(juno6_render_stereo(&j,l,r,128));
        for(unsigned k=0;k<128;k++){
            union{float f;uint32_t u;}w={l[k]};d=(d^w.u)*16777619u;w.f=r[k];d=(d^w.u)*16777619u;
        }
    }
    return d;
}
#if DNBX_HOST_SJ6_FILTER_MODE
static unsigned test(unsigned w,unsigned rate,unsigned irregular,unsigned stressful)
{
    const unsigned pieces[]={13,37,5,91,29};
    const unsigned regular[]={16};
    const unsigned frames[]={0,64,96,640};
    const unsigned keys[]={48,53,57,61,65,69,72};
    const unsigned counts[]={1,2,3,2},current[][4]={{1,0,0,0},{1,0,0,0},{1,0,1,0},{1,0,0,0}};
    const unsigned when[][4]={{0,0,0,0},{0,64,0,0},{0,64,96,0},{0,640,0,0}};
    juno6_init(&j,(float)rate); juno6_init(&other,(float)rate);
    j.core.sj6_mode_test.armed=1;
    j.core.sj6_mode_test.gain=1.f;
    j.core.sj6_mode_test.step=1.f/((float)rate*.005f);
    j.core.sj6_mode_test.b0=0x1.851d9c7caa4aap-1;
    j.core.sj6_mode_test.a1=0x1.7cb2883c57ccap+0;
    j.core.sj6_mode_test.a2=0x1.1b116179f9916p-1;
    assert(dnbx_note_on(&j.core,57,87)>=0);
    assert(dnbx_note_on(&other.core,51,87)>=0);
    assert(j.core.sj6_ota==0&&other.core.sj6_ota==0);
    memset(mode,-1,sizeof mode);unsigned q=0;cursor=0;resets=0;pcm=2166136261u;
    for(pos=0;pos<1024;q++){
        for(unsigned z=0;z<counts[w];z++)
            if(pos==when[w][z])juno6_sj6_set_ota(&j,current[w][z]);
        if(stressful&&pos==64){
            j.core.sj6_mode_test.b0=0x1.cb5f067432c00p-12;
            j.core.sj6_mode_test.a1=-0x1.ea91fe81f06abp+0;
            j.core.sj6_mode_test.a2=0x1.d609ac871aeedp-1;
            for(unsigned n=0;n<7;n++)assert(dnbx_note_on(&j.core,keys[n],87)>=0);
            assert(dnbx_active_voices(&j.core)==6);
        }
        if(stressful&&pos==96){
            j.core.sj6_mode_test.b0=0x1.851d9c7caa4aap-1;
            j.core.sj6_mode_test.a1=0x1.7cb2883c57ccap+0;
            j.core.sj6_mode_test.a2=0x1.1b116179f9916p-1;
            assert(dnbx_note_off(&j.core,72)==DNBX_OK);
        }
        unsigned n=irregular?pieces[q%5]:regular[0];
        for(unsigned z=0;z<counts[w];z++)if(when[w][z]>pos && when[w][z]-pos<n)n=when[w][z]-pos;
        if(stressful)for(unsigned z=1;z<3;z++)if(frames[z]>pos && frames[z]-pos<n)n=frames[z]-pos;
        if(n>1024-pos)n=1024-pos;
        assert(juno6_render_stereo(&j,l,r,n));
        assert(cursor==pos+n);
        for(unsigned k=0;k<n;k++)assert(isfinite(l[k])&&isfinite(r[k]));
        if(pos+n<1024&&n<=DNBX_MAX_FRAMES)assert(juno6_render_stereo(&other,l,r,n));
        assert(other.core.sj6_ota==0);
        pos+=n;
    }
    printf("%u %u %u %u type",rate,irregular,w,stressful);
    for(unsigned k=0;k<1024;k++){
       assert(mode[k]==0||mode[k]==1);
       assert(gains[k]>=0.f&&gains[k]<=1.f);
       if(!k||mode[k]!=mode[k-1])printf(" %u:%d",k,mode[k]);
    }
    uint32_t digest=2166136261u;
    for(unsigned k=0;k<1024;k++){union {float f;uint32_t u;} x={.f=gains[k]};digest=(digest^x.u)*16777619u;}
    printf(" resets=%u gain=%08x pcm=%08x\n",resets,digest,pcm);return sizeof(j.core.sj6_mode_test);
}
#endif
int main(void){
 printf("layout %zu / %zu / %zu offdigest %08x\n",sizeof(dnbx_layer_voice),sizeof(dnbx_voice),sizeof(dnbx_part),digest_unarmed());
#if DNBX_HOST_SJ6_FILTER_MODE
 for(unsigned rate=44100;rate<=48000;rate+=3900)for(unsigned p=0;p<2;p++)for(unsigned w=0;w<4;w++)test(w,rate,p,0);
 test(2,44100,1,1);
#endif
}
