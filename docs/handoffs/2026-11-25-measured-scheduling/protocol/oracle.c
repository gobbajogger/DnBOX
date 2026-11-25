#include "protocol.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#define TAG(n,p) (((n)<<2)|(p))
static uint32_t rng=42;
static unsigned next(void){return (rng=rng*1664525u+1013904223u)>>8;}
static uint32_t direct(uint32_t old,unsigned gen,unsigned part,unsigned k,unsigned v){
    return gen ? gen<<25 | ((old+(1u<<17))&(255u<<17)) | v<<10 | k<<3 | part : old;
}
int main(void)
{
    for(unsigned e=1;e<=8000;e++){
        struct job j[JOBS]={{0}};
        const unsigned gen=e%128u, cur=e%17u==0?(gen+1u)%128u:gen;
        uint32_t a=next()<<8, b=a, packed[JOBS]={0};
        unsigned order[JOBS]={0,1,2,3,4}, renders[JOBS]={0};
        for(unsigned p=0;p<JOBS;p++){
            unsigned n=(e+p)%7==0?256u:next()%513u;
            if(e%23u==0)n=0;
            for(unsigned i=0;i<n;i++){
                unsigned k=next()%128u,v=next()%128u;
                packed[p]=dnbx_demo_event(packed[p],p,k,v);
                if(gen==cur) b=direct(b,gen,p,k,v);
            }
            j[p].tag=TAG(e,QUEUED);
            unsigned i=next()%(p+1u), tmp=order[i];order[i]=order[p];order[p]=tmp;
        }
        for(unsigned i=0;i<JOBS;i++){
            unsigned p=order[i];
            assert(dnbx_demo_classify(&j[p],e,0)==DEADLINE_UNCERTAIN);
            assert(dnbx_demo_start(&j[p],e));renders[p]++;
            assert(!dnbx_demo_start(&j[p],e));
            assert(dnbx_demo_classify(&j[p],e,1)==DEADLINE_UNCERTAIN);
            assert(dnbx_demo_finish(&j[p],e,packed[p]));
            assert(dnbx_demo_classify(&j[p],e,1)==USE_RESULT && renders[p]==1);
            assert(dnbx_demo_classify(&j[p],e+1u,1)==STALE_SLOT);
        }
        uint32_t committed=0;
        assert(dnbx_demo_join_voice(&committed,a,gen,cur,j,e,31u)==b);
        assert(committed==e);
        assert(dnbx_demo_join_voice(&committed,b,gen,cur,j,e,31u)==b);
        committed=0;
        j[JOB_A].tag=TAG(e,STARTED);
        assert(dnbx_demo_join_voice(&committed,a,gen,cur,j,e,31u)==a && !committed);
        j[JOB_A].tag=TAG(e+1u,DONE);
        assert(dnbx_demo_join_voice(&committed,a,gen,cur,j,e,31u)==a && !committed);
        struct job queued={.tag=TAG(e,QUEUED)};
        assert(dnbx_demo_classify(&queued,e,0)==DEADLINE_UNCERTAIN);
        assert(dnbx_demo_classify(&queued,e,1)==LOCAL_FALLBACK);
        assert(dnbx_demo_start(&queued,e));
        assert(dnbx_demo_classify(&queued,e,1)==DEADLINE_UNCERTAIN);
    }
    puts("8000 ordered five-job stretches; 40000 worker starts; timeout/quarantine, stale tags, generations and stamp wrap: PASS");
}
