#include "soundpipe.h"
#include <stdlib.h>
typedef struct{sp_data*s;sp_revsc*r;}Ref;
void *ref_new(void){Ref*a=calloc(1,sizeof* a);sp_create(&a->s);a->s->sr=44100;sp_revsc_create(&a->r);sp_revsc_init(a->s,a->r);a->r->lpfreq=22050;return a;}
void ref_drop(void*v){Ref*a=v;sp_revsc_destroy(&a->r);sp_destroy(&a->s);free(a);}
void ref_run(void*v,const float*x,float*y,const float*fb,unsigned n){Ref*a=v;for(unsigned i=0;i<n;i++){float l=*x++,r=*x++;a->r->feedback=*fb++;sp_revsc_compute(a->s,a->r,&l,&r,y,y+1);y+=2;}}
