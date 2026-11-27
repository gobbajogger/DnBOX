/* Strict, disposable native fixture author and exact stock stream compressor. */
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "app/x0x.h"
#include "app/original_sound.h"
#include "app/bass_identity.h"
#include "app/lz.c"
static int sink(void *ctx,const uint8_t *p,uint32_t len){return fwrite(p,1,len,ctx)==len?0:-1;}
int main(int argc,char **argv)
{
    if(argc!=2) return 1;
    if(!strcmp(argv[1],"layout")){
        printf("%zu %zu %zu %zu %zu %zu %zu %zu\n",sizeof(project_t),offsetof(project_t,sound),
               sizeof(sound_t),offsetof(project_t,pat),offsetof(sound_t,v[T_303][0]),
               offsetof(sound_t,v[T_303][ARP_BLOCK_VOICE0]),offsetof(sound_t,bass),
               offsetof(sound_t,v[T_303][S1_EXTRA_ROW]));
        return 0;
    }
    if(!strcmp(argv[1],"encode")){
        static uint8_t payload[sizeof(project_t)-offsetof(project_t,pat)];
        if(fread(payload,1,sizeof(payload),stdin)!=sizeof(payload) || fgetc(stdin)!=EOF)return 3;
        lz_sink_t out={sink,stdout}; return lz_compress(payload,sizeof(payload),&out)!=0;
    }
    if(strcmp(argv[1],"joint")) return 4;
    static project_t p;
    if(fread(&p,1,sizeof p,stdin)!=sizeof p || fgetc(stdin)!=EOF)return 5;
    uint8_t *const cell=p.sound.v[T_303][1];
    static const bass_identity_entry_t sj6[] = {
      {ORIGINAL_SOUND_JUNO_ID, ORIGINAL_SOUND_SCHEMA, ORIGINAL_JUNO_NPARAMS, 1}
    };
    if (!original_sound_init(cell,ORIGINAL_SOUND_JUNO,0) || !original_sound_valid(cell))
        return 6;
    bass_sound_info_t info={{ORIGINAL_SOUND_JUNO_ID,ORIGINAL_SOUND_SCHEMA,
                            ORIGINAL_JUNO_NPARAMS,BASS_IDENTITY_KNOWN},0,0};
    if (bass_sound_tag_encode(&p.sound.bass[1],&info,sj6,1)!=BASS_IDENTITY_READY)
        return 7;
    return fwrite(&p,1,sizeof p,stdout)!=sizeof p;
}
