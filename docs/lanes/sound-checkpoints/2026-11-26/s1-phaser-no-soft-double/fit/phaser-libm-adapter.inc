/* Never-install guarded source-matched phaser ABI witness. */
#include <math.h>
#define FAUSTFLOAT float
#define DNBOX_PH_COEF 1
#define DNBOX_PH_COS 1
#define DNBOX_PH_FASTCTL 1
#define max(a,b) ((a < b) ? b : a)
#define min(a,b) ((a < b) ? a : b)
typedef s1u_phaser phaser;
_Static_assert(sizeof(s1u_phaser)==224,"matched owner size");
static float faustpower2_f(float value) {
	return (value * value);

}
static float faustpower3_f(float value) {
	return ((value * value) * value);

}
static float faustpower4_f(float value) {
	return (((value * value) * value) * value);

}

static void instanceInitphaser(phaser* dsp, int samplingFreq) {
	dsp->fSamplingFreq = samplingFreq;
	dsp->fHslider0 = (FAUSTFLOAT)0.;
	/* C99 loop */
	{
		int i0;
		for (i0 = 0; (i0 < 2); i0 = (i0 + 1)) {
			dsp->iVec0[i0] = 0;

		}

	}
	dsp->fCheckbox0 = (FAUSTFLOAT)0.;
	dsp->fHslider1 = (FAUSTFLOAT)1.;
	dsp->iConst0 = min(192000, max(1, dsp->fSamplingFreq));
	dsp->fConst1 = (1.f / (float)dsp->iConst0);
	dsp->fHslider2 = (FAUSTFLOAT)1000.;
	dsp->fHslider3 = (FAUSTFLOAT)1.5;
	dsp->fHslider4 = (FAUSTFLOAT)100.;
	dsp->fHslider5 = (FAUSTFLOAT)800.;
	dsp->fConst2 = (0.10472f / (float)dsp->iConst0);
	dsp->fHslider6 = (FAUSTFLOAT)30.;
	/* C99 loop */
	{
		int i1;
		for (i1 = 0; (i1 < 2); i1 = (i1 + 1)) {
			dsp->fRec5[i1] = 0.f;

		}

	}
	/* C99 loop */
	{
		int i2;
		for (i2 = 0; (i2 < 2); i2 = (i2 + 1)) {
			dsp->fRec6[i2] = 0.f;

		}

	}
	dsp->fHslider7 = (FAUSTFLOAT)0.;
	/* C99 loop */
	{
		int i3;
		for (i3 = 0; (i3 < 3); i3 = (i3 + 1)) {
			dsp->fRec4[i3] = 0.f;

		}

	}
	/* C99 loop */
	{
		int i4;
		for (i4 = 0; (i4 < 3); i4 = (i4 + 1)) {
			dsp->fRec3[i4] = 0.f;

		}

	}
	/* C99 loop */
	{
		int i5;
		for (i5 = 0; (i5 < 3); i5 = (i5 + 1)) {
			dsp->fRec2[i5] = 0.f;

		}

	}
	/* C99 loop */
	{
		int i6;
		for (i6 = 0; (i6 < 3); i6 = (i6 + 1)) {
			dsp->fRec1[i6] = 0.f;

		}

	}
	/* C99 loop */
	{
		int i7;
		for (i7 = 0; (i7 < 2); i7 = (i7 + 1)) {
			dsp->fRec0[i7] = 0.f;

		}

	}
	dsp->fCheckbox1 = (FAUSTFLOAT)0.;
	/* C99 loop */
	{
		int i8;
		for (i8 = 0; (i8 < 3); i8 = (i8 + 1)) {
			dsp->fRec11[i8] = 0.f;

		}

	}
	/* C99 loop */
	{
		int i9;
		for (i9 = 0; (i9 < 3); i9 = (i9 + 1)) {
			dsp->fRec10[i9] = 0.f;

		}

	}
	/* C99 loop */
	{
		int i10;
		for (i10 = 0; (i10 < 3); i10 = (i10 + 1)) {
			dsp->fRec9[i10] = 0.f;

		}

	}
	/* C99 loop */
	{
		int i11;
		for (i11 = 0; (i11 < 3); i11 = (i11 + 1)) {
			dsp->fRec8[i11] = 0.f;

		}

	}
	/* C99 loop */
	{
		int i12;
		for (i12 = 0; (i12 < 2); i12 = (i12 + 1)) {
			dsp->fRec7[i12] = 0.f;

		}

	}

}

