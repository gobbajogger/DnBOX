#include "app/original_runtime.h"
#include "dsp/s1c/s1c_front.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
static original_runtime_part owner;
#define part (owner.dsp.synthone)
static _Alignas(8) unsigned char arena[S1C_ARENA_BYTES];
static void render(unsigned count,FILE *o) {
 float l[128],r[128]; while(count){unsigned n=count<128?count:128;s1_part_render(&part,l,r,n);if(o)for(unsigned i=0;i<n;i++){fwrite(l+i,4,1,o);fwrite(r+i,4,1,o);}count-=n;}
}
int main(int argc,char **argv){
 if(argc!=5)return 1; FILE *f=fopen(argv[1],"rb"),*ev=fopen(argv[3],"r"),*o=fopen(argv[4],"wb"); if(!f||!ev||!o)return 2;
 unsigned i=atoi(argv[2]),count,pos=0,k,v,t,id,frames,active; float u,seq;
 if(fseek(f,8+1156*i,SEEK_SET)||fread(&count,4,1,f)!=1||count!=144||fscanf(ev,"S1T1 %u %u %u %f",&id,&frames,&active,&seq)!=4||id!=i||frames!=264728)return 3;
 s1_part_init(&part);s1_part_bind(&part,arena);
#if DNBOX_S1_WET_LEASE_USE
 part.e.pool=owner.dsp.s1_wet_lease.delay;
 part.e.pool_bytes=sizeof owner.dsp.s1_wet_lease.delay;
#endif
 if(!s1_part_program(&part,S1_PART_INIT)) return 8;
 for(unsigned j=0;j<count;j++){ if(fread(&k,4,1,f)!=1||fread(&u,4,1,f)!=1||k>=150)return 4; (void)s1c_front_set(&part.e,k,u);}
 if(active){while(fscanf(ev,"%u %u %u",&t,&k,&v)==3){if(t<pos||t>=frames)return 5;render(t-pos,o);pos=t;s1_part_note(&part,k,v);}render(frames-pos,o);}
 else {const unsigned n[]={48,55,60,64,67,72};render(128,NULL);for(unsigned j=0;j<6;j++){s1_part_note(&part,n[j],64+11*j);render(33075,o);s1_part_note(&part,n[j],0);}render(66150,o);}
 fprintf(stderr,"dec=%d pool=%d width=%d fb=%g\n",(int)part.e.ddec,part.e.pool_bytes,(int)part.e.wpos,part.e.rv.fb);fclose(o);return 0;
}
