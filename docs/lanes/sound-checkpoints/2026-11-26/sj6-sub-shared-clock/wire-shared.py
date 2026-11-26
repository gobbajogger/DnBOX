from pathlib import Path
import sys
t=Path(sys.argv[1])
def change(file,a,b):
 p=t/file;s=p.read_text();assert s.count(a)==1,(file,a);p.write_text(s.replace(a,b))
change('dnbx_core.h','        sj6_warm_amp sub_amp;\n','')
change('dnbx_core.c','''            int active=s->main_oracle.sub_amp.stage!=0;
            dnbox_sub_offer(&s->main_oracle.sub,(uint8_t)note,active);
            if(active)sj6_amp_restart(&s->main_oracle.sub_amp);
            else sj6_amp_on(&s->main_oracle.sub_amp);''','''            /* Owned co-timed shared AMP state. Admitted before the first note. */
            const int active=s->main_oracle.amp[0].stage!=0;
            dnbox_sub_offer(&s->main_oracle.sub,(uint8_t)note,active);''')
change('dnbx_core.c','                sj6_amp_off(&s->main_oracle.sub_amp);\n','')
change('dnbx_core.c','''                    if(!i&&s->main_oracle.enabled&&s->main_oracle.sub_enabled&&s->main_oracle.sub.active){
                        const uint64_t f=s->main_oracle.pos+k;
                        const unsigned prep=sj6_source_prep_frames(s->main_oracle.cb,f);
                        if(prep&&sj6_amp_prep(&s->main_oracle.sub_amp,prep)
                           &&dnbox_sub_commit(&s->main_oracle.sub))s->main_oracle.sub_commit=f;
                        (void)sj6_amp_sample(&s->main_oracle.sub_amp);
                    }''','''                    if(!i&&s->main_oracle.enabled&&s->main_oracle.sub_enabled&&s->main_oracle.sub.pending){
                        const uint64_t f=s->main_oracle.pos+k;
                        if(s->main_oracle.commit[0]==f&&dnbox_sub_commit(&s->main_oracle.sub))
                            s->main_oracle.sub_commit=f;
                    }''')
change('replay_source_bus_tap.c','''            if(getenv("SUB_OWNER")){
                s.main_oracle.sub_enabled=1;
                sj6_amp_config_panel(&s.main_oracle.sub_amp,44100,capture_amp[0][0],
                   capture_amp[0][1],capture_amp[0][2],capture_amp[0][3]);
            }''','''            /* This exact diagnostic binding admits velocity, stage routing,
               co-timed Main/Sub input and receipt-confirmed shared AMP
               physical words. It is not a runtime auto-detection heuristic. */
            if(getenv("SUB_OWNER")&&!getenv("SUB_UNQUALIFIED"))s.main_oracle.sub_enabled=1;''')
change('replay_source_bus_tap.c','if(oracle && getenv("SUB_OWNER")){','if(oracle && getenv("SUB_OWNER")&&!getenv("SUB_UNQUALIFIED")){')
