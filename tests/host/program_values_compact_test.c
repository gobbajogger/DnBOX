/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "miami_program_private.h"
#include "chicago_program_private.h"
#include "chicago_drive_private.h"
#include "modulation_private.h"
#include "../wobble/miami_decimator_tables_private.h"
#include "linear_control.h"
#define ORIGINAL_PROGRAM_VALUES_MIAMI 1
#define ORIGINAL_PROGRAM_VALUES_CHICAGO 1
#include "program_values_compact.h"
static uint32_t bits(float f){uint32_t u;memcpy(&u,&f,4);return u;}
int main(void){unsigned n=0;
 for(unsigned value=0;value<128;value++){
  if(original_linear_control(value)!=mod_linear[value] ||
     original_linear_control(value)!=original_drive_table[value] ||
     original_linear_control(value)!=miami_decimator_wet[value] ||
     original_drive_gain(value)!=original_drive_gain_table[value] ||
     original_chorus_wet(value)!=mod_wet[value] ||
     original_chorus_feedback(value)!=mod_feedback[value] ||
     original_decimator_frequency(value)!=miami_decimator_frequency[value])
   return fprintf(stderr,"FX control %u mismatch\n",value),1;
  n+=7;
 }
 for(unsigned f=0;f<30;f++){
  for(unsigned p=0;p<18;p++){if(bits(compact_miami_factory_value(f,p))!=bits(original_miami_factory_macro[f][p]))return fprintf(stderr,"factory %u/%u mismatch\n",f,p),1;n++;}
  const char *s=compact_miami_factory_name(f);if(strcmp(s,original_miami_factory_name[f]))return fprintf(stderr,"name %u mismatch\n",f),1;
  if(s!=compact_miami_factory_name(f))return 1;
  n++;
 }
 for(unsigned f=0;f<81;f++)for(unsigned p=0;p<22;p++){
  unsigned bit=(f*22+p)*9,byte=bit>>3;
  unsigned code=((original_chicago_factory_codes[byte]|((unsigned)original_chicago_factory_codes[byte+1]<<8))>>(bit&7))&511;
  if(bits(compact_chicago_factory_value(f,p))!=bits(original_chicago_factory_dictionary[code]))
   return fprintf(stderr,"chicago factory %u/%u mismatch\n",f,p),1;
  n++;
 }
 for(unsigned i=0;i<109;i++){if(bits(compact_miami_base_value(i))!=bits(original_miami_base[i]))return fprintf(stderr,"miami base %u mismatch\n",i),1;n++;}
 for(unsigned i=0;i<117;i++){if(bits(compact_chicago_base_value(i))!=bits(original_chicago_base[i]))return fprintf(stderr,"chicago base %u mismatch\n",i),1;n++;}
 for(unsigned id=0;id<117;id++){
  unsigned code=compact_chicago_native_codes[id],words=sizeof original_program_value_words/sizeof original_program_value_words[0],slot=0;
  while(slot<29 && original_chicago_mappings[slot].native_id!=id)slot++;
  if(slot<29 ? code!=words+slot : code>=words || bits(compact_program_value(code))!=bits(original_chicago_base[id]))
   return fprintf(stderr,"chicago native %u mismatch\n",id),1;
  n++;
 }
 printf("PASS compact program metadata: %u exact binary32/name checks\n",n);return 0;
}
