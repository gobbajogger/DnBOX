/* Diagnostic: raw IMA vs packed IMA as exposed by the same renderer. */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "dsp/drum909.c"

static void check(int x) { if (!x) abort(); }
static drum909_t d;
static float buf[4][256];

static void shot(void)
{
    memset(buf,0,sizeof buf);
    drum909_render_st(&d,buf[0],buf[1],buf[2],buf[3],256);
    check(fwrite(buf,1,sizeof buf,stdout)==sizeof buf);
}

int main(void)
{
    unsigned boundary[3]={0,0,0};
    unsigned ended[3]={0,0,0}, sounded[3]={0,0,0};
    for (int fast=0;fast<3;fast++) {
        drum909_init(&d);
        for (int i=0;i<130;i++) drum909_advance_noise(&d,256);
        for (int v=DR_CH;v<=DR_RD;v++) {
            if(fast) drum909_set(&d,v,v<=DR_OH?D9_HAT_HEAD:0,fast==1?127:64);
            drum909_set(&d,v,v<=DR_OH?0:D9_CY_HEAD,127);
            const d9_smp_t *s=&d.smp[v-DR_CH];
            fprintf(stderr,"rate %d %d %u.%u\n",fast,v,s->inc,s->incf);
            check(fast==1 ? s->inc==4 && s->incf==0 : s->inc==1 && (fast==2) == !!s->incf);
        }
        for (int t=0;t<1200;t++) {
            if(t==0 || t==11) {
                drum909_trigger(&d,DR_CH,1.f);
                drum909_trigger(&d,DR_CR,.7f);
                drum909_trigger(&d,DR_RD,1.f);
            }
            if(t==1 || t==12) drum909_trigger(&d,DR_OH,1.f);
            if(t==2 || t==15) drum909_trigger(&d,DR_CH,.62f);
            if(t==200) {
                drum909_trigger(&d,DR_CH,.92f);
                /* Endpoint-only witness: allow decoder to reach last nibble
                 * independently of the shorter real closed-hat max decay. */
                d.smp[0].mute=(int)d.smp[0].len+1;
            }
            if(t==300) drum909_trigger(&d,DR_OH,.92f);
            shot();
            for (int i=0;i<256;i++) sounded[fast]+=(buf[0][i]!=0.f);
            for(int v=DR_CH;v<=DR_RD;v++) {
                const d9_smp_t *s=&d.smp[v-DR_CH];
                boundary[fast]|=(s->pos>=s->len && s->dk<=s->len)<<(v-DR_CH);
                ended[fast]|=(s->mute==0 && s->dk>0)<<(v-DR_CH);
            }
        }
        check(sounded[fast]>0 && ended[fast]==15);
        fprintf(stderr,"Decoder end masks stage %d: %u\n",fast,boundary[fast]);
        check(boundary[fast]==15);
    }
    fprintf(stderr,"Unity/max Tune/restored detent64, CH/OH choke + retrigger, crash+ride overlap and every tail off: %u/%u/%u audible frames; stop masks %u/%u/%u\n",sounded[0],sounded[1],sounded[2],ended[0],ended[1],ended[2]);
}
