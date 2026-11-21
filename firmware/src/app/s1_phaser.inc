/* SPDX-License-Identifier: GPL-3.0-only */
/* The Synth One kernel fixes the Faust phaser to four notches, a 1.5
 * frequency ratio, 100..800 Hz modulation, unity vibrato, and zero invert.
 * Its exposed controls are crossfade, LFO BPM, feedback and notch width. */
float s1_phaser_control(unsigned slot,unsigned q)
{
    if (slot==0) return (float)q/127.f;
    if (slot==2) return .8f*((float)q/127.f);
    if (slot==1) return q<=5 ? 1.f+(float)q*(11.f/5.f) : 12.f+(float)(q-5u)*(288.f/122.f);
    return q<=99 ? 100.f+(float)q*(700.f/99.f) : 800.f+(float)(q-99u)*(200.f/28.f);
}

/* All original notch angles lie in [-0.58,0.58] at 44.1 kHz. */
static float phaser_tap(float state,float angle)
{
    float x=angle*angle;
    /* Faust evaluates state*cos(angle) in binary64 before storing a float.
     * Retaining the small cosine correction separately avoids rounding the
     * coefficient to binary32 before multiplication. */
    float correction=x*(-.5f+x*(1.f/24.f+x*(-1.f/720.f+x*(1.f/40320.f))));
    return state+state*correction;
}

static float phaser_channel(s1_phaser_channel *s,float input,float lfo,float radius2,float twice_radius,float feedback)
{
    static const float factors[4]={(1.f/44100.f)*1.5f,(1.f/44100.f)*2.25f,(1.f/44100.f)*3.375f,(1.f/44100.f)*5.0625f};
    float frequency=(.5f*((6.28319f*800.f)-(6.28319f*100.f)))*(1.f-lfo)+(6.28319f*100.f);
    float previous_tap=0.f, previous_second=0.f, output=0.f;
    for (unsigned j=0;j<4;++j) {
        float tap=phaser_tap(s->z1[j],factors[j]*frequency);
        float second=s->z2[j];
        if (j==0)
            output=-((twice_radius*tap+radius2*second)-(input+feedback*s->feedback));
        else
            output=twice_radius*(previous_tap-tap)+(previous_second+radius2*(output-second));
        s->z2[j]=s->z1[j];
        s->z1[j]=output;
        previous_tap=tap;
        previous_second=second;
    }
    s->feedback=radius2*output+(twice_radius*previous_tap+previous_second);
    return s->feedback;
}

static void phaser_stereo(s1_phaser *s,float *left,float *right,unsigned n)
{
    if (!s->current[0] && !s->target[0]) {
        /* The source freezes its Faust state at zero mix. Its parameter ports
         * still advance, including edits made while the instrument is silent. */
        for(unsigned k=1;k<4;++k) {
            float value=s->current[k];
            for(unsigned i=0;i<n;++i) {
                float next=pan_port(value,s->target[k]);
                if (next==value) break;
                value=next;
            }
            s->current[k]=value;
        }
        return;
    }
    for(unsigned i=0;i<n;++i) {
        for(unsigned k=0;k<4;++k)s->current[k]=pan_port(s->current[k],s->target[k]);
        float mix=s->current[0];
        if(mix==0.f) continue;
        float angle=(.10472f/44100.f)*s->current[1];
        float sn=angle*(1.f-angle*angle*(1.f/6.f)), cs=1.f-.5f*angle*angle;
        float sine=sn*s->cosine+cs*s->sine;
        float cosine=(1.f+(cs*s->cosine-sn*s->sine))-(float)s->started;
        s->sine=sine; s->cosine=cosine; s->started=1;
        float x=(1.f/44100.f)*(-3.14159f*s->current[3]);
        float radius=1.f+x*(1.f+x*(.5f+x*(1.f/6.f+x*(1.f/24.f+x*(1.f/120.f+x/720.f)))));
        float r2=radius*radius, r_2=-2.f*radius, dry=1.f-mix;
        float l=phaser_channel(&s->channel[0],left[i],sine,r2,r_2,s->current[2]);
        float r=phaser_channel(&s->channel[1],right[i],cosine,r2,r_2,s->current[2]);
        left[i]=dry*left[i]+(1.f-dry)*l;
        right[i]=dry*right[i]+(1.f-dry)*r;
    }
}

int s1_part_effect_tail(const s1_dry_part *part)
{
    if (part->width.pending) return 1;
    if (part->phaser.current[0] == 0.f) return 0;
    /* Below one 24-bit codec count no residual will keep a part audible. */
    for(unsigned ch=0;ch<2;++ch) {
        const s1_phaser_channel *s=&part->phaser.channel[ch];
        for(unsigned j=0;j<4;++j)
            if(fm_fabsf(s->z1[j])>0x1p-24f || fm_fabsf(s->z2[j])>0x1p-24f) return 1;
    }
    return 0;
}
