#include <stdlib.h>
#include "soundpipe.h"
int sp_auxdata_alloc(sp_auxdata *aux,size_t n){aux->size=n;aux->ptr=calloc(1,n);return SP_OK;}
int sp_auxdata_free(sp_auxdata *aux){free(aux->ptr);aux->ptr=0;return SP_OK;}
unsigned run(const float *cu,float *o,unsigned n){
 sp_revsc *q=0;sp_data sp={0};sp.sr=44100;
 sp_revsc_create(&q);sp_revsc_init(&sp,q);q->lpfreq=22050.f;
 for(unsigned i=0;i<n;i++){
  float l=cu[3*i],r=cu[3*i+1];q->feedback=cu[3*i+2];
  sp_revsc_compute(&sp,q,&l,&r,o+2*i,o+2*i+1);
 }
 unsigned bytes=q->aux.size;sp_revsc_destroy(&q);return bytes;
}
