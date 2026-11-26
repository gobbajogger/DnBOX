import sys
from pathlib import Path
core,fx,driver,out=map(Path,sys.argv[1:])
s=core.read_text()
p='''        left[k]=0; right[k]=0;
        for (unsigned l=0;l<DNBX_LAYERS;l++) {'''
assert s.count(p)==1
s=s.replace(p,'''        left[k]=0; right[k]=0;
#if defined(X0X_HOST) && X0X_HOST && DNBX_HOST_SJ6_AUTHENTIC_CENTER
        /* Narrow comparison: centered typed donors only. Do not generalize
           the authored behavior to unsupported pan or non-SJ6 owners. */
        const int use_source = s->superj6_amp_range &&
                              s->layer[0].params.pan==0.f && s->layer[1].params.pan==0.f;
        extern void center_meter(int);
        center_meter(use_source);
#endif
        for (unsigned l=0;l<DNBX_LAYERS;l++) {''')
p='''            float x=mix[l]*(p->gain/6.0f);
            left[k]+=x*(.5f-.5f*p->pan); right[k]+=x*(.5f+.5f*p->pan);'''
assert s.count(p)==1
s=s.replace(p,'''#if defined(X0X_HOST) && X0X_HOST && DNBX_HOST_SJ6_AUTHENTIC_CENTER
            if(use_source) {
                float restored=mix[l]*p->gain;
                left[k]+=restored;
                right[k]+=restored;
                continue;
            }
#endif
'''+p)
(out/'core.c').write_text(s)
s=fx.read_text()
p='        c->mode_history[ch][s->write]=j60_quantize(h);'
assert s.count(p)==1
fx.write_text(s.replace(p,'        extern void bbd_meter(float); bbd_meter(h);\n'+p))
s=driver.read_text()
p='static float peak[2][2];';assert s.count(p)==1
s=s.replace(p,'''#include <stdlib.h>
static unsigned accept,refuse,wet_samples,clipped;
static float wet_peak;
void center_meter(int enabled){if(enabled)++accept;else ++refuse;}
void bbd_meter(float h){++wet_samples;h=fabsf(h);if(h>wet_peak)wet_peak=h;if(h>7.999f)++clipped;}
'''+p)
p='  prepare(bpm);';assert s.count(p)==1
s=s.replace(p,p+'''
  /* Refusal-only proof; authored and dry comparisons never use this mode. */
  if(getenv("SJ6_COMPARISON_PAN_GUARD")){
    dnbx_layer_params lp;
    assert(!dnbx_get_layer(&part.dsp.juno.synth.core,0,&lp));
    lp.pan=.25f;
    assert(!dnbx_set_layer(&part.dsp.juno.synth.core,0,&lp));
  }''')
p='  fprintf(stderr,"gain ';assert s.count(p)==1
s=s.replace(p,'''  fprintf(stderr,"center %u %u bbd %u %u %a\\n",accept,refuse,wet_samples,clipped,(double)wet_peak);
'''+p)
(out/'driver.c').write_text(s)
