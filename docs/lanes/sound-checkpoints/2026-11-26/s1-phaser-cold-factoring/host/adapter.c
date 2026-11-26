#include <string.h>
#define DNBOX_S1_MATCHED_PHASER_F32 1
#include "/tmp/dnbox-phaser-fit.quJTsGT6/tree/firmware/src/dsp/fastmath.h"
typedef struct {
 unsigned tags[3], valid;
 float gain, pole, rsin, rcos;

    union {
      struct {
	float fRec4[3];
	float fRec3[3];
	float fRec2[3];
	float fRec1[3];
	float fRec11[3];
	float fRec10[3];
	float fRec9[3];
	float fRec8[3];
      };
      float ph_lane[2][4][3];
    };
	int iVec0[2];
	float fRec5[2];
	float fRec6[2];
	float fRec0[2];
	float fRec7[2];
	float fHslider0;
	float fCheckbox0;
	float fHslider1;
	int fSamplingFreq;
	int iConst0;
	float fConst1;
	float fHslider2;
	float fHslider3;
	float fHslider4;
	float fHslider5;
	float fConst2;
	float fHslider6;
	float fHslider7;
	float fCheckbox1;

} native_ph;
#define s1u_phaser native_ph
#define s1u_phaser_run ph_native_internal
#include "/tmp/dnbox-phaser-fit.quJTsGT6/pre-cold.inc"
void ph_native_run(void*p,float*l,float*r,float m,float f,float w,float fb,float sr){ph_native_internal(p,l,r,m,f,w,fb,sr);}
