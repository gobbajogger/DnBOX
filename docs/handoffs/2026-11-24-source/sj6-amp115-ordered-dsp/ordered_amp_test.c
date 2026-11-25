/* SPDX-License-Identifier: GPL-3.0-only */
#include "juno6.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
enum {N=28544};
static float pcm[3][2][N];
#define C(x) do{if(!(x)){fprintf(stderr,"FAIL at line %u\n",__LINE__);return 1;}}while(0)
static int play(juno6_t s,unsigned which)
{
 if(!juno6_note_on(&s,57,100))return 0;
 for(unsigned n=0;n<N;n+=256) {
  if(n==16384)juno6_note_off(&s,57);
  if(!juno6_render_stereo(&s,pcm[which][0]+n,pcm[which][1]+n,256))return 0;
 }
 return 1;
}
static int prepare(juno6_t *s)
{
 juno6_init(s,44100);
 dnbx_layer_params p;
 for(unsigned i=0;i<2;++i){
  if(dnbx_get_layer(&s->core,i,&p)!=DNBX_OK) return 0;
  p.pan=i?1.f:-1.f;
  if(dnbx_set_layer(&s->core,i,&p)!=DNBX_OK)return 0;
 }
 return 1;
}
#ifdef POST
static float eg(const juno6_t *s,unsigned l,unsigned st)
{
 const dnbx_envelope_params *p=&s->core.layer[l].params.amp;
 switch(st){case 0:return p->attack_s;case 1:return p->decay_s;case 2:return p->sustain;default:return p->release_s;}
}
static unsigned ndiff(unsigned a,unsigned b,unsigned lane)
{
 unsigned n=0;for(unsigned i=0;i<N;++i)n+=pcm[a][lane][i]!=pcm[b][lane][i];return n;
}
#endif
int main(int argc,char **argv)
{
 C(argc==2);
 juno6_t s;
 C(prepare(&s)&&play(s,0));
 FILE *file=fopen(argv[1],"wb"); C(file&&fwrite(pcm[0],sizeof pcm[0],1,file)==1&&!fclose(file));
#ifdef POST
 for(unsigned owner=0;owner<2;++owner){
  C(prepare(&s));
  const dnbx_envelope_params f[2]={s.core.layer[0].params.filter,s.core.layer[1].params.filter};
  juno6_amp_state st,base;
  C(juno6_read_amp_state(&s,&st)==0);
  C(!juno6_edit_amp_stage(&s,0,0,0,0));
  C(juno6_edit_amp_link(&s,1,0));
  C(juno6_read_amp_state(&s,&base)==1 && base.link==2 && base.mask==15);
  for(unsigned i=0;i<4;++i) C(fabsf(eg(&s,0,i)-eg(&s,1,i))<.001f);
  for(unsigned stage=0;stage<4;++stage){
   /* Build meaningful timing at a genuine 44.1 kHz note, using distinct
    * values whose region falls inside the observed audio window. */
   unsigned a=stage==2?36000u:12000u, b=stage==2?53000u:26000u;
   C(juno6_edit_amp_stage(&s,0,stage,a,0));
   C(eg(&s,0,stage)==eg(&s,1,stage) && play(s,0));
   float primary=eg(&s,0,stage);
   C(juno6_edit_amp_stage(&s,1,stage,b,0));
   C(juno6_read_amp_state(&s,&st)==1 && st.link==2 && st.q16[stage]==b);
   C(eg(&s,0,stage)==primary && eg(&s,1,stage)!=primary && play(s,1));
   unsigned differ=ndiff(0,1,1);
   C(!ndiff(0,1,0)&&differ>100u);
   printf("owner=%u stage=%u secondary-only %u/%u right stereo frames, primary exact\n",owner,stage,differ,N);
   C(juno6_edit_amp_link(&s,0,0));
   C(juno6_read_amp_state(&s,&st)==1&&st.link==1&&st.q16[stage]==b);
   C(play(s,2)&&!memcmp(pcm[1],pcm[2],sizeof pcm[1])); /* OFF no restore */
   C(juno6_edit_amp_link(&s,1,0) && play(s,2));
   C(!ndiff(0,2,0) && !ndiff(0,2,1)); /* ON reprojection is exact for this controlled stage */
   st.link=2; /* Seq restore must not invoke ON/copy. */
   C(juno6_restore_amp_state(&s,&st,0)&&play(s,2)&&!memcmp(pcm[1],pcm[2],sizeof pcm[1]));
   C(juno6_read_amp_state(&s,&base)==1 && base.q16[stage]==b && base.link==2);
   C(juno6_edit_amp_stage(&s,0,stage,a,0) && play(s,2)&&!memcmp(pcm[0],pcm[2],sizeof pcm[0]));
  }
  C(juno6_set(&s,JUNO6_APP_PARAM_BASE+119u,.9f));
  memset(&st,0x51,sizeof st); base=st;
  C(juno6_read_amp_state(&s,&st)==-1&&!memcmp(&st,&base,sizeof st));
  C(juno6_edit_amp_link(&s,1,0)&&juno6_read_amp_state(&s,&st)==1&&st.mask==15);
  C(juno6_set(&s,5u,.25f));
  C(juno6_read_amp_state(&s,&st)==-1);
  C(juno6_edit_amp_link(&s,1,0)&&juno6_read_amp_state(&s,&st)==1&&st.mask==15);
  C(!memcmp(f,&s.core.layer[0].params.filter,sizeof f[0]));
  C(!memcmp(f+1,&s.core.layer[1].params.filter,sizeof f[1]));
  juno6_t before=s;
  C(!juno6_edit_amp_link(&s,1,1)&&!memcmp(&before,&s,sizeof s));
  C(!juno6_edit_amp_stage(&s,1,3,1000,1)&&!memcmp(&before,&s,sizeof s));
  C(!juno6_restore_amp_state(&s,&st,1)&&!memcmp(&before,&s,sizeof s));
  st.mask=0; C(!juno6_restore_amp_state(&s,&st,0)&&!memcmp(&before,&s,sizeof s));
  C(dnbx_set_filter_link(&s.core,1)==DNBX_OK);before=s;
  C(!juno6_edit_amp_stage(&s,1,3,1000,0)&&!memcmp(&before,&s,sizeof s));
  C(dnbx_set_filter_link(&s.core,0)==DNBX_OK&&juno6_note_on(&s,57,100)); before=s;
  C(!juno6_edit_amp_link(&s,1,0)&&!memcmp(&before,&s,sizeof s));
 }
#endif
 printf("PASS juno6_t=%zu dnbx_part=%zu\n",sizeof s,sizeof s.core);
 return 0;
}
