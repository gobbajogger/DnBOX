#pragma once
#include "numeric_boundary.h"
namespace ota_f32_math {
inline float fma(float a,float b,float c){return fma32_fused(a,b,c);}
inline float ingress(double value){return f64_to_f32_rne(value);}
}
