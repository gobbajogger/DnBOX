#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include "dsp/juno6.h"
static juno6_t *last_juno;
void __real_juno6_init(juno6_t *, float);
void __wrap_juno6_init(juno6_t *p,float hz) { __real_juno6_init(p,hz);last_juno=p; }
static void select_owner(unsigned owner,const char *page)
{
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
 visit(page);smc_peek_cancel();smc_page_set(cur_page());
}
static unsigned count_page(const char *name)
{
 unsigned n=0;for(int i=0;i<pg.n;i++)n+=!strcmp(page_title(&pg,i),name);return n;
}
static void arrange(unsigned s)
{
 project_reset_stage();project_stage.set.snd_rev=REV137;
 for(unsigned i=0;i<NBASS;i++) {
  CHECK(original_sound_init(project_stage.sound.v[T_303][i],i==s?ORIGINAL_SOUND_SYNTHONE:ORIGINAL_SOUND_JUNO,0));
  engine_bass_sound_refresh(&project_stage.sound,i);
  slot137(&project_stage.sound,i)[0]=137;
  slot137(&project_stage.sound,i)[1]=(uint8_t)i;
 }
 CHECK(!project_validate_stage());project_commit_stage();stabilize();undo_start();
}
static void owner_roundtrip(unsigned s)
{
 unsigned j=1-s;
 arrange(s);
 s1_dry_part *synth=current;
 juno6_t *juno=last_juno;
 CHECK(synth&&juno);
 knob_mod(s,2);
 select_owner(s,"WHEELS");
 CHECK(count_page("FILTER MOD")==0);
 detent(0,-1);detent(1,1);render(4);
 CHECK(s1_pb_decode(read137(&proj.sound,s)[4])==11);
 CHECK(s1_pb_decode(read137(&proj.sound,s)[5])==13);
 select_owner(s,"FX SWITCHES");
 CHECK(s1_patch_at(pg.r[cur_page()][2].c)->factory_id==51);
 detent(2,1);render(4);CHECK(read_slot(s,51).position==1);
 select_owner(j,"FILTER MOD");
 CHECK(count_page("WHEELS")==0 && count_page("PITCH MOD")==0);
 CHECK(pg.r[cur_page()][0].c==SJ6_FILTER_VELOCITY && pg.r[cur_page()][0].b==j);
 for(unsigned n=0;n<7;n++){detent(0,1);render(4);}
 CHECK(juno->filter_velocity_position==7 && synth->mod_wheel_route==2);
 const uint8_t *a=read137(&proj.sound,s),*b=read137(&proj.sound,j);
 CHECK(a[2]==0&&a[3]==2&&b[2]==7&&b[3]==0&&b[4]==0&&b[5]==0);
 witness=proj;
 CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound));
 CHECK(current->mod_wheel_route==2 && last_juno->filter_velocity_position==7 && read_slot(s,51).position==1);
 CHECK(s1_pb_decode(read137(&proj.sound,s)[4])==11 && s1_pb_decode(read137(&proj.sound,s)[5])==13);
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 owner_roundtrip(0);
 owner_roundtrip(1);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/mixed-media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 printf("PASS actual mixed project S1 MOD/PB/ID51 + SJ6 ID137, swapped dynamic UI owners and exact staged recall: %u checks\n",checks);
}
