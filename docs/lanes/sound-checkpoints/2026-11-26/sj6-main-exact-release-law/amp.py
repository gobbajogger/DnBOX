import sys
from pathlib import Path
c,d,o=map(Path,sys.argv[1:]);t=o/'lease-types.h';s=t.read_text()
s=s.replace('typedef struct sj6_cold_ctx {','''#include "warm_amp.h"
typedef struct sj6_cold_ctx { sj6_warm_amp voice_amp[6]; unsigned gate[6]; float adsr[4]; unsigned amp_on;''')
t.write_text(s)
t=o/'lease-logic.h';s=t.read_text();a='float stock_sub_filtered,float *result)';assert s.count(a)==1
s=s.replace(a,'float stock_sub_filtered,float *result,float *main_y)')
a=' *result=(y*f->main_gain)+stock_sub_filtered;';assert s.count(a)==1
s=s.replace(a,' *main_y=y*f->main_gain;\n *result=*main_y+stock_sub_filtered;')
s=s.replace('float stock_sub,float *out)','float stock_sub,float *out,float *main_y)').replace('run->profile,f,stock_sub,out)','run->profile,f,stock_sub,out,main_y)')
t.write_text(s)
s=c.read_text()
s=s.replace('assert(arm<=3);','assert(arm<=4);').replace('if(ota_arm!=3 || gain==0.f)','if(ota_arm<3 || gain==0.f)')
a='void bind_lab(dnbx_part *s';assert s.count(a)==1
s=s.replace(a,'''static void amp_birth(sj6_cold_ctx *ctx,unsigned i){
 if(!ctx->amp_on)return;
 float *a=ctx->adsr; sj6_amp_config_panel(ctx->voice_amp+i,44100.f,a[0],a[1],a[2],a[3]);
 sj6_amp_on(ctx->voice_amp+i);ctx->gate[i]=1;
}
static float amp_step(sj6_cold_ctx *ctx,unsigned i,unsigned k,unsigned n,unsigned gate){
 sj6_warm_amp *v=ctx->voice_amp+i;
 if(ctx->gate[i]&&!gate){sj6_amp_off(v);v->remain=0;}
 ctx->gate[i]=gate;
 if(k==0||!v->remain){unsigned f=n-k;sj6_amp_prep(v,f>16?16:f);}
 return sj6_amp_sample(v);
}
void bind_amp(dnbx_part *s,float a,float d,float u,float r){
 if(s->cold_lab && ota_arm==4){
   s->cold_lab->amp_on=1;float *p=s->cold_lab->adsr;p[0]=a;p[1]=d;p[2]=u;p[3]=r;
 }
}
'''+a)
s=s.replace('++lab_resets;}\n    if(ota_arm)','++lab_resets;amp_birth(s->cold_lab,chosen);}\n    if(ota_arm)')
s=s.replace('++lab_resets;}}','++lab_resets;amp_birth(s->cold_lab,0);}}')
a='                    float y;\n                    if(guarded_main_sample(&ctx->run,ctx->lease,v,i,&f,sub_y,&y)!=1)abort();\n                    mix[l]+=velocity*amp*y;'
assert s.count(a)==1
s=s.replace(a,'''                    float y,main_y;
                    if(guarded_main_sample(&ctx->run,ctx->lease,v,i,&f,sub_y,&y,&main_y)!=1)abort();
                    float audible_main_amp=amp;
                    if(ctx->amp_on){
                      float shaped=amp_step(ctx,i,k,n,v->gate);
                      if(!v->gate)audible_main_amp=shaped;
                    }
                    if(audible_main_amp==amp)mix[l]+=velocity*amp*y;
                    else mix[l]+=velocity*(amp*sub_y+audible_main_amp*main_y);''')
s=s.replace('lv->amp.stage,amp,sub_y,y};','lv->amp.stage,audible_main_amp,sub_y,y};')
c.write_text(s)
s=d.read_text();a='donor->osc1_level_code?((float)(donor->osc1_level_code-1)/127.f)*.808f:1.f);';assert s.count(a)==1
s=s.replace(a,a+'''
  extern void bind_amp(dnbx_part *,float,float,float,float);
  bind_amp(&donor->core,word[36],word[37],word[38],word[39]);''')
d.write_text(s)
