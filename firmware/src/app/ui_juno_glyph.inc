/* SPDX-License-Identifier: GPL-3.0-only */
/* Source-linked Juno curves. Wave uses the production carrier equation;
 * filter plots are its actual discrete recurrence at a fixed44100Hz rate.
 * These phase/frequency response cues are not an audio oscilloscope. */
static float juno_q(pref_t r,int id,int selected,float value)
{
    return id==selected?value/127.f:(float)matrix_value(PR(R_ENG,T_303,r.b,id))/127.f;
}
static float juno_filter_mag(float a,float resonance,float radians)
{
    float z=1.f-a,b=-2.f*z+a*a*resonance*.8f,c=z*z;
    float re=1.f+b*fm_cosf(radians)+c*fm_cosf(2.f*radians);
    float im=-b*fm_sinf(radians)-c*fm_sinf(2.f*radians);
    return a*a/fm_sqrtf(fm_maxf(1e-12f,re*re+im*im));
}
static UI_DRAW void juno_strip_glyph(pref_t r,int value,int gx,int gy,uint16_t col)
{
    int id=r.c;float q=(float)value/127.f,xy[66];
    float phase=0,bipolar=0,current=0,target=0;int active=0;
    (void)engine_bass_motion_view(r.b,&phase,&bipolar,&current,&target,&active);
    if(id==15) {
        for(int i=0;i<6;i++) {
            float h=2.f+(float)i*2.5f;
            cv_stroke_aa(gx+2.f+i*4.f,gy+16.f,gx+2.f+i*4.f,gy+16.f-h,2.f,
                         q>=(float)(i+1)/6.f?col:SOUND_LINE);
        }
        return;
    }
    for(int i=0;i<=32;i++) {
        float t=(float)i/32.f,x=gx+25.f*t,y=gy+9.f;
        if(id==0||id==1||id==12) {
            float cyc=2.f*t,p=cyc-fm_floorf(cyc),sub=.5f*cyc;
            sub-=fm_floorf(sub);
            float morph=juno_q(r,0,id,value),width=.1f+.8f*juno_q(r,12,id,value);
            float sublevel=.5f*juno_q(r,1,id,value);
            float wave=id==1?(sub<.5f?1.f:-1.f)*sublevel:
                juno6_wave_value(p,261.625565f/44100.f,width,sub,morph,sublevel);
            y-=wave*6.f;
        } else if(id==2||id==3||id==4||id==9) {
            float hz=35.f*fm_exp2f(t*9.1597467f),w=6.28318531f*hz/44100.f,mag;
            if(id==9) {
                float hp=1.f/(1.f+6.28318531f*(15.f+2000.f*q*q)/44100.f);
                mag=hp*fm_sqrtf(fm_maxf(0.f,2.f-2.f*fm_cosf(w)))/
                    fm_sqrtf(fm_maxf(1e-12f,1.f+hp*hp-2.f*hp*fm_cosf(w)));
            } else {
                float cut=juno_q(r,2,id,value),res=juno_q(r,3,id,value);
                float env=juno_q(r,4,id,value),level=0,mod=0;unsigned state=6,held=0;
                if(!engine_bass_voice_view(r.b,&level,&mod,&state,&held)||state==6)level=0;
                if(id==4)level=1; /* maximum envelope response, not a fabricated hit */
                float a=6.28318531f*(35.f+cut*cut*12000.f+level*7000.f*env)/44100.f;
                a=fm_minf(.75f,fm_maxf(.001f,a));mag=juno_filter_mag(a,res,w);
            }
            y=gy+3.f-3.f*fm_log2f(fm_maxf(mag,.04f));
        } else if(id>=5&&id<=8) {
            float a=juno_q(r,5,id,value),d=juno_q(r,6,id,value),s=juno_q(r,7,id,value),re=juno_q(r,8,id,value);
            a=.002f+a*a*3.f;d=.01f+d*d*3.f;re=.01f+re*re*4.f;
            float seconds=t*4.f,level;
            if(id==5)level=fm_minf(1.f,seconds/a);
            else if(id==6)level=1.f-(1.f-s)*fm_minf(1.f,seconds/d);
            else if(id==7)level=s;
            else level=s*fm_powf(1.f-1.f/(re*44100.f),seconds*44100.f);
            y=gy+16.f-level*14.f;
        } else if(id==10||id==11) {
            float rate=.1f+8.f*juno_q(r,10,id,value)*juno_q(r,10,id,value);
            float p=t*rate+phase;p-=fm_floorf(p);
            float triangle=1.f-4.f*fm_fabsf(p-.5f);
            y-=triangle*(id==11?q:1.f)*7.f;
        } else { /* chorus delay LFO rate / wet amount; stereo wet polarities */
            float rate=.1f+4.9f*juno_q(r,13,id,value);
            y-=fm_sinf(6.28318531f*t*rate)*(id==14?q:1.f)*6.f;
        }
        if(y<gy+1.f)y=gy+1.f;
        if(y>gy+17.f)y=gy+17.f;
        xy[2*i]=x;xy[2*i+1]=y;
    }
    cv_poly_aa(xy,33,col);
    if(id==14) {
        for(int i=0;i<33;i++)xy[2*i+1]=gy+18.f-xy[2*i+1]+gy;
        cv_poly_aa(xy,33,SOUND_GRAY);
    }
}
