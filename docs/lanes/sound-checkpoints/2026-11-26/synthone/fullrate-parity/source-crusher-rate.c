/* SPDX-License-Identifier: GPL-3.0-only */
#include <math.h>
/* Match the original C++ Float32 log2 and exp2 overloads. The compact engine's
 * translation unit maps log2f/exp2f to different fast-math helpers. */
float source_crusher_rate(float base,float modulation){
    float x=log2f(base);
    x+=4.f*modulation;
    x=exp2f(x);
    return x<2048.f?2048.f:x>48000.f?48000.f:x;
}
