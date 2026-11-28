#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "app/s1c_part.h"
extern void candidate_s1c_render(s1c *,float *,float *,unsigned);
extern unsigned candidate_lab_osc4_hits;
static s1_dry_part part, before, expected;
_Alignas(16) static uint8_t arena[S1C_ARENA_BYTES], start[S1C_ARENA_BYTES], end[S1C_ARENA_BYTES];
static float actual[2][256], original[2][256];
#define set(id,v) s1c_set(&part.e,(id),(v))
int main(void)
{
    unsigned total=0, spans=0, tails=0, mono_moves=0;
    const unsigned keys[6]={48,51,55,60,64,66};
    const unsigned sizes[]={1,3,4,7,8,9,15,16,17,31,32,64,127,128,129,255,256};
    for(unsigned test=0;test<36;test++){
        memset(arena,0,sizeof arena);
        s1_part_init(&part);
        s1_part_bind(&part,arena);
        set(20,test&1);
        if(test>=32){set(0,.9998104f);set(1,.94f);set(146,0.f);set(50,0.f);part.e.port_c2=0.f;}
        set(115,test%3); /* FTYPE */
        set(12,(test&2)?.43f:0.f);
        set(10,(test&4)?.31f:0.f);
        set(21,.024f);
        set(37,(test&8)!=0); set(38,.71f); set(40,.36f);
        set(41,(test&16)!=0); set(42,.52f); set(43,.0137f); set(44,.41f);
        set(35,.48f);set(36,1.9f); set(121,.63f);
        set(116,.59f);set(117,.81f);set(118,-.2f);set(119,.77f);
        set(58,test%4); /* LBC */
        set(59,test%4); /* LTREM */
        set(14,.32f);set(15,2.1f);set(45,1);set(46,.39f);set(47,.67f);
        for(unsigned j=0;j<6;j++)s1_part_note(&part,keys[j],70+5*j);
        for(unsigned i=0;i<170;i++) {
            unsigned n=sizes[(i+test*11u)% (sizeof sizes/sizeof sizes[0])];
            if(test<32&&i%11==0){set(0,(i%12)*(.0833333f));set(16,220.f+77.f*i);set(34,2048.f+224.f*i);}
            if(i%19==4){ s1_part_note(&part,keys[i%6],0); s1_part_note(&part,keys[i%6]+1,100); }
            if(i==37 || i==117) {set(20,(i==37)? !(test&1):(test&1));++mono_moves;}
            if(i==81){for(unsigned j=0;j<6;j++)s1_part_note(&part,keys[j]+1,0);}
            if(i==101){set(43,.0299f);set(12,(test&2)?0.f:.34f);set(2,.37f);}
            if(i==111)s1_part_all_off(&part);
            if(i==114) for(unsigned j=0;j<6;j++)s1_part_note(&part,keys[j],100);
            memcpy(&before,&part,sizeof part);memcpy(start,arena,sizeof arena);
            s1c_render(&part.e,original[0],original[1],n);
            memcpy(&expected,&part,sizeof part);memcpy(end,arena,sizeof arena);
            memcpy(&part,&before,sizeof part);memcpy(arena,start,sizeof arena);
            candidate_s1c_render(&part.e,actual[0],actual[1],n);
            if(memcmp(original[0],actual[0],4*n)||memcmp(original[1],actual[1],4*n)||
               memcmp(end,arena,sizeof arena)||memcmp(&expected,&part,sizeof part)){
                fprintf(stderr,"mismatch test=%u span=%u n=%u L=%d R=%d part=%d arena=%d\n",test,i,n,memcmp(original[0],actual[0],4*n),memcmp(original[1],actual[1],4*n),memcmp(&expected,&part,sizeof part),memcmp(end,arena,sizeof arena));
                return 1;
            }
            total+=n;++spans;tails+=(n%8!=0);
        }
    }
    if(!candidate_lab_osc4_hits)return 4;
    printf("admitted exact 4-frame oscillator tiles: %u\n", candidate_lab_osc4_hits);
    printf("PASS %u signed stereo frames, %u spans (%u short tails), %u mono transitions, exact whole part and arena each span.\n",total,spans,tails,mono_moves);
}
