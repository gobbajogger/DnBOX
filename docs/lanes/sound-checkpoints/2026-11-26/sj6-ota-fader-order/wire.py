from pathlib import Path
import sys
base,out=map(Path,sys.argv[1:])
s=base.read_text()
a='                if (osc_gain[l]!=1.f) x*=osc_gain[l];'
assert s.count(a)==1
s=s.replace(a,'''                /* Current source fader is outside its asymmetric OTA.
                 * A shared Sub/noise input cannot be separated here. */
                const int gain_after=HOST_OTA_GAIN_AFTER && s->superj6_amp_range && s->sj6_ota &&
                    p->oscillator_mode==DNBX_OSC_SUPERJ6 &&
                    p->sub_gain==0.f && !s->sj6_noise_gain && !s->sj6_sub_gain;
                if (!gain_after && osc_gain[l]!=1.f) x*=osc_gain[l];''')
a='                if(s->sj6_ota)mix[l]+=velocity*amp*sj6_ota_filter(lv,x,a,p->resonance);'
assert s.count(a)==1
s=s.replace(a,'''                if(s->sj6_ota){
                    float y=sj6_ota_filter(lv,x,a,p->resonance);
                    if(gain_after)y*=osc_gain[l];
                    mix[l]+=velocity*amp*y;
                }''')
out.write_text(s)
