#ifndef DNBOX_HOST_TUNING_LOAD_H
#define DNBOX_HOST_TUNING_LOAD_H
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
static inline int load_s1_authored_tuning(float *words){
 const char *path=getenv("S1_AUTHORED_TUNING_F32");
 if(!path||!*path)return 0;
 FILE*f=fopen(path,"rb");
 if(!f || fread(words,sizeof(float),127,f)!=127 || fgetc(f)!=EOF)abort();
 fclose(f);
 for(unsigned i=0;i<127;i++)if(!(isfinite(words[i])&&words[i]>0.f&&words[i]<=22050.f))abort();
 return 1;
}
#endif
