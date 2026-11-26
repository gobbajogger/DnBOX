"""One independently supported, fixed physical Normal20k/Q1 receiver arm.
This is not the general Normal cutoff/envelope-law implementation."""
from pathlib import Path
import hashlib,sys
p,q=map(Path,sys.argv[1:])
b=p.read_bytes()
assert hashlib.sha256(b).hexdigest()=='e31bc550a857ff50a8e30f9eb07176ba6ae0263299a4da8edbc863ea991fb2da'
s=b.decode()
a='''                {
                lv->filter_first=fm_clampf(lv->filter_first+a*(x-lv->filter_first-p->resonance*.8f*lv->filter_second),-4,4);'''
b='''                {
                /* Settled physical Normal, authored fully open and no destination
                 * moves the cutoff. Retain the existing fade/state for all other
                 * controls. No effect order or shared output gains changed. */
                if(amp_before && p->cutoff_hz>=20000.f && p->cutoff_hz<=20000.002f
                    && p->resonance==0.f && authored_strength && strength==0.f &&
                    !authored_track && s->sample_rate==44100.f){
                    const double b0=0x1.851d9c7caa4aap-1,b1=0x1.851d9c7caa4aap+0;
                    const double a1=0x1.7cb2883c57ccap+0,a2=0x1.1b116179f9916p-1;
                    const float y=(float)fma(-a2,lv->ota_tail[1],fma(-a1,lv->ota_tail[0],
                                 fma(b0,lv->filter_second,fma(b0,x,b1*lv->filter_first))));
                    lv->filter_second=lv->filter_first;lv->filter_first=x;
                    lv->ota_tail[1]=lv->ota_tail[0];lv->ota_tail[0]=y;
                    mix[l]+=y;
                } else {
                lv->filter_first=fm_clampf(lv->filter_first+a*(x-lv->filter_first-p->resonance*.8f*lv->filter_second),-4,4);'''
assert s.count(a)==1
s=s.replace(a,b)
a='''                mix[l]+=(amp_before?1.f:output_amp)*lv->filter_second;
                }'''
assert s.count(a)==1
s=s.replace(a,a+'\n                }')
q.write_text(s)
