/* SPDX-License-Identifier: GPL-3.0-only */
#include "synth_init_test_helpers.h"
#include "app/s1c_part.h"
#include "dsp/s1c/s1c_front.h"
#include <math.h>
static s1_dry_part *cur,*part[2];
void __real_s1_part_init(s1_dry_part *);
void __wrap_s1_part_init(s1_dry_part *s) { __real_s1_part_init(s);cur=s; }
static void event(unsigned owner,unsigned note,unsigned vel)
{
    const unsigned ch=seq.ch[TRK_BASS0+owner];
    host_midi_put(0x9u | ((0x90u | ch)<<8) | (note<<16) | (vel<<24));
    render(1);
}
static const s1c_voice *seen(unsigned owner,unsigned note)
{
    for (unsigned i=0;i<S1C_VOICES;i++) {
        const s1c_voice *v=&part[owner]->e.v[i];
        if (v->root==(int)note && v->gate) return v;
    }
    CHECK(0);return 0;
}
static void tune(unsigned owner,float input,float exact)
{
    CHECK(s1c_front_set(&part[owner]->e,143,input));
    CHECK(part[owner]->e.t[143]==exact);
}
int main(void)
{
    setup(V_PART,PART_303A);
    for(unsigned i=0;i<2;i++){cur=NULL;select_patch(i);CHECK(cur);part[i]=cur;}
    CHECK(part[0]!=part[1]);
    CHECK(part[0]->e.t[143]==440.f && part[1]->e.t[143]==440.f);
    const char *dump=getenv("DNBOX_A4_UNEDITED");
    if(dump){
       event(0,69,101);event(1,66,100);record(reference);
       FILE *f=fopen(dump,"wb");CHECK(f && fwrite(reference,sizeof reference,1,f)==1 && !fclose(f));return 0;
    }
    const sound_t saved=proj.sound;
    const unsigned writes=store_writes;
    for(unsigned i=0;i<2;i++) {
      unsigned other=1-i;
      event(i,69,101);event(other,69,97);
      CHECK(seen(i,69)->freq==440.f && seen(other,69)->freq==440.f);
      tune(i,470.99f,470.f);
      CHECK(seen(i,69)->freq==440.f); /* original voice copies pitch on note onset */
      event(i,69,0);event(i,69,101);
      CHECK(seen(i,69)->freq==470.f && seen(other,69)->freq==440.f);
      event(i,57,100);
      CHECK(seen(i,57)->freq==235.f);
      tune(i,-1.e30f,410.f);event(i,69,0);event(i,69,101);
      CHECK(seen(i,69)->freq==410.f && seen(other,69)->freq==440.f);
      tune(i,1.e30f,470.f);
      tune(i,432.999f,432.f);event(i,81,100);
      CHECK(seen(i,81)->freq==864.f);
      CHECK(!s1c_front_set(&part[i]->e,143,NAN) && part[i]->e.t[143]==432.f);
      for(unsigned n=0;n<=127;++n) {
        event(i,n,100);
        const float oracle=(float)(432.0*exp2(((double)n-69.0)/12.0));
        const float hz=seen(i,n)->freq;
        CHECK(fabsf(hz-oracle)<=oracle*5.e-7f);
        event(i,n,0);
      }
      tune(i,440.f,440.f);
      event(i,57,0);event(i,69,0);event(i,81,0);event(other,69,0);
    }
    CHECK(!memcmp(&proj.sound,&saved,sizeof saved)&&store_writes==writes);
    printf("PASS original Synth One 12ET reference edit, both live owners, all MIDI notes and held non-retuning, legacy Init unchanged; %u checks\n",checks);
    return 0;
}
