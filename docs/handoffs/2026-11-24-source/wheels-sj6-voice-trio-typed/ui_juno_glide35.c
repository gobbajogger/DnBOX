#include "gate_helpers.inc"
static void voice(unsigned v)
{
 ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_PART);build_pages();
 visit("VOICE");smc_peek_cancel();smc_page_set(cur_page());
 for(unsigned i=0;i<3;i++)CHECK(pg.r[cur_page()][i].kind==R_SJ6MONO32&&pg.r[cur_page()][i].b==v&&pg.r[cur_page()][i].c==i);
 for(unsigned i=3;i<4;i++)CHECK(pg.r[cur_page()][i].kind==R_NONE);
}
static dnbx_voice *voice0(unsigned v,unsigned n,unsigned vel)
{
 dnbx_voice *a=&parts_juno[v]->core.voice[0];
 CHECK(all_gates(parts_juno[v])==1&&a->gate&&a->note==n&&a->velocity==vel/127.f&&!all_gates(parts_juno[1-v]));
 return a;
}
static void edge(unsigned v,unsigned note,unsigned vel){key_midi(v,note,vel);sample(1);}
static void dry_full(unsigned n)
{
 int32_t lr[512];unsigned nonzero=0;
 while(n--){engine_render(lr,256);for(unsigned i=0;i<512;i++)nonzero+=lr[i]!=0;}
 CHECK(nonzero>256);
}
static void expect_narrow(float a,float b){CHECK(fabsf(a-b)<.00002f);}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;stage_juno(1);
 sound_t clean=proj.sound;unsigned writes=store_writes;
 for(unsigned o=0;o<2;o++){
  juno6_t *j=parts_juno[o];dnbx_part *p=&j->core;
  voice(o);
  CHECK(!pref_get(pg.r[cur_page()][2]));for(unsigned n=0;n<32;n++)detent(2,1);
  char num[16],unit[8];pref_value(pg.r[cur_page()][2],num,unit);
  CHECK(!strcmp(num,"126")&&!strcmp(unit,"ms")&&j->glide35_detent==32);
  CHECK(p->sj6_glide_secs==.5f*(32.f/127.f)&&!parts_juno[1-o]->glide35_detent);
  edge(o,48,79);edge(o,60,96);CHECK(all_gates(j)==2&&p->sj6_glide_offset==0.f);
  edge(o,48,0);edge(o,60,0);juno6_all_off(j);
  detent(0,1);detent(1,1);CHECK(j->mono32_on&&j->legato33_on);
  edge(o,48,79);dnbx_voice *v=voice0(o,48,79);const float hz48=v->layer[0].increment;
  CHECK(p->sj6_glide_offset==0.f);
  const float d=16.f/p->sample_rate*12.f/p->sj6_glide_secs;
  edge(o,60,101);v=voice0(o,60,79);CHECK(v->layer[0].increment>1.99f*hz48);
  expect_narrow(p->sj6_glide_offset,-12.f+d);
  CHECK(p->sj6_glide_ratio>0.5f&&p->sj6_glide_ratio<.6f);
  for(unsigned ticks=0;ticks<24;ticks++)sample(16);
  expect_narrow(p->sj6_glide_offset,-12.f+25.f*d);
  dry_full(4);const float prior=p->sj6_glide_offset;CHECK(prior<0.f);
  edge(o,60,0);voice0(o,48,79);
  expect_narrow(p->sj6_glide_offset,prior+12.f-d);
  CHECK(p->sj6_glide_offset>0.f);
  for(unsigned q=0;q<500u && p->sj6_glide_offset;++q)sample(32);CHECK(p->sj6_glide_offset==0.f&&p->sj6_glide_ratio==1.f);
  edge(o,48,0);juno6_all_off(j);
  for(unsigned n=0;n<32;n++)detent(2,-1);
  edge(o,48,79);edge(o,60,96);CHECK(p->sj6_glide_offset==0.f);
  edge(o,48,0);edge(o,60,0);juno6_all_off(j);
  detent(0,-1);detent(1,-1);
 }
 CHECK(store_writes==writes&&!memcmp(&clean,&proj.sound,sizeof clean));
 printf("PASS physical Glide Time K3 actual oscillator phase/rate trajectory with Mono's original arm, above and below targets, mid-glide retune/lowest fallback, audible both owners and no writes: %u checks\n",checks);
}
