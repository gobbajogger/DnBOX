from pathlib import Path
import sys
t=Path(sys.argv[1])
def put(file,a,b):
 p=t/file;s=p.read_text();assert s.count(a)==1,(file,a);p.write_text(s.replace(a,b))
put('dnbx_core.h','#include "warm_amp.h"','#include "warm_amp.h"\n#if defined(DNBOX_SUB_OWNER_WITNESS)\n#include "sub_owner.h"\n#endif')
a='        unsigned enabled, seen, pending;'
put('dnbx_core.h',a,a+'''
#if defined(DNBOX_SUB_OWNER_WITNESS)
        sj6_warm_amp sub_amp;
        dnbox_sub_owner sub;
        uint64_t sub_commit;
        unsigned sub_enabled;
#endif''')
a='        s->main_oracle.pending_velocity=v->velocity;'
put('dnbx_core.c',a,'''
#if defined(DNBOX_SUB_OWNER_WITNESS)
        if(s->main_oracle.sub_enabled){
            int active=s->main_oracle.sub_amp.stage!=0;
            dnbox_sub_offer(&s->main_oracle.sub,(uint8_t)note,active);
            if(active)sj6_amp_restart(&s->main_oracle.sub_amp);
            else sj6_amp_on(&s->main_oracle.sub_amp);
        }
#endif
'''+a)
a='            sj6_amp_off(&s->main_oracle.amp[l]);'
put('dnbx_core.c',a,'''
#if defined(DNBOX_SUB_OWNER_WITNESS)
            if(!l&&s->main_oracle.sub_enabled){
                sj6_amp_off(&s->main_oracle.sub_amp);
                dnbox_sub_cancel(&s->main_oracle.sub);
            }
#endif
'''+a)
a='                if(!l) {\n                    if(s->sj6_noise_gain)'
put('dnbx_core.c',a,'''                if(!l) {
#if defined(X0X_HOST) && defined(DNBX_HOST_MAIN_SOURCE_ORACLE) && defined(DNBOX_SUB_OWNER_WITNESS)
                    if(!i&&s->main_oracle.enabled&&s->main_oracle.sub_enabled&&s->main_oracle.sub.active){
                        const uint64_t f=s->main_oracle.pos+k;
                        const unsigned prep=sj6_source_prep_frames(s->main_oracle.cb,f);
                        if(prep&&sj6_amp_prep(&s->main_oracle.sub_amp,prep)
                           &&dnbox_sub_commit(&s->main_oracle.sub))s->main_oracle.sub_commit=f;
                        (void)sj6_amp_sample(&s->main_oracle.sub_amp);
                    }
#endif
                    if(s->sj6_noise_gain)''')
a='                        x+=s->sj6_sub_gain*sj6_synthesize_sub(v->sj6_sub_phase,step,sj6_harmonics[i]);'
put('dnbx_core.c',a,'''#if defined(X0X_HOST) && defined(DNBX_HOST_MAIN_SOURCE_ORACLE) && defined(DNBOX_SUB_OWNER_WITNESS)
                        if(!i&&s->main_oracle.enabled&&s->main_oracle.sub_enabled&&s->main_oracle.sub.active){
                            dnbox_sub_owner *sub=&s->main_oracle.sub;
                            x+=s->sj6_sub_gain*sj6_synthesize_sub(sub->phase,step,sj6_sub_harmonic[sj6_sub_zone(sub->active_key)]);
                            dnbox_sub_advance(sub,step);
                        }else
#endif
                        {
'''+a)
a='                        if(v->sj6_sub_phase>=1.f) v->sj6_sub_phase-=1.f;'
put('dnbx_core.c',a,a+'\n                        }')
a='    if(oracle) {\n        assert(dnbx_host_main_oracle_setup(&s,capture_amp)==DNBX_OK);'
put('replay_source_bus_tap.c',a,a+'''
        if(getenv("SUB_TAPE")){
            const uint8_t levels[2]={0,92};
            dnbx_set_sj6_sources(&s,0,levels);
            if(getenv("SUB_OWNER")){
                s.main_oracle.sub_enabled=1;
                sj6_amp_config_panel(&s.main_oracle.sub_amp,44100,capture_amp[0][0],
                   capture_amp[0][1],capture_amp[0][2],capture_amp[0][3]);
            }
        }''')
a='    assert(t==148684 && e==sizeof capture_events/sizeof *capture_events);'
put('replay_source_bus_tap.c',a,a+'''
    if(oracle && getenv("SUB_OWNER")){
        assert(s.main_oracle.sub_commit==49875);
        assert(s.main_oracle.sub.active_key==67);
        fprintf(trace,"Sub owner commit=%llu active=%u\\n",(unsigned long long)s.main_oracle.sub_commit,s.main_oracle.sub.active_key);
    }
''')
p=t/'replay_source_bus_tap.c';p.write_text('#include <stdlib.h>\n'+p.read_text())
