/* SPDX-License-Identifier: GPL-3.0-only */
#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"
#include "s1c_part.h"
static s1_dry_part *last;
void __real_s1_part_init(s1_dry_part *);
void __wrap_s1_part_init(s1_dry_part *p){__real_s1_part_init(p);last=p;}
static void send(unsigned owner,unsigned type,unsigned d1,unsigned d2)
{
    const unsigned ch=seq.ch[TRK_BASS0+owner];
    host_midi_put((type>>4) | ((type|ch)<<8) | d1<<16 | d2<<24);
    render(2);
}
static void select_part(unsigned v,unsigned bank)
{
    EXPECT(!engine_bass_select_preset(v,bank,0,&proj.sound));
    for(unsigned i=0;i<1024 && engine_bass_preset_pending(v);++i)render(1);
    EXPECT(engine_bass_bank(v)==(int)bank);
}
int main(void)
{
    setup(V_PART,PART_303A);
    s1_dry_part *part[2];
    for(unsigned v=0;v<2;++v){
        last=NULL;select_part(v,ORIGINAL_SOUND_SYNTHONE);
        EXPECT(last);part[v]=last;
    }
    EXPECT(part[0]!=part[1]);
    static const unsigned ticks[]={1,2,63,64,126,127};
    const sound_t saved=proj.sound;const unsigned writes=store_writes;
    for(unsigned v=0;v<2;++v)
      for(unsigned i=0;i<sizeof ticks/sizeof ticks[0];++i){
        send(v,0x90,48,101);
        send(v,0xb0,64,ticks[i]);
        EXPECT(part[v]->damper && !part[1-v]->damper);
        send(v,0x80,48,0);
        EXPECT(part[v]->held && !part[1-v]->held);
        send(v,0xb0,64,0);
        EXPECT(!part[v]->held && !part[v]->damper && !part[1-v]->damper);
      }
    for(unsigned v=0;v<2;++v){
        for(unsigned n=0;n<2;++n){
            unsigned positive=n?127:1;
            send(v,0xb0,64,positive);
            EXPECT(part[v]->damper);
            send(v,0x90,48,101);send(v,0x80,48,0);
            EXPECT(part[v]->held);
            send(v,0xb0,64,positive);
            EXPECT(!part[v]->held && !part[v]->damper);
            send(v,0xb0,64,0);
            EXPECT(!part[v]->damper && !part[1-v]->damper);
        }
    }
    EXPECT(!memcmp(&saved,&proj.sound,sizeof saved) && store_writes==writes);
    for(unsigned v=0;v<2;++v) {
        for(unsigned bank=0;bank<=ORIGINAL_SOUND_JUNO;++bank){
            if(bank==ORIGINAL_SOUND_SYNTHONE)continue;
            select_part(v,bank);
            send(v,0xb0,64,1);
            EXPECT(engine_bass_perf(v,1,1)==1);
            send(v,0xb0,64,63);
            EXPECT(engine_bass_perf(v,1,1)==63);
            send(v,0xb0,64,127);
            EXPECT(engine_bass_perf(v,1,1)==127);
            send(v,0xb0,64,0);
            EXPECT(engine_bass_perf(v,1,1)==0);
        }
    }
    if(failures)return 1;
    printf("PASS real Synth One MIDI pedal low+high decisions, held voices, owner isolation, legacy peers (%u checks)\n",expectations);
    return 0;
}
