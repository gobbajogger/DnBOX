from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
s=s[:s.index('__attribute__((visibility("default")))')]
s=s.replace('clock_2x(OTA *s,float x','clock_2x(float *cond,float pole[4],float x')
s=s.replace('(x-s->condition)','(x-*cond)').replace('s->condition+dv','*cond+dv').replace('s->condition=dv+lp','*cond=dv+lp')
s=s.replace('s->pole[i]','pole[i]')
s=s.replace('static inline float physical_tick(OTA *s, float x','static inline float routed_tick(Q24 up[2][2],Q24 down[2][2],float *cond,float pole[4],float x')
s=s.replace('s->up[','up[').replace('s->down[','down[')
s=s.replace('clock_2x(s,a,','clock_2x(cond,pole,a,').replace('clock_2x(s,b,','clock_2x(cond,pole,b,')
s+='''/* Named individual gap components: never access across their sizes. */
typedef struct {
    struct { Q24 up[2][2],down[2][2]; float cond; } voice[6];
    float first[3][4];
    float last_0;
} GapA;
typedef struct {float next[2][4];} GapB;
typedef struct {float last_123[3];} GapC;
_Static_assert(sizeof(GapA)==220,"real proposed gap A");
_Static_assert(sizeof(GapB)==32,"real proposed gap B");
_Static_assert(sizeof(GapC)==12,"real proposed gap C");
typedef struct {
    GapA a; uint32_t fence_a;
    GapB b; uint32_t fence_b;
    GapC c; uint32_t fence_c;
} TestLayout;
static TestLayout split[2];
static OTA old[2][6];

__attribute__((visibility("default"))) void reset(unsigned p,unsigned v) {
    TestLayout *t=&split[p];
    memset(&t->a.voice[v],0,sizeof t->a.voice[v]);
    memset(&old[p][v],0,sizeof old[p][v]);
    if(v<3)memset(t->a.first[v],0,sizeof t->a.first[v]);
    else if(v<5)memset(t->b.next[v-3],0,sizeof t->b.next[v-3]);
    else{
        t->a.last_0=0.f;
        memset(t->c.last_123,0,sizeof t->c.last_123);
    }
    t->fence_a=0x34a85190;
    t->fence_b=0x92198c42;
    t->fence_c=0x04bd37a2;
}
__attribute__((visibility("default"))) float step_contig(unsigned p,unsigned v,unsigned mask,float x,const float *g,const float *coeff) {
    if(p>=2||v>=6||(mask&(1u<<v))==0u)return -0.f;
    OTA *t=&old[p][v];
    return routed_tick(t->up,t->down,&t->condition,t->pole,x,g,coeff);
}
__attribute__((visibility("default"))) float step_split(unsigned p,unsigned v,unsigned mask,float x,const float *g,const float *coeff) {
    if(p>=2||v>=6||(mask&(1u<<v))==0u)return -0.f;
    TestLayout *t=&split[p];
    float local[4];
    float *pole=local;
    if(v<3) pole=t->a.first[v];
    else if(v<5) pole=t->b.next[v-3];
    else{
        local[0]=t->a.last_0;
        memcpy(local+1,t->c.last_123,sizeof t->c.last_123);
    }
    float y=routed_tick(t->a.voice[v].up,t->a.voice[v].down,&t->a.voice[v].cond,pole,x,g,coeff);
    if(v==5){
        t->a.last_0=local[0];
        memcpy(t->c.last_123,local+1,sizeof t->c.last_123);
    }
    return y;
}
__attribute__((visibility("default"))) unsigned state_equal(unsigned p) {
    const TestLayout *t=&split[p];
    if(t->fence_a!=0x34a85190||t->fence_b!=0x92198c42||t->fence_c!=0x04bd37a2)return 0;
    for(unsigned v=0;v<6;v++){
        const OTA *u=&old[p][v];
        if(memcmp(u->up,t->a.voice[v].up,sizeof u->up)
         ||memcmp(u->down,t->a.voice[v].down,sizeof u->down)
         ||memcmp(&u->condition,&t->a.voice[v].cond,sizeof u->condition))return 0;
        if(v==5){
            if(memcmp(u->pole,&t->a.last_0,4)||memcmp(u->pole+1,t->c.last_123,12))return 0;
        }else if(memcmp(u->pole,v<3?t->a.first[v]:t->b.next[v-3],16))return 0;
    }
    return 1;
}
'''
Path(sys.argv[2]).write_text(s)
