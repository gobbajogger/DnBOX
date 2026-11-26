#include "halfband.h"
float measured(compact_halfband *s,const float x[4]){return compact_decimate(s,x);}
unsigned footprint(void){return sizeof(compact_halfband);}
