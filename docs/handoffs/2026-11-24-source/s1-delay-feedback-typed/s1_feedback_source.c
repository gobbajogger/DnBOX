#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include <float.h>
static s1c *watch;
static float gain,ticks;
static int dec;
static unsigned observed;
void host_feedback_tap(const s1c *s,float f,float t,int d)
{if(s==watch){gain=f;ticks=t;dec=d;observed++;}}
static void fixture_env(s1_dry_part *p){
 s1c_set(&p->e,S1C_P_DLT,.020f);
 s1c_set_delay_gate(&p->e,1.f,1);
 s1c_set(&p->e,S1C_P_DLMIX,1.f);
 s1c_set(&p->e,S1C_P_RVON,0.f);
 s1c_set(&p->e,29,.004f);
 s1c_set(&p->e,144,.0001f);
}
static double take(unsigned owner,int typed,float feedback,float *g){
 stabilize();
 s1_dry_part *p=parts[owner];watch=&p->e;watch->delay_feedback_direct=typed;
 fixture_env(p);s1c_set(watch,S1C_P_DLFB,feedback);
 static float l[256],r[256];
 for(unsigned f=0;f<512;f++)s1_part_render(p,l,r,256);
 CHECK(s1_part_note(p,57,100));double en=0;observed=0;
 for(unsigned f=0;f<64;f++){
   if(f==8)CHECK(s1_part_note(p,57,0));
   s1_part_render(p,l,r,256);
   if(f>=32)for(unsigned k=0;k<256;k++){
     CHECK(isfinite(l[k])&&isfinite(r[k]));
     en+=(double)l[k]*l[k]+(double)r[k]*r[k];
   }
 }
 CHECK(observed==64*256&&dec>1&&en>0);
 *g=gain;watch=NULL;return en;
}
static void unsafe_time(unsigned owner) {
 stabilize();s1_dry_part*p=parts[owner];watch=&p->e;fixture_env(p);
 static float l[256],r[256];s1_part_render(p,l,r,256);
 unsigned len=watch->dl[0].len;
 int original=watch->ddec;
 CHECK(s1_part_note(p,64,97));
 for(unsigned j=0;j<16;j++)s1_part_render(p,l,r,256);
 CHECK(s1c_front_set(watch,S1C_P_DLT,2.5f));
 for(unsigned j=0;j<128;j++)s1_part_render(p,l,r,256);
 CHECK(watch->ddec==original && (unsigned)watch->dl[0].len==len && 2*ticks>3*len);
 fprintf(stderr,"owner%u ID43 existing long target %.3fs, 2T tap %.1f ticks vs %u short-ring, ddec=%d unchanged\n",owner,watch->t[S1C_P_DLT],2*ticks,len,dec);
 watch=NULL;
}
int main(void){
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 for(unsigned i=0;i<NBASS;i++){current=NULL;select_patch(i);CHECK(current);parts[i]=current;}
 stabilize();
 const char *dump=getenv("S1_FEEDBACK_UNTOUCHED");
 if(dump){engine_bass_on(0,60,0,0);engine_bass_on(1,67,0,0);record(reference);
   FILE*f=fopen(dump,"wb");CHECK(f);CHECK(fwrite(reference,sizeof reference,1,f)==1&&!fclose(f));return 0;}
 for(unsigned v=0;v<NBASS;v++){
   float g[5];double e[5];
   e[0]=take(v,0,.8f,g+0);e[1]=take(v,1,.8f,g+1);e[2]=take(v,1,.9f*114/127.f,g+2);
   e[3]=take(v,1,.1f,g+3);e[4]=take(v,1,.9f*15/127.f,g+4);
   CHECK(g[0]>.95f&&fabsf(g[1]-.8f)<1e-3f&&g[2]>g[1]&&g[4]>g[3]);
   CHECK(e[2]>e[1]&&e[4]>e[3]);
   fprintf(stderr,"owner%u old %.7f, typed .8 gain %.7f -> %.7f tail %.5g->%.5g; typed INIT gain %.7f -> %.7f tail %.5g->%.5g\n",v,g[0],g[1],g[2],e[1],e[2],g[3],g[4],e[3],e[4]);
   unsafe_time(v);
 }
 printf("PASS real both-owner feedback, fresh direct gain prevents first-detent tail inversion; ID43 diagnosed on actual short ring: %u checks\n",checks);
}