static void initphaser(phaser* dsp, int samplingFreq) {
#if DNBOX_PH_COEF
 dsp->valid=0;
#endif
	instanceInitphaser(dsp, samplingFreq);
}

#if DNBOX_PH_COEF
static unsigned ph_word(float f){unsigned u; __builtin_memcpy(&u,&f,4); return u;}
#if DNBOX_PH_FASTCTL
static float ph_pole(float a){
    const unsigned w=ph_word(a),z=w&0x7fffffffu;
    if((w>>31) && z>=0x00800000u && z<=0x3ec00000u)
        return 1.f+a*(1.f+a*(.5f+a*(0x1.555556p-3f+
                a*(0x1.555556p-5f+a*(0x1.111112p-7f+
                a*(0x1.6c16c2p-10f+a*(0x1.a01a02p-13f+a*0x1.a01a02p-16f)))))));
    return (float)exp((double)a);
}
static void ph_rotation(float a,float *s,float *c){
    const unsigned z=ph_word(a)&0x7fffffffu;
    if(z>=0x00800000u && z<=0x3c800000u){
        const float a2=a*a;
        *s=a+a*a2*(-0x1.555556p-3f+a2*0x1.111112p-7f);
        *c=1.f+a2*(-.5f+a2*0x1.555556p-5f);
    }else{
        *s=(float)sin((double)a);
        *c=(float)cos((double)a);
    }
}
#endif
static void ph_slow(phaser*dsp){
    const float gain=(float)dsp->fHslider0,width=(float)dsp->fHslider2,bpm=(float)dsp->fHslider6;
    const unsigned g=ph_word(gain),w=ph_word(width),b=ph_word(bpm);
    if(!dsp->valid||g!=dsp->tags[0]){
        dsp->gain=pow(10.f,.05f*gain);
        dsp->tags[0]=g;
    }
    if(!dsp->valid||w!=dsp->tags[1]){
        const float a=dsp->fConst1*(0.f-(3.14159f*width));
#if DNBOX_PH_FASTCTL
        dsp->pole=ph_pole(a);
#else
        dsp->pole=exp(a);
#endif
        dsp->tags[1]=w;
    }
    if(!dsp->valid||b!=dsp->tags[2]){
        const float angle=dsp->fConst2*bpm;
#if DNBOX_PH_FASTCTL
        ph_rotation(angle,&dsp->rsin,&dsp->rcos);
#else
        dsp->rsin=sin(angle);dsp->rcos=cos(angle);
#endif
        dsp->tags[2]=b;
    }
    dsp->valid=1;
}
#endif
/* Source notch arguments are already float32. Guard unsupported ranges by
 * exact Float32 exponent/magnitude, including nonzero subnormal inputs;
 * preserve the original double cosine and product outside the proof. */
#if DNBOX_PH_COS_TRACE
#include <stdio.h>
static float peak[8], lower[8];
static unsigned long counts[8], fallback[8];
static void ph_trace_done(void) {
 for(int i=0;i<8;i++) fprintf(stderr,"PHCOS %d %lu %a %a %lu\n",i,counts[i],(double)lower[i],(double)peak[i],fallback[i]);
}
static void ph_trace(int lane,float x,int falls){
 if(counts[0]==0&&lane==0)atexit(ph_trace_done);
 if(!counts[lane]||x<lower[lane])lower[lane]=x;
 if(!counts[lane]||x>peak[lane])peak[lane]=x;
 counts[lane]++;fallback[lane]+=falls;
}
#endif
static __attribute__((noinline)) float ph_cosprod(float v,float x,int lane) {
#if DNBOX_PH_COS || DNBOX_PH_COS_TRACE
 unsigned ux=ph_word(x)&0x7fffffffu, raw=ph_word(v),uv=raw&0x7fffffffu;
 int outside=ux>0x3f200000u || (ux && ux<0x00800000u) || uv>=0x7f800000u;
#if DNBOX_PH_COS_TRACE
 ph_trace(lane,x,outside);
#endif
#if DNBOX_PH_COS
 if(!outside) {
     const float z=x*x;
     const float q=-0.5f+z*(0x1.555556p-5f+z*(-0x1.6c16c2p-10f+z*(0x1.a01a02p-16f+z*-0x1.27e4fcp-22f)));
     const float c=1.f+z*q;
     if(uv<0x01000000u){
         if(!uv)return v; /* cosine positive; preserve signed zero. */
         /* Low-magnitude Float32 payload represents v/2^-149 exactly
          * (both subnormal and first normal bin). Quantize once, ties-even. */
         const unsigned cw=ph_word(c);
         if(cw==0x3f800000u)return v;
         const uint32_t cm=0x00800000u|(cw&0x007fffffu);
         const uint64_t p=(uint64_t)uv*cm;
         unsigned q=(unsigned)(p>>24),r=(unsigned)p&0x00ffffffu;
         q+=(r>0x00800000u || (r==0x00800000u && (q&1u)));
         const unsigned result=(raw&0x80000000u)|q;
         float out;
         __builtin_memcpy(&out,&result,4);return out;
     }
     return v*c;
 }
#endif
#endif
 return (float)((double)v*cos((double)x));
}

