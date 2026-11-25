/* Headless stimulus only. Writes raw stereo f32le (little-endian hosts). */
#include "app/s1c_part.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
static s1_dry_part part;
#if X0X_S1_DELAY_TIME_NATIVE
extern int s1c_delay_time_fit(s1c *,float,int);
#endif
static _Alignas(8) unsigned char arena[S1C_ARENA_BYTES];
static const unsigned notes[]={48,55,60,64,67,72};
static void render(unsigned count,FILE *out){
    float l[128],r[128],st[256];
    while(count){
        unsigned n=count<128?count:128;
        unsigned i;
        s1_part_render(&part,l,r,n);
        for(i=0;i<n;i++){
            if(!isfinite(l[i])||!isfinite(r[i]))exit(4);
            st[2*i]=l[i];st[2*i+1]=r[i];
        }
        if(out && fwrite(st,sizeof(float)*2,n,out)!=n)exit(5);
        count-=n;
    }
}
#if S1C_EXT
static void ring(const char *phase)
{
    uint32_t h=2166136261u;
    for(int i=0;i<part.e.pool_bytes;i++)h=(h^part.e.pool[i])*16777619u;
    printf("warm-probe %s dec %d phase %d ring %08x\n",phase,part.e.ddec,part.e.dk,h);
}
#endif
int main(int argc,char **argv){
    if(argc!=5)return 2;
    FILE *in=fopen(argv[1],"rb");
    if(!in)return 2;
    if(fseek(in,0,SEEK_END))return 2;
    long bytes=ftell(in);
    if(bytes<32||bytes>1000000||fseek(in,0,SEEK_SET))return 2;
    unsigned char *pack=malloc((size_t)bytes);
    if(!pack||fread(pack,1,(size_t)bytes,in)!=(size_t)bytes)return 2;
    fclose(in);
    if(!s1_part_set_pack(pack,(uint32_t)bytes))return 3;
    unsigned index=(unsigned)strtoul(argv[2],0,10);
    s1_part_init(&part);
    s1_part_bind(&part,arena);
    if(!s1_part_program(&part,index))return 3;
    if(!strcmp(argv[4],"dry")){
        s1c_set(&part.e,S1C_P_RVON,0.0f);
        s1c_set(&part.e,S1C_P_DLON,0.0f);
        s1c_set(&part.e,S1C_P_BCD,0.0f);
        s1c_set(&part.e,S1C_P_PANA,0.0f);
        s1c_set(&part.e,S1C_P_PHMIX,0.0f);
        s1c_set(&part.e,S1C_P_WIDEN,0.0f);
        s1c_set(&part.e,S1C_P_RVMIX,0.0f);
        s1c_set(&part.e,S1C_P_DLMIX,0.0f);
    }else if(strcmp(argv[4],"wet"))return 2;
    FILE *out=fopen(argv[3],"wb");
    if(!out)return 2;
    printf("%u: %s arena %u\n",index,s1_part_program_name(index),(unsigned)sizeof arena);
    render(128,NULL);
    for(unsigned i=0;i<sizeof notes/sizeof *notes;i++){
        s1_part_note(&part,notes[i],64+11*i);
        render(33075,out);
        s1_part_note(&part,notes[i],0);
    }
    render(33075,out);
    render(33075,out);
#if S1C_EXT && X0X_S1_DELAY_TIME_NATIVE
    if(getenv("WARM_PROBE")) {
        s1_part_note(&part,72,103);
        render(16000,NULL);
        ring("active");
        s1c_set(&part.e,S1C_P_DLON,0);
        s1c_set(&part.e,S1C_P_DLMIX,0);
        render(513,NULL);
        ring("bypass");
        if(!s1c_delay_time_fit(&part.e,2.4f,0))return 6;
        s1c_set(&part.e,S1C_P_DLT,2.4f);
        render(8192,NULL);
        ring("grown");
        s1c_set(&part.e,S1C_P_DLT,0.425f);
        s1c_set(&part.e,S1C_P_DLON,1);
        s1c_set(&part.e,S1C_P_DLMIX,0.4f);
        render(12001,NULL);
        ring("revealed");
    }
#endif
    if(fclose(out))return 2;
#if S1C_EXT
    printf("%u %s delay_on %.3g reverb_on %.3g delay_decimation %d pool_bytes %d\n",
           index,argv[4],part.e.t[S1C_P_DLON],part.e.t[S1C_P_RVON],
           part.e.ddec,part.e.pool_bytes);
#endif
    free(pack);
    return 0;
}
