/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdio.h>
#include <string.h>
#include "break_retrigger_reference.inc"
#include "../../firmware/src/dsp/break_retrigger_lut.h"
int main(void){unsigned checks=0;for(unsigned l=0;l<6;l++)for(unsigned p=0;p<256;p++){
float a=reference_probability(l,p),b=brk_retrigger_probability(l,p);
if(memcmp(&a,&b,4)){fprintf(stderr,"PROBABILITY_BITS_MISMATCH %u %u\n",l,p);return 1;}checks++;}
printf("PASS %u immutable original-table binary32 comparisons\n",checks);return 0;}
