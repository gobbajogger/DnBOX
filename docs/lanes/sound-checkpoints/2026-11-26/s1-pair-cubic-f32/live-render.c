#include "exact-room.h"
#include "exact-delay.h"
#include "exact-filter.h"
#include "exact-morph.h"
#include "exact-aux.h"
#include "exact-stereo.h"
/* Headless stimulus only. Writes raw stereo f32le (little-endian hosts). */
#include "app/s1c_part.h"
#include "s1c_front.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "tuning-load.h"
float s1_ab_authored_tuning[127];
int s1_ab_has_authored_tuning;
static s1_dry_part part;
static _Alignas(8) unsigned char arena[S1C_ARENA_BYTES];
typedef struct { unsigned frame, pitch, velocity; } tape_event;
static tape_event *event;
static unsigned event_count, cursor, frame_counter;
static FILE *witness;
void s1_external_tape_frame(s1c *core)
{
    if (core!=&part.e) abort();
    while (cursor<event_count && event[cursor].frame==frame_counter) {
        const tape_event *e=&event[cursor++];
        if(!s1_part_note(&part,e->pitch,e->velocity)) abort();
        if(witness) fprintf(witness,"%u %u %u\n",frame_counter,e->pitch,e->velocity);
    }
    ++frame_counter;
}
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
int main(int argc,char **argv){
    if(argc!=5&&argc!=7)return 2;
    s1_ab_has_authored_tuning=load_s1_authored_tuning(s1_ab_authored_tuning);
#if S1_MORPH_EXACT
    if(!exact_morph_load())return 6;
#endif
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
    unsigned active=0,total=0;
    if (argc==7) {
        unsigned tapepreset=0;
        float seq=0;
        FILE *in=fopen(argv[5],"rb");
        if(!in||fscanf(in,"S1T1 %u %u %u %f",&tapepreset,&total,&active,&seq)!=4||tapepreset!=index||active>1||total!=264728) return 6;
        tape_event e;
        int count;
        while((count=fscanf(in,"%u %u %u",&e.frame,&e.pitch,&e.velocity))==3){
            if(!active||e.frame>=total||e.pitch>127||e.velocity>127||(event_count&&e.frame<event[event_count-1].frame))return 6;
            event=realloc(event,(event_count+1)*sizeof *event);
            if(!event) return 6;
            event[event_count++]=e;
        }
        if(count!=EOF||fclose(in))return 6;
        witness=fopen(argv[6],"wb"); if(!witness)return 6;
        fprintf(witness,"S1T1 %u %u %u %.9g\n",index,total,active,seq);
    }
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
        if(!active) s1_part_note(&part,notes[i],64+11*i);
        render(11025,out);
        if(!s1c_front_set(&part.e,139,(float)(i&1u)))abort();
        if(!s1c_front_set(&part.e,64,(float)(67u+19u*i)))abort();
        if(!s1c_front_set(&part.e,43,i&1u?.304111f:2.5f))abort();
        if(!s1c_front_set(&part.e,42,i&1u?.981f:.1f))abort();
        if(!s1c_front_set(&part.e,44,i&1u?.88f:0.f))abort();
        fprintf(stderr,"frame %u sync %.2f BPM %.6g time %.9g feedback %.9g mix %.9g arp %.0f\n",
           frame_counter,part.e.t[139],part.e.t[64],part.e.t[43],part.e.t[42],part.e.t[44],part.e.t[62]);
        render(22050,out);
        if(!active) s1_part_note(&part,notes[i],0);
    }
    render(66150,out);
    if(fclose(out))return 2;
#if S1C_EXT
    printf("%u %s delay_on %.3g reverb_on %.3g delay_decimation %d pool_bytes %d\n",
           index,argv[4],part.e.t[S1C_P_DLON],part.e.t[S1C_P_RVON],
           part.e.ddec,part.e.pool_bytes);
#endif
#if S1_ROOM_EXACT
    exact_room_drop(part.e.host_exact_room);
#endif
#if S1_DELAY_EXACT
    dnbox_s1_delay_free(part.e.host_exact_delay);
#endif
#if S1_FILTER_EXACT
    for(unsigned i=0;i<S1C_VOICES;++i)exact_voice_filter_drop(part.e.v[i].host_exact_filter);
#endif
#if S1_AUX_EXACT
    for(unsigned i=0;i<S1C_VOICES;++i)exact_aux_drop(part.e.v[i].host_exact_aux);
#endif
#if S1_STEREO_EXACT
    exact_stereo_drop(part.e.host_exact_stereo);
#endif
    free(pack);
    free(event);
    if(argc==7&&(frame_counter!=total||cursor!=event_count||fclose(witness)))return 7;
    return 0;
}
