#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "commit.c"
#include "cases.h"
static unsigned rounded(float start,int full_float)
{
    double y=start, d=-(double)start/27.;
    if(full_float)d=(float)d;
    unsigned n=0;
    do{y+=d;if(full_float)y=(float)y;n++;}while(y>0.);
    return n;
}
int main(void)
{
    for(unsigned i=0;i<sizeof cases/sizeof *cases;i++){
        sj6_stage1 s={0};assert(sj6_stage1_prep(&s)==-1);
        assert(sj6_stage1_offer(&s,cases[i].anchor,27,55));
        assert(s.step==cases[i].step);
        const sj6_stage1 unmodified=s;
        assert(!sj6_stage1_offer(&s,0.,27,48)&&!memcmp(&s,&unmodified,sizeof s));
        assert(!sj6_stage1_offer(&s,NAN,27,48)&&!memcmp(&s,&unmodified,sizeof s));
        for(unsigned f=cases[i].begin;f<cases[i].end;f+=16)
            assert(sj6_stage1_prep(&s)==-1);
        assert(sj6_stage1_prep(&s)==55&&sj6_stage1_prep(&s)==-1);
        const unsigned n=1+(cases[i].end-cases[i].begin)/16;
        printf("%s exact %u; rounded input %u; float path %u; frame %u\n",
            cases[i].name,n,rounded((float)cases[i].anchor,0),rounded((float)cases[i].anchor,1),cases[i].end);
        assert(sj6_stage1_offer(&s,cases[i].anchor,27,55));
        sj6_stage1_cancel(&s);
        assert(sj6_stage1_prep(&s)==-1);
        assert(sj6_stage1_offer(&s,cases[i].anchor,27,23));
        for(unsigned k=1;k<n;k++)assert(sj6_stage1_prep(&s)==-1);
        assert(sj6_stage1_prep(&s)==23);
    }
}
