/* SPDX-License-Identifier: GPL-3.0-only */
/* Isolated settled-value sweep of actual production dv_glyph. No device I/O,
 * no production edits and no blanket monotonicity/admission assertion. */
#define SMC_UI_MAIN retained_smc_fixture_main
#include "smc_ui_test.c"
#include "remote_glyph_cases.inc"
static void live_capture(int kit,int voice,const char *prefix)
{
 setup(V_PART,kit);ui.sel[kit]=(uint8_t)voice;build_pages();*page_of(V_PART)=0;build_pages();
 if(voice==1){pref_set(pg.r[0][1],127);pref_set(find_ref(T_909,1,"Decay"),127);}
 int32_t audio[512];for(int i=0;i<50;i++)engine_render(audio,256);audio_due_ms=now_ms;run_ms(300);ui_ease_reset();
 const int times[]={0,20,170,6170};int previous=0;
 for(int phase=0;phase<4;phase++){
  if(phase==1)engine_drum(kit,voice,1.f);
  int ms=times[phase]-previous;previous=times[phase];for(int i=0;i<ms;i+=6)engine_render(audio,256);audio_due_ms=now_ms;run_ms(ms);now_ms++;ui_frame();
  char name[80];snprintf(name,sizeof name,"%s-%d",prefix,times[phase]);shot(name);
  uint32_t age=0;float env=0;int live=engine_drum_view(kit,voice,&age,&env);
  printf("LIVE_FRAME %s live=%d age=%u env=%.9g\n",name,live,age,(double)env);
 }
}
int main(void)
{
 const char *dir=getenv("REMOTE_GLYPH_OUT");CHECK(dir && *dir);
 for(unsigned i=0;i<sizeof cases/sizeof cases[0];i++) {
  const sweep_case *c=&cases[i];setup(V_PART,c->kit==1?PART_808:PART_909);
  ui.sel[ui.part]=(uint8_t)c->selected_voice;build_pages();*page_of(V_PART)=(uint8_t)c->page;build_pages();
  pref_t r=PR(c->kind,c->target,c->voice,c->slot);const x0x_param_t *d=pref_desc(r);
  CHECK(d && !strcmp(d->name,c->name));int found=0;for(int k=0;k<4;k++)if(pg.r[c->page][k].kind==r.kind&&pg.r[c->page][k].a==r.a&&pg.r[c->page][k].b==r.b&&pg.r[c->page][k].c==r.c)found=1;CHECK(found);
  int original=pref_get(r);char path[1024];snprintf(path,sizeof path,"%s/t%d-v%d-p%d.rgb565be",dir,c->target,c->voice,c->slot);FILE *f=fopen(path,"wb");CHECK(f);
  printf("{\"kind\":%d,\"target\":%d,\"voice\":%d,\"slot\":%d,\"selectedVoice\":%d,\"page\":%d,\"name\":\"%s\",\"role\":%d,\"minimumRaw\":0,\"maximumRaw\":%u,\"storedRaw\":%d,\"width\":64,\"height\":48,\"origin\":[8,8],\"scale\":[1.375,1.5],\"backgroundRgb565\":%u,\"inkRgb565\":%u,\"values\":[",c->kind,c->target,c->voice,c->slot,c->selected_voice,c->page,d->name,drum_strip_role(r),(unsigned)d->max,original,(unsigned)RGB(36,48,54),(unsigned)part_col());
  for(int raw=0;raw<=d->max;raw++) {
   char number[16],unit[8];pref_value_of(r,raw,number,unit);
   cv_begin(64,48,RGB(36,48,54));cv_band(0,48);dv_glyph(r,(float)raw,8.f,8.f,1.375f,1.5f,part_col());
   CHECK(cv_w==64 && cv_h==48);CHECK(fwrite(cv_px,2,64*48,f)==64*48);
   CHECK(pref_get(r)==original);
   printf("%s{\"raw\":%d,\"number\":\"%s\",\"unit\":\"%s\"}",raw?",":"",raw,number,unit);
  }
  uint32_t age=0;float env=0;int live=drum_live(r,&age,&env);CHECK(!live);CHECK(env==0);CHECK(age==0xffffffffu);
  printf("],\"liveContext\":{\"live\":%d,\"age\":%u,\"envelope\":%.9g},\"dependencyVector\":[",live,age,(double)env);
  int first=1;
  for(int voice=0;voice<engine_nvoices(c->target);voice++)for(int slot=0;slot<engine_nparams(c->target,voice);slot++){
   pref_t dep=PR(R_ENG,c->target,voice,slot);const x0x_param_t *dd=pref_desc(dep);CHECK(dd);
   printf("%s{\"target\":%d,\"voice\":%d,\"slot\":%d,\"raw\":%d}",first?"":",",c->target,voice,slot,pref_get(dep));first=0;
  }
  puts("]}");CHECK(!fclose(f));
 }
 live_capture(0,0,"decay909");live_capture(1,0,"decay808");live_capture(0,1,"noise909");
 fprintf(stderr,"PASS read-only native glyph sweep: %u checks\n",checks);return 0;
}
