/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "miami_program_private.h"
#include "chicago_program_private.h"
#define ORIGINAL_PROGRAM_VALUES_MIAMI 1
#define ORIGINAL_PROGRAM_VALUES_CHICAGO 1
#include "program_values_compact.h"
static uint32_t bits(float f){uint32_t u;memcpy(&u,&f,4);return u;}
int main(void){unsigned n=0;
 for(unsigned f=0;f<30;f++){
  for(unsigned p=0;p<18;p++){if(bits(compact_miami_factory_value(f,p))!=bits(original_miami_factory_macro[f][p]))return fprintf(stderr,"factory %u/%u mismatch\n",f,p),1;n++;}
  const char *s=compact_miami_factory_name(f);if(strcmp(s,original_miami_factory_name[f]))return fprintf(stderr,"name %u mismatch\n",f),1;
  if(s!=compact_miami_factory_name(f))return 1;
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
