from pathlib import Path
import sys
j,c,out=map(Path,sys.argv[1:])
s=j.read_text()
a='  c->delay[write] = (int16_t)clampf(in * 32767, -32768, 32767);'
assert s.count(a)==1
s=s.replace(a,'  extern void ring_meter(float); ring_meter(in * 32767);\n'+a)
classic=j.with_name('juno6_classic_native.inc').read_text()
needle='        c->mode_history[0][s->pos]=(int16_t)'
assert classic.count(needle)==1
(out/'classic-measured.inc').write_text(classic.replace(needle,'        extern void typed_ring_meter(float); typed_ring_meter(x); typed_ring_meter(y);\n'+needle))
s=s.replace('#include "juno6_classic_native.inc"','#include "classic-measured.inc"')
(out/'juno-measured.c').write_text(s)
s=c.read_text().replace('#include <assert.h>','#include <assert.h>\n#include <stdlib.h>')
s=s.replace('static float peak','static unsigned ring_n,ring_cuts; static float ring_peak;\nvoid ring_meter(float x){ ++ring_n; ring_cuts+=x < -32768.f || x > 32767.f; if(fabsf(x)>ring_peak)ring_peak=fabsf(x); }\nvoid typed_ring_meter(float x){ ++ring_n; ring_cuts+=fabsf(x)>7.999f; if(fabsf(x*4096.f)>ring_peak)ring_peak=fabsf(x*4096.f); }\nstatic float peak')
s=s.replace('  prepare(bpm); select_suffix();','''  const char *stress=getenv("SJ6_STRESS");
  if(stress) {
    word[32]=word[33]=word[205]=0; word[1]=1.f;word[3]=word[201]=word[202]=0;
    word[208]=0;word[40]=strstr(stress,"classic")!=0;word[41]=0;
    present[42]=present[43]=present[44]=0;
  }
  prepare(bpm); select_suffix();
  if(stress) {
    if(strstr(stress,"legacy")) part.dsp.juno.chorus.classic_native.typed=0;
    juno6_t *j=&part.dsp.juno.synth;
    for(unsigned k=0;k<2;k++){
      dnbx_layer_params lp;
      assert(!dnbx_get_layer(&j->core,k,&lp));
      lp.waveform=0; lp.pan=0; lp.gain=1; lp.cutoff_hz=20000;lp.resonance=0;
      lp.sub_gain=0;lp.filter_env_hz=0;lp.oscillator_mode=DNBX_OSC_SUPERJ6;
      lp.amp.attack_s=.002f;lp.amp.decay_s=.002f;lp.amp.sustain=1;lp.amp.release_s=.008f;
      assert(!dnbx_set_layer(&j->core,k,&lp));
    }
    const unsigned keys[]={57,52,55,60,64,67};
    const unsigned count=stress[strlen(stress)-1]-'0';
    for(unsigned k=0;k<count;k++)
        assert(original_runtime_key(&part,ORIGINAL_INPUT_USB,keys[k],127)==ORIGINAL_RUNTIME_OK);
    assert(dnbx_active_voices(&j->core)==count);
    part.dsp.juno.chorus.mix=.5f;
    float l[128],r[128];
    for(unsigned k=0;k<80;k++){
      assert(original_runtime_render(&part,&work,l,r,128)==ORIGINAL_RUNTIME_OK);
      assert(dnbx_active_voices(&j->core)==count);
      for(unsigned n=0;n<128;n++) {assert(isfinite(l[n])&&isfinite(r[n])); fwrite(&l[n],4,1,out);fwrite(&r[n],4,1,out);}
    }
    goto report;
  }''')
s=s.replace('  fprintf(stderr,"gain','report:\n  fprintf(stderr,"classic_ring_words=%u saturated=%u peakCode=%g\\n",ring_n,ring_cuts,ring_peak);\n  fprintf(stderr,"gain')
s=s.replace('assert(total==148684);','assert(stress || total==148684);')
(out/'probe.c').write_text(s)
