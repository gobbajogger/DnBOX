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
    if(strcmp(argv[1],"room")) return 4;
    static project_t p;
    if(fread(&p,1,sizeof p,stdin)!=sizeof p || fgetc(stdin)!=EOF)return 5;
    uint8_t *const cell=p.sound.v[T_303][0], *const extra=p.sound.v[T_303][S1_EXTRA_ROW];
    if (!original_sound_s1_set(cell,S1_DELAY_ON,0) ||
        !original_sound_s1_set(cell,S1_REVERB_ON,1) ||
        !original_sound_s1_extra_set(cell,extra,S1_REVERB_MIX,97) || !original_sound_s1_extra_set(cell,extra,S1_WIDEN,2)) return 6;
    static const bass_identity_entry_t s1[] = {
      {ORIGINAL_SOUND_SYNTHONE_ID, ORIGINAL_SOUND_SCHEMA, ORIGINAL_SYNTHONE_NPARAMS, 1}
    };
    bass_sound_info_t info;
    if (bass_sound_tag_decode(&info,&p.sound.bass[0],s1,1) != BASS_IDENTITY_READY ||
        !original_sound_valid(cell) || !original_sound_s1_extra_valid(cell,extra) ||
        !original_sound_is_edited(cell)) return 7;
    info.flags = BASS_SOUND_EDITED;
    if (bass_sound_tag_encode(&p.sound.bass[0],&info,s1,1) != BASS_IDENTITY_READY) return 8;
    return fwrite(&p,1,sizeof p,stdout)!=sizeof p;
}
