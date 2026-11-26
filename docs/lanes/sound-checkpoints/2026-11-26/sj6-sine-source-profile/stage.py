from pathlib import Path
import sys
p=Path(sys.argv[1]); s=p.read_text()
a='    const float osc_gain[]={gain1,gain2};'
b='''#if defined(X0X_HOST) && DNBX_HOST_SJ6_SINE_PROFILE
    /* Source calibration for generated typed Sine only. Bake into existing
     * receiver gain before sample loops (outside oscillator and filter). */
    float osc_gain[]={gain1,gain2};
    if(s->superj6_amp_range) for(unsigned l=0;l<DNBX_LAYERS;l++) {
        const dnbx_layer_params *p=&s->layer[l].params;
        if(p->oscillator_mode==DNBX_OSC_SUPERJ6 && p->waveform==0.f && osc_gain[l]<=.808f)
            osc_gain[l]*=0x1.3cdfdep+0f;
    }
#else
    const float osc_gain[]={gain1,gain2};
#endif'''
assert s.count(a)==1;s=s.replace(a,b)
p.write_text(s)
