#include <math.h>
#include "candidate.h"
#include "fixed_candidate.h"
float coeff_fixed(float seconds,float rate,int attack){return s1_retention_fixed(seconds,rate,attack);}
float coeff_fast(float seconds,float rate,int attack){return s1_retention_fast(seconds,rate,attack);}
float coeff_exp2(float seconds,float rate,int attack){
 if(seconds==0.f)return 0.f;
 float x=-1.f/((rate*seconds)*(attack?.6f:1.f));
 return fm_exp2f(x*FM_LOG2E);
}
float coeff_double_x(float seconds,float rate,int attack){
 if(seconds==0.f)return 0.f;
 double x=-1./(((double)rate*(double)seconds)*(attack?.6:1.));
 return fm_expf((float)x);
}
float coeff_oracle(float seconds,float rate,int attack){
 if(seconds==0.f)return 0.f;
 return (float)exp(-1./(((double)rate*(double)seconds)*(attack?.6:1.)));
}
