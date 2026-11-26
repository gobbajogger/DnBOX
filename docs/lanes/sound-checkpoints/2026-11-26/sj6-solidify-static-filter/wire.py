from pathlib import Path
import sys
p,t,q=map(Path,sys.argv[1:])
s=p.read_text()
m=__import__('json').loads(t.read_text())
r=m['cases'][0]['receipts'][0][2][:5]
assert all(l[2][:5]==l[4][:5]==r for case in m['cases'] for l in case['receipts'])
a='                lv->filter_first=fm_clampf(lv->filter_first+a*(x-lv->filter_first-p->resonance*.8f*lv->filter_second),-4,4);'
assert s.count(a)==1
coeff=', '.join(k+'='+v.hex() for k,v in zip(['b0','b1','b2','a1','a2'],r))
s=s.replace(a,'''                if (HOST_EXACT_OPEN && s->sample_rate==44100.f && s->superj6_amp_range &&
                    p->oscillator_mode==DNBX_OSC_SUPERJ6 && p->waveform==0.f && p->resonance==0.f &&
                    p->filter_env_hz==0.f && !authored_track && authored_strength && strength==0.f &&
                    p->cutoff_hz>=20000.f && p->cutoff_hz<=20000.002f){
                    const double '''+coeff+''';
                    const float ctl=velocity*amp;
                    const float input=HOST_EXACT_OPEN==2?ctl*x:x;
                    const float y=(float)fma(-a2,lv->ota_tail[1],fma(-a1,lv->ota_tail[0],
                                 fma(b2,lv->filter_second,fma(b0,input,b1*lv->filter_first))));
                    lv->filter_second=lv->filter_first;
                    lv->filter_first=input;
                    lv->ota_tail[1]=lv->ota_tail[0];
                    lv->ota_tail[0]=y;
                    mix[l]+=HOST_EXACT_OPEN==2?y:ctl*y;
                } else {
'''+a)
a='                mix[l]+=velocity*amp*lv->filter_second;'
assert s.count(a)==1
s=s.replace(a,a+'\n                }')
q.write_text(s)
