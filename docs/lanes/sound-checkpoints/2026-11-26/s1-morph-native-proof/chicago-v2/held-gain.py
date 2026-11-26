from pathlib import Path
import sys
p=Path(sys.argv[1])/'tree/experiments/original_synth/chicago_mp4.c'
t=p.read_text(); a=t.index('static inline __attribute__((always_inline)) int ck_step_hold'); b=t.index('static inline int ramp_held',a)
u=t[a:b]
u=u.replace('const float gain[4],float ng','const float half[4],unsigned mask,float ng')
u=u.replace('gain[i]!=0.0f','(mask&(1u<<i))!=0').replace('gain[i+1]!=0.0f','(mask&(2u<<i))!=0').replace('gain[0]!=0.0f','(mask&1u)!=0')
u=u.replace('(gain[0]*.5f)','half[0]').replace('(gain[i]*.5f)','half[i]')
t=t[:a]+u+t[b:]
a='  float gain[4]={og[0].current,og[1].current,og[2].current,og[3].current};'
assert t.count(a)==1
t=t.replace(a,'''  /* Compute exact held mixer factors only once. Keep activity from
   * the original pre-half values: tiny nonzero values (including those
   * whose half rounds to zero) still take the same oscillator/mix branch. */
  float half[4]; unsigned mask=0;
  for(unsigned i=0;i<4;i++){
   half[i]=og[i].current*.5f;
   if(og[i].current!=0.0f)mask|=1u<<i;
  }''').replace('phase,gain,ng,A,E,R','phase,half,mask,ng,A,E,R')
p.write_text(t)
