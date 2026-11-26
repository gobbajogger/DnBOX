import hashlib,difflib,sys
from pathlib import Path
base=Path(sys.argv[1]);d=Path(__file__).parent
patch=[]
for f in ('dnbx_core.h','dnbx_core.c'):
 p=base/f; old=p.read_text();s=old
 def rep(a,b):
  global s
  assert s.count(a)==1
  s=s.replace(a,b)
 if f.endswith('.h'):
  rep('    uint8_t damper, velocity_mode, superj6_amp_range, filter_link;', '''    uint8_t damper, velocity_mode, superj6_amp_range, filter_link;
#if defined(X0X_HOST) && DNBX_HOST_SJ6_MONO_SOURCE_ROUTE
    uint8_t sj6_host_mono_source; /* diagnostic owner only, zero when initialized */
#endif''')
 else:
  rep('    const float osc_gain[]={gain1,gain2};','''    const float osc_gain[]={gain1,gain2};
#if defined(X0X_HOST) && DNBX_HOST_SJ6_MONO_SOURCE_ROUTE
    int source_out=-1;
    if(s->sj6_host_mono_source && s->superj6_amp_range && s->sample_rate==44100.f &&
       fvel==0.f && source_bend==1.f && voice_active(&s->voice[0]) &&
       dnbx_active_voices(s)==1) {
        float strength=0.f, tracking=0.f;
        (void)dnbx_read_raw(s,136u,&strength);
        (void)dnbx_read_raw(s,131u,&tracking);
#if X0X_SJ6_NOISE_SUB_NATIVE
        if(s->sj6_noise_gain==0.f && s->sj6_sub_gain==0.f)
#endif
#if X0X_SJ6_OTA_NATIVE
        if(!s->sj6_ota)
#endif
#if X0X_SJ6_MONO32_PROBE
        if(s->sj6_glide_offset==0.f)
#endif
#if X0X_SJ6_PITCH_ENV_NATIVE
        if(!s->sj6_pitch_depth
#if X0X_SJ6_AUX_LFO_NATIVE
           && !s->sj6_pitch_routed
#endif
          )
#endif
        if(strength==0.f && tracking==0.f)
            for(unsigned q=0;q<DNBX_LAYERS;q++) {
                const dnbx_layer_params *a=&s->layer[q].params,*b=&s->layer[q^1u].params;
                const dnbx_layer_voice *other=&s->voice[0].layer[q^1u];
                /* Muting an input is not proof its old filter output is zero. */
                if(osc_gain[q]>0.f && osc_gain[q^1u]==0.f &&
                   a->oscillator_mode==DNBX_OSC_SUPERJ6 && a->waveform==0.f &&
                   a->pan==0.f && a->gain==1.f && a->sub_gain==0.f &&
                   b->sub_gain==0.f && a->resonance==0.f && a->filter_env_hz==0.f &&
                   other->filter_first==0.f && other->filter_second==0.f)
                    source_out=(int)q;
            }
    }
#endif''')
  rep('        left[k]=0; right[k]=0;', '''#if defined(X0X_HOST) && DNBX_HOST_SJ6_MONO_SOURCE_ROUTE
        if(source_out>=0) {
            left[k]=right[k]=mix[source_out];
            continue;
        }
#endif
        left[k]=0; right[k]=0;''')
 n='firmware/src/dsp/'+f
 patch+=list(difflib.unified_diff(old.splitlines(True),s.splitlines(True),'a/'+n,'b/'+n))
 print(hashlib.sha256(p.read_bytes()).hexdigest(),n)
(d/'candidate.patch').write_text(''.join(patch))
