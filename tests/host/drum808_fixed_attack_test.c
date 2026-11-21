/* The fixed MARACAS owner has a nominal 99%-rise duration, not a CLAP attack. */
#include <math.h>
#include <stdio.h>
#include "drum808.h"
int main(void) {
 for(int q=0;q<128;q++) {
  float got=-1;int unit=drum808_param_si(D8_CP,3,q,&got);
  double a=(double)q/32.0, want=fmax(.2,1.2*a*a)*log(100.0);
  if(unit!=X0X_U_MS || fabs((double)got-want)>.0001) {
   fprintf(stderr,"FIXED_MA_RISE_MISMATCH raw=%d unit=%d ms=%.9g expected=%.9g\n",q,unit,(double)got,want);return 1;
  }
 }
 puts("PASS 128 nominal fixed-MARACAS 99-percent rise durations");return 0;
}
