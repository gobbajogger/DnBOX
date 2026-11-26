from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
a='static inline float ck_phase(uint32_t u){return (float)(int32_t)u*0x1p-31f;}'
addition='''
/* The prepared master/slave increments and XMOD control are immutable for a
 * span admitted without a control tick. Cache the source float conversion and
 * exact saturated truncation; the integer bit pattern is not the raw increment. */
static inline uint32_t ck_slave_delta(float b){
 float step=ck_clamp(b,-1,1)*0x1p31f;
 return (uint32_t)(step>=0x1p31f?INT32_MAX:(int32_t)step);
}
struct ck_slave_plan {float base[2];uint32_t step[2];};
'''
assert s.count(a)==1;s=s.replace(a,a+addition)
for name in ('ck_step','ck_step_hold'):
 a='int '+name+'(const original_prepared_oscillators *p,float vco_gain,'
 assert s.count(a)==1;s=s.replace(a,a+'\n const struct ck_slave_plan *plan,')
a='''  float b=(float)c->increment[i+1]*0x1p-31f,step;
  if(gain[i]!=0.0f||c->xmod!=0.0f){
   w[i]=ck_wave(c->shape[i],ck_phase(phase[i]),c->width[i],p->correction[i],p->triangle[i]);
   float a=w[i]*c->xmod;step=ck_clamp(a+b,-1,1)*0x1p31f;
  }else{w[i]=0.0f;step=ck_clamp(b,-1,1)*0x1p31f;}
  phase[i+1]-=(uint32_t)(step>=0x1p31f?INT32_MAX:(int32_t)step);'''
b='''  uint32_t step=plan->step[i>>1];
  const int modulated=c->xmod!=0.0f;
  w[i]=(gain[i]!=0.0f||modulated)?
    ck_wave(c->shape[i],ck_phase(phase[i]),c->width[i],p->correction[i],p->triangle[i]):0.0f;
  if(modulated) step=ck_slave_delta(w[i]*c->xmod+plan->base[i>>1]);
  phase[i+1]-=step;'''
assert s.count(a)==2;s=s.replace(a,b)
a=' if(!p->ready||!f->prepared)return 0;'
assert s.count(a)==1;s=s.replace(a,a+'''
 struct ck_slave_plan plan;
 for(unsigned i=0;i<2;i++){
  plan.base[i]=(float)p->controls.increment[2*i+1]*0x1p-31f;
  plan.step[i]=ck_slave_delta(plan.base[i]);
 }''')
s=s.replace('ck_step_hold(p,vco_gain,phase','ck_step_hold(p,vco_gain,&plan,phase').replace('ck_step(p,vco_gain,phase','ck_step(p,vco_gain,&plan,phase')
p.write_text(s)
