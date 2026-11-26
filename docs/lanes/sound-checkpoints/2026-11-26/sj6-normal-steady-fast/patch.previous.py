from pathlib import Path
import sys
d=Path(sys.argv[1])
h=d/'dnbx_core.h'; p=d/'dnbx_core.c'
s=h.read_text();a='    uint8_t damper, velocity_mode, superj6_amp_range, filter_link;'
b='''#if defined(X0X_HOST) && DNBX_HOST_SJ6_FILTER_MODE && X0X_SJ6_OTA_NATIVE
    struct {
        float gain, step;
        float b0, a1, a2; /* fixture supplied; no firmware coefficient law */
        uint8_t armed, request, in_tile;
    } sj6_mode_test;
#endif
'''+a
assert s.count(a)==1;h.write_text(s.replace(a,b))
s=p.read_text()
a='    if(s->sj6_ota==value)return 1;'
b='''#if defined(X0X_HOST) && DNBX_HOST_SJ6_FILTER_MODE
    if(s->sj6_mode_test.armed){s->sj6_mode_test.request=(uint8_t)value;return 1;}
#endif
'''+a
assert s.count(a)==1;s=s.replace(a,b)
i=s.index('static int render_with_osc_gains')
a='    for (unsigned k=0;k<n;k++) {'
if a not in s[i:]:
 a='    for(unsigned k=0;k<n;k++) {'
 if a not in s[i:]:
  a='    for (unsigned k = 0; k < n; ++k) {'
b=a+'''
#if defined(X0X_HOST) && DNBX_HOST_SJ6_FILTER_MODE && X0X_SJ6_OTA_NATIVE
        if(s->sj6_mode_test.armed && s->sj6_mode_test.in_tile==0u &&
           s->sj6_ota!=s->sj6_mode_test.request && s->sj6_mode_test.gain<=0.f){
            s->sj6_ota=s->sj6_mode_test.request;
            for(unsigned q=0;q<DNBX_VOICES;q++)for(unsigned l=0;l<DNBX_LAYERS;l++){
                dnbx_layer_voice *lv=&s->voice[q].layer[l];
                lv->filter_first=lv->filter_second=lv->ota_tail[0]=lv->ota_tail[1]=0.f;
            }
        }
#endif'''
assert a in s[i:];s=s[:i]+s[i:].replace(a,b,1)
a='                lv->filter_first=fm_clampf(lv->filter_first+a*'
b='''#if defined(X0X_HOST) && DNBX_HOST_SJ6_FILTER_MODE && X0X_SJ6_OTA_NATIVE
                if(s->sj6_mode_test.armed){
                    const float x1=lv->filter_first,x2=lv->filter_second;
                    const float y1=lv->ota_tail[0],y2=lv->ota_tail[1];
                    const float y=(s->sj6_mode_test.b0*((x+x2)+(x1+x1))-s->sj6_mode_test.a1*y1)-s->sj6_mode_test.a2*y2;
                    lv->filter_first=x;lv->filter_second=x1;
                    lv->ota_tail[0]=y;lv->ota_tail[1]=y1;
                    mix[l]+=velocity*amp*y;
                } else
#endif
                {
'''+a
assert s.count(a)==1;s=s.replace(a,b)
a='                mix[l]+=velocity*amp*lv->filter_second;'
assert s.count(a)==1;s=s.replace(a,a+'\n                }')
a='            left[k]+=x*(.5f-.5f*p->pan); right[k]+=x*(.5f+.5f*p->pan);\n        }'
b=a+'''
#if defined(X0X_HOST) && DNBX_HOST_SJ6_FILTER_MODE && X0X_SJ6_OTA_NATIVE
        if(s->sj6_mode_test.armed){
            float g=s->sj6_mode_test.gain;
            if(s->sj6_ota!=s->sj6_mode_test.request)g=fm_maxf(0.f,g-s->sj6_mode_test.step);
            else if(g<1.f)g=fm_minf(1.f,g+s->sj6_mode_test.step);
            s->sj6_mode_test.gain=g;
            left[k]*=g;right[k]*=g;
            s->sj6_mode_test.in_tile=(uint8_t)((s->sj6_mode_test.in_tile+1u)&15u);
        }
#endif'''
assert s.count(a)==1;s=s.replace(a,b)
p.write_text(s)
q=d/'juno6_factory.inc'
s=q.read_text()
arm='''#if defined(X0X_HOST) && DNBX_HOST_SJ6_FILTER_MODE && X0X_SJ6_OTA_NATIVE
 if(s->core.sj6_mode_test.armed)s->core.sj6_mode_test.in_tile=0;
#endif'''
a='''int juno6_render_stereo(juno6_t *s,float *left,float *right,unsigned frames)
#endif
{'''
assert s.count(a)==1
s=s.replace(a,a+'\n#if !X0X_SJ6_LFO1_DELAY_PORT\n'+arm+'\n#endif')
a='''int juno6_render_stereo(juno6_t *s,float *left,float *right,unsigned frames)
{
    if (s->typed && !s->lfo1_delay_started'''
assert s.count(a)==1
s=s.replace(a,a.replace('\n    if','\n'+arm+'\n    if'))
a='void juno6_sj6_set_ota(juno6_t *s,unsigned mode)\n{'
assert s.count(a)==1
s=s.replace(a,a+'''
#if defined(X0X_HOST) && DNBX_HOST_SJ6_FILTER_MODE
    /* Diagnostic receiver only. No application cutoff/ADSR mapping change. */
    if(s->core.sj6_mode_test.armed){(void)dnbx_set_sj6_ota(&s->core,mode);return;}
#endif''')
q.write_text(s)