static void computephaser(phaser* dsp, int count, FAUSTFLOAT** inputs, FAUSTFLOAT** outputs) {
	FAUSTFLOAT* input0 = inputs[0];
	FAUSTFLOAT* input1 = inputs[1];
	FAUSTFLOAT* output0 = outputs[0];
	FAUSTFLOAT* output1 = outputs[1];
#if DNBOX_PH_COEF
    ph_slow(dsp);
    float fSlow0=dsp->gain;
#else
    float fSlow0 = pow(10.f, (0.05f * (float)dsp->fHslider0));
#endif
	float fSlow1 = (0.5f * ((int)(float)dsp->fCheckbox0?2.f:(float)dsp->fHslider1));
	float fSlow2 = (1.f - fSlow1);
#if DNBOX_PH_COEF
    float fSlow3=dsp->pole;
#else
    float fSlow3 = exp((dsp->fConst1 * (0.f - (3.14159f * (float)dsp->fHslider2))));
#endif
	float fSlow4 = faustpower2_f(fSlow3);
	float fSlow5 = (0.f - (2.f * fSlow3));
	float fSlow6 = (float)dsp->fHslider3;
	float fSlow7 = (dsp->fConst1 * fSlow6);
	float fSlow8 = (float)dsp->fHslider4;
	float fSlow9 = (6.28319f * fSlow8);
	float fSlow10 = (0.5f * ((6.28319f * max(fSlow8, (float)dsp->fHslider5)) - fSlow9));
	float fSlow11 = (dsp->fConst2 * (float)dsp->fHslider6);
#if DNBOX_PH_COEF
    float fSlow12=dsp->rsin;
#else
    float fSlow12 = sin(fSlow11);
#endif
#if DNBOX_PH_COEF
    float fSlow13=dsp->rcos;
#else
    float fSlow13 = cos(fSlow11);
#endif
	float fSlow14 = (0.f - fSlow12);
	float fSlow15 = (float)dsp->fHslider7;
	float fSlow16 = (dsp->fConst1 * faustpower2_f(fSlow6));
	float fSlow17 = (dsp->fConst1 * faustpower3_f(fSlow6));
	float fSlow18 = (dsp->fConst1 * faustpower4_f(fSlow6));
	float fSlow19 = ((int)(float)dsp->fCheckbox1?(0.f - fSlow1):fSlow1);
	/* C99 loop */
	{
		int i;
		for (i = 0; (i < count); i = (i + 1)) {
			dsp->iVec0[0] = 1;
			float fTemp0 = (float)input0[i];
			dsp->fRec5[0] = ((fSlow12 * dsp->fRec6[1]) + (fSlow13 * dsp->fRec5[1]));
			dsp->fRec6[0] = ((1.f + ((fSlow13 * dsp->fRec6[1]) + (fSlow14 * dsp->fRec5[1]))) - (float)dsp->iVec0[1]);
			float fTemp1 = ((fSlow10 * (1.f - dsp->fRec5[0])) + fSlow9);
			float fTemp2 = ph_cosprod(dsp->fRec4[1], fSlow7 * fTemp1, 0);
			dsp->fRec4[0] = (0.f - (((fSlow5 * fTemp2) + (fSlow4 * dsp->fRec4[2])) - ((fSlow0 * fTemp0) + (fSlow15 * dsp->fRec0[1]))));
			float fTemp3 = ph_cosprod(dsp->fRec3[1], fSlow16 * fTemp1, 1);
			dsp->fRec3[0] = ((fSlow5 * (fTemp2 - fTemp3)) + (dsp->fRec4[2] + (fSlow4 * (dsp->fRec4[0] - dsp->fRec3[2]))));
			float fTemp4 = ph_cosprod(dsp->fRec2[1], fSlow17 * fTemp1, 2);
			dsp->fRec2[0] = ((fSlow5 * (fTemp3 - fTemp4)) + (dsp->fRec3[2] + (fSlow4 * (dsp->fRec3[0] - dsp->fRec2[2]))));
			float fTemp5 = ph_cosprod(dsp->fRec1[1], fSlow18 * fTemp1, 3);
			dsp->fRec1[0] = ((fSlow5 * (fTemp4 - fTemp5)) + (dsp->fRec2[2] + (fSlow4 * (dsp->fRec2[0] - dsp->fRec1[2]))));
			dsp->fRec0[0] = ((fSlow4 * dsp->fRec1[0]) + ((fSlow5 * fTemp5) + dsp->fRec1[2]));
			output0[i] = (FAUSTFLOAT)((fSlow0 * (fSlow2 * fTemp0)) + (dsp->fRec0[0] * fSlow19));
			float fTemp6 = (float)input1[i];
			float fTemp7 = ((fSlow10 * (1.f - dsp->fRec6[0])) + fSlow9);
			float fTemp8 = ph_cosprod(dsp->fRec11[1], fSlow7 * fTemp7, 4);
			dsp->fRec11[0] = (0.f - (((fSlow5 * fTemp8) + (fSlow4 * dsp->fRec11[2])) - ((fSlow0 * fTemp6) + (fSlow15 * dsp->fRec7[1]))));
			float fTemp9 = ph_cosprod(dsp->fRec10[1], fSlow16 * fTemp7, 5);
			dsp->fRec10[0] = ((fSlow5 * (fTemp8 - fTemp9)) + (dsp->fRec11[2] + (fSlow4 * (dsp->fRec11[0] - dsp->fRec10[2]))));
			float fTemp10 = ph_cosprod(dsp->fRec9[1], fSlow17 * fTemp7, 6);
			dsp->fRec9[0] = ((fSlow5 * (fTemp9 - fTemp10)) + (dsp->fRec10[2] + (fSlow4 * (dsp->fRec10[0] - dsp->fRec9[2]))));
			float fTemp11 = ph_cosprod(dsp->fRec8[1], fSlow18 * fTemp7, 7);
			dsp->fRec8[0] = ((fSlow5 * (fTemp10 - fTemp11)) + (dsp->fRec9[2] + (fSlow4 * (dsp->fRec9[0] - dsp->fRec8[2]))));
			dsp->fRec7[0] = ((fSlow4 * dsp->fRec8[0]) + ((fSlow5 * fTemp11) + dsp->fRec8[2]));
			output1[i] = (FAUSTFLOAT)((fSlow0 * (fSlow2 * fTemp6)) + (dsp->fRec7[0] * fSlow19));
			dsp->iVec0[1] = dsp->iVec0[0];
			dsp->fRec5[1] = dsp->fRec5[0];
			dsp->fRec6[1] = dsp->fRec6[0];
			dsp->fRec4[2] = dsp->fRec4[1];
			dsp->fRec4[1] = dsp->fRec4[0];
			dsp->fRec3[2] = dsp->fRec3[1];
			dsp->fRec3[1] = dsp->fRec3[0];
			dsp->fRec2[2] = dsp->fRec2[1];
			dsp->fRec2[1] = dsp->fRec2[0];
			dsp->fRec1[2] = dsp->fRec1[1];
			dsp->fRec1[1] = dsp->fRec1[0];
			dsp->fRec0[1] = dsp->fRec0[0];
			dsp->fRec11[2] = dsp->fRec11[1];
			dsp->fRec11[1] = dsp->fRec11[0];
			dsp->fRec10[2] = dsp->fRec10[1];
			dsp->fRec10[1] = dsp->fRec10[0];
			dsp->fRec9[2] = dsp->fRec9[1];
			dsp->fRec9[1] = dsp->fRec9[0];
			dsp->fRec8[2] = dsp->fRec8[1];
			dsp->fRec8[1] = dsp->fRec8[0];
			dsp->fRec7[1] = dsp->fRec7[0];

		}

	}

}


void s1u_phaser_run(s1u_phaser *p,float *l,float *r,float pm,float rate,float width,float fb,float sr)
{
 if(pm==0.f)return;
 if(!p->valid){
     initphaser(p,(int)sr);
     p->fCheckbox0=1.f;
 }
 p->fHslider2=width;p->fHslider6=rate;p->fHslider7=fb;
 float pl=*l,pr=*r,ol=0.f,orr=0.f;
 float *in[]={&pl,&pr},*out[]={&ol,&orr};
 computephaser(p,1,in,out);
 float dry=1.f-pm;
 *l=dry*pl+(1.f-dry)*ol;*r=dry*pr+(1.f-dry)*orr;
}
