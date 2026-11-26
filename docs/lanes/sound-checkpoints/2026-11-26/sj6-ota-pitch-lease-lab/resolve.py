import sys
from pathlib import Path
p=Path(sys.argv[1]);st=p.read_text(); import hashlib
sites={'    if(routed && !s->sj6_pitch_routed && !s->sj6_pitch_depth)':'    assert(!routed);\n',
       '    if(code[4]>60u)return 0;':'    assert(!code[4]);\n',
       '      )envelope_on(&v->sj6_pitch,&s->sj6_pitch_param,s->sample_rate);':'      )assert(!"PitchEnv on while reserved");\n',
       '      )envelope_off(&v->sj6_pitch,&s->sj6_pitch_param,s->sample_rate);':'      )assert(!"PitchEnv off while reserved");\n',
       '                float env=envelope_sample(&v->sj6_pitch,&s->sj6_pitch_param,s->sample_rate,w);':'                assert(!"PitchEnv render while reserved");\n'}
for a,b in sites.items():
 assert st.count(a)==1,a
 if a.startswith('      )envelope_'):st=st.replace(a,b)
 else:st=st.replace(a,b+a)
p.write_text(st)
st=p.read_text()
a='                if(s->sj6_ota){'
assert st.count(a)==1
st=st.replace(a,'''                if(ota_arm && l==0 && s->sj6_ota){
                    static int marked=0;
                    if(!marked){ fprintf(stderr,"OTA witness base %.9g env %.9g amount %.9g rawHz %.9g a %.9g rate %g resonance %.9g oscm %u osc %g pitch %u/%u\\n",base,env,amount,base+env*amount,a,s->sample_rate,p->resonance,p->oscillator_mode,p->waveform,s->sj6_pitch_depth,s->sj6_pitch_routed); marked=1; }
                    assert(amount==0.f && base+env*amount==30000.00390625f);
                    assert(s->sample_rate==44100.f && p->resonance==0.f &&
                         p->oscillator_mode==DNBX_OSC_SUPERJ6 && p->waveform==2.f/3.f &&
                         !s->sj6_pitch_depth && !s->sj6_pitch_routed && !s->sj6_noise_gain && !p->sub_gain);
                    assert(main_in>=-1.191f && main_in<=1.191f);
                    if(osc_gain[0]>0.f) assert(osc_gain[0]==.808f);
                }
''' + a)
p.write_text('#include <stdio.h>\n'+st)
