#include "gate_helpers.inc"
static pref_t mono(unsigned v)
{
 ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_PART);build_pages();
 visit("VOICE");smc_peek_cancel();smc_page_set(cur_page());
 CHECK(pg.r[cur_page()][0].kind==R_SJ6MONO32 && pg.r[cur_page()][0].b==v);
 for(unsigned k=1;k<4;k++)CHECK(pg.r[cur_page()][k].kind==R_NONE);
 return pg.r[cur_page()][0];
}
static void expected(unsigned owner,unsigned note,unsigned vel)
{
 juno6_t *p=parts_juno[owner];
 CHECK(all_gates(p)==1&&p->core.voice[0].gate&&p->core.voice[0].note==note);
 CHECK(p->core.voice[0].velocity==vel/127.f);
 CHECK(!all_gates(parts_juno[1-owner]));
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 stage_juno(0);ui.overlay=O_NONE;ui.part=PART_303A;set_view(V_PART);build_pages();
 CHECK(page_named("VOICE")<0);
 stage_juno(1);sound_t clean=proj.sound;unsigned writes=store_writes;
 for(unsigned v=0;v<NBASS;v++){
   const pref_t r=mono(v);CHECK(!pref_get(r));detent(0,1);render(2);
   CHECK(pref_get(r)==1&&parts_juno[v]->mono32_on&&!parts_juno[1-v]->mono32_on);
   key_midi(v,48,79);sample(1);expected(v,48,79);
   key_midi(v,67,95);sample(1);expected(v,67,95);
   key_midi(v,72,110);sample(1);expected(v,72,110);
   detent(0,-1);CHECK(pref_get(r)==1); /* requested active-mode policy explicitly not claimed */
   int32_t lr[512];unsigned audible=0;
   for(unsigned n=0;n<8;n++){engine_render(lr,256);for(unsigned k=0;k<512;k++)audible+=lr[k]!=0;}
   CHECK(audible>512);
   key_midi(v,72,0);sample(1);expected(v,48,100);
   key_midi(v,67,0);sample(1);expected(v,48,100);
   key_midi(v,48,0);sample(1);CHECK(!all_gates(parts_juno[v]));
   juno6_all_off(parts_juno[v]);detent(0,-1);CHECK(!pref_get(r));
   key_midi(v,48,79);sample(1);key_midi(v,67,95);sample(1);
   CHECK(all_gates(parts_juno[v])==2);key_midi(v,48,0);key_midi(v,67,0);sample(2);
   juno6_all_off(parts_juno[v]);
 }
 CHECK(!memcmp(&clean,&proj.sound,sizeof clean)&&store_writes==writes);
 printf("PASS physical Mono K1, newest onset/fixed voice0, lowest held fallback/100, audible alternate owners, poly restore, no project writes: %u checks\n",checks);
}
