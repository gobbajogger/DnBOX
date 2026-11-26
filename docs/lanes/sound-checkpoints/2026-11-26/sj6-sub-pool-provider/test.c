#include "provider.h"
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
struct virtual_pool {unsigned n,dim,missing_block; int16_t *source;unsigned reads;};
static int read_pair(void *p,unsigned ch,unsigned i,float *a,float *b){
    struct virtual_pool *v=p;
    if ((i/2)==v->missing_block || ((i+1)/2)==v->missing_block) return 0;
    sub_pcm16 view={v->source,v->n,v->dim};
    v->reads++;
    return sub_pcm16_pair(&view,ch,i,a,b);
}
int main(void) {
 uint32_t x=1; int16_t data[514];
 for(unsigned j=0;j<514;j++) {x=x*1664525u+1013904223u;data[j]=(int16_t)(x>>16);}
 for(unsigned n=17;n<=257;n+=240) for(unsigned dim=1;dim<=2;dim++){
    struct virtual_pool pool={n,dim,UINT32_MAX,data,0};
    sub_media m={read_pair,&pool,n,dim,1,2.5f,(float)(n-1),4.5f,8.5f};
    sub_owner owner={0};float y[2]={3,4};
    assert(!sub_commit(&owner,NULL)&&sub_frame(&owner,1,1,1,y)==-1&&!owner.active&&y[0]==3&&y[1]==4);
    assert(sub_commit(&owner,&m) && owner.cursor==2.5);
    pool.missing_block=1;
    assert(sub_frame(&owner,1,1,1,y)==-1&&owner.cursor==2.5&&y[0]==3&&y[1]==4);
    pool.missing_block=UINT32_MAX;
    assert(sub_frame(&owner,.5,1,1,y)==1&&owner.cursor==3);
    assert((y[0]==y[1])==(dim==1));
    while(owner.cursor<8.5) assert(sub_frame(&owner,.5,1,1,y)==1);
    assert(sub_frame(&owner,.5,1,1,y)==1&&owner.cursor==5);
    assert(sub_gate(&owner,0));while(owner.cursor<=n-1)assert(sub_frame(&owner,.5,1,1,y)==1);
    assert(sub_frame(&owner,.5,1,1,y)==0&&!owner.active);
    assert(sub_commit(&owner,&m) && owner.cursor==2.5 && pool.reads>0);
 }
 puts("4 virtual-pool dimensions: exact signed center/read, cross-page failure atomic, end, release and loop");
}
