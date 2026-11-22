#include <assert.h>
#include <stdio.h>
#include "original_runtime.h"
static original_runtime_part part;
static original_runtime_workspace work;
int main(int argc,char **argv)
{
    assert(argc==2);
    assert(original_sound_factories(ORIGINAL_SOUND_JUNO)==4);
    for(unsigned program=0;program<original_sound_factories(ORIGINAL_SOUND_JUNO);++program) {
        uint8_t cell[32]; char path[600]; float l[256],r[256];
        snprintf(path,sizeof path,"%s/preset-%u.f32le",argv[1],program);
        FILE *f=fopen(path,"wb"); assert(f);
        assert(original_sound_init(cell,ORIGINAL_SOUND_JUNO,program));
        assert(original_runtime_init(&part,cell,120,0,0)==ORIGINAL_RUNTIME_OK);
        assert(original_runtime_key(&part,ORIGINAL_INPUT_KEYS,60,100)==ORIGINAL_RUNTIME_OK);
        for(unsigned i=0;i<128;++i) {
            if(i==48) assert(original_runtime_key(&part,ORIGINAL_INPUT_KEYS,60,0)==ORIGINAL_RUNTIME_OK);
            assert(original_runtime_render(&part,&work,l,r,256)==ORIGINAL_RUNTIME_OK);
            for(unsigned k=0;k<256;++k) {
                assert(fwrite(l+k,4,1,f)==1 && fwrite(r+k,4,1,f)==1);
            }
        }
        assert(fclose(f)==0);
    }
    return 0;
}
