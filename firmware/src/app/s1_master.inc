/* SPDX-License-Identifier: GPL-3.0-only */
/* Original AudioKit Synth One masterVolume and independent Soundpipe Faust
 * compressor detectors. The five compressor controls change immediately.
 * Only masterVolume traverses the kernel's 0.1 second parameter port. */
float s1_master_control(unsigned slot,unsigned pot)
{
    float q=(float)pot;
    switch(slot) {
    case 0: return pot<=32 ? q/64.f : .5f+(q-32.f)*(1.5f/95.f);
    case 1: return 1.f+q*(19.f/127.f);
    case 2: return pot<=108 ? -60.f+q*(51.f/108.f) : -9.f+(q-108.f)*(9.f/19.f);
    case 3: return pot<=13 ? q*(.001f/13.f) : .001f+(q-13.f)*(.009f/114.f);
    case 4: return pot<=38 ? q*(.15f/38.f) : .15f+(q-38.f)*(.35f/89.f);
    default: return pot<=18 ? .5f+q*(.5f/18.f) : pot<=54 ? 1.f+(q-18.f)/36.f : 2.f+(q-54.f)*(2.f/73.f);
    }
}

/* The original's zero attack or release gives exp(-infinity) = 0. */
static float master_coefficient(float numerator,float seconds)
{
    return seconds>0.f ? fm_expf(numerator/seconds) : 0.f;
}
void s1_master_targets(s1_master *s,const float values[S1_MASTER_CONTROLS])
{
    s->target_volume=values[0];
    s->attack=master_coefficient(-1.f/44100.f,values[3]);
    s->release=master_coefficient(-1.f/44100.f,values[4]);
    s->gain_smooth=master_coefficient(-2.f/44100.f,values[3]);
    s->ratio_scale=(1.f-s->gain_smooth)*(1.f/values[1]-1.f);
    s->threshold=values[2];
    s->linear_threshold=fm_db2lin(values[2]);
    s->makeup=values[5];
}
static void master_param(s1_master *s,unsigned slot,unsigned q)
{
    static const uint8_t defaults[S1_MASTER_CONTROLS]={32,0,127,0,0,18};
    if (!s->active) memcpy(s->knobs,defaults,sizeof defaults);
    s->knobs[slot]=(uint8_t)q; s->active|=(uint8_t)(1u<<slot);
    float physical[S1_MASTER_CONTROLS];
    for(unsigned k=0;k<S1_MASTER_CONTROLS;++k) physical[k]=s1_master_control(k,s->knobs[k]);
    s1_master_targets(s,physical);
}
void s1_master_release(s1_master *s,unsigned slot,unsigned default_pot)
{
    if (slot>=S1_MASTER_CONTROLS) return;
    unsigned mask=s->active & ~(1u<<slot);
    if (mask==s->active) return;
    if (!mask) { memset(s,0,sizeof *s); return; }
    master_param(s,slot,default_pot);
    s->active=(uint8_t)mask;
}
/* Centered eighth-mantissa segments bound the logarithm polynomial input. The target avoids reciprocal estimates on the audio path. */
static float master_log2(float input)
{
    static const struct { float inverse, log; } segment[8]={
        {0x1.e1e1e20000000p-1f,0x1.663f700000000p-4f},
        {0x1.af286c0000000p-1f,0x1.fbc16c0000000p-3f},
        {0x1.8618620000000p-1f,0x1.91bba80000000p-2f},
        {0x1.642c860000000p-1f,0x1.0c10500000000p-1f},
        {0x1.47ae140000000p-1f,0x1.49a7840000000p-1f},
        {0x1.2f684c0000000p-1f,0x1.82809e0000000p-1f},
        {0x1.1a7b960000000p-1f,0x1.b749480000000p-1f},
        {0x1.0842100000000p-1f,0x1.e88c6c0000000p-1f}
    };
    fm_bits_t word={.f=input};
    int exponent=(int)(word.u>>23)-127;
    unsigned k=(word.u>>20)&7u;
    word.u=(word.u&0x7fffffu)|0x3f800000u;
    float x=word.f*segment[k].inverse-1.f;
    float log=x*(1.44269504f+x*(-.72134752f+x*(.48089835f+x*(-.36067376f+x*.28853901f))));
    return (float)exponent+(segment[k].log+log);
}
static float master_gain(float db)
{
    static const uint32_t power[8]={
        0x3f800000u,
        0x3f8b95c2u,
        0x3f9837f0u,
        0x3fa5fed7u,
        0x3fb504f3u,
        0x3fc5672au,
        0x3fd744fdu,
        0x3feac0c7u
    };
    float octave=db*(0.05f*3.32192809489f);
    fm_bits_t nearest={.f=octave*8.f+0x1.8p23f};
    int ticks=(int)(nearest.u-0x4b400000u);
    float x=octave-(float)ticks*(1.f/8.f);
    fm_bits_t result={.u=power[(unsigned)ticks&7u]+(((unsigned)ticks>>3)<<23)};
    return result.f*(1.f+x*(.69314718f+x*(.24022651f+x*.05550411f)));
}

static void master_stereo(s1_master *s,float *left,float *right,unsigned frames)
{
    if (!s->active) return;
    float volume=s->volume, target=s->target_volume;
    float pregain=2.f*volume;
    if (pan_port(volume,target)!=volume) {
        for(unsigned i=0;i<frames;++i) {
            volume=pan_port(volume,target);
            left[i]*=2.f*volume;right[i]*=2.f*volume;
        }
        s->volume=volume;
        pregain=1.f;
    }
    const float attack=s->attack, release=s->release;
    const float rise=1.f-attack, fall=1.f-release;
    const float threshold=s->threshold, linear=s->linear_threshold;
    const float ratio=s->ratio_scale, smooth=s->gain_smooth, makeup=s->makeup;
    for(unsigned ch=0;ch<2;++ch) {
        float *buffer=ch?right:left;
        float level=s->detect[ch], gain=s->gain_db[ch];
        for(unsigned i=0;i<frames;++i) {
            float input=buffer[i]*pregain, magnitude=fm_fabsf(input);
            level=level>magnitude ? level*release+fall*magnitude : level*attack+rise*magnitude;
            float over=ratio!=0.f && level>linear ? 6.020599913f*master_log2(level)-threshold : 0.f;
            gain=smooth*gain+ratio*(over>0.f?over:0.f);
            /* Below this reduction the correctly rounded input is unchanged
             * at every binary32 mantissa, including a decaying silent tail. */
            buffer[i]=(gain < -0x1p-22f ? master_gain(gain)*input : input)*makeup;
        }
        s->detect[ch]=level;s->gain_db[ch]=gain;
    }
}
