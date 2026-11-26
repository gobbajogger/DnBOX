#include <stdint.h>
#include <stdio.h>
#include "drum808.h"
int main(void){
    drum808_t d;
    const int sizes[]={0,1,31,32,33,63,64,127,256};
    for(int track=0;track<D8_NUM;track++){
        drum808_init(&d);
        for(int block=0;block<1200;block++){
            if(block==4||block==1100)drum808_trigger(&d,track,block==4?1.0f:0.3f);
            float l[256]={0},r[256]={0},v[256]={0},w[256]={0};
            int n=sizes[block%9],before=drum808_active(&d);
            drum808_render_st(&d,l,r,v,w,n);
            int after=drum808_active(&d);
            float meter=drum808_env(&d,track);
            if(fwrite(&before,4,1,stdout)!=1||fwrite(&after,4,1,stdout)!=1||fwrite(&meter,4,1,stdout)!=1)return 1;
            for(int s=0;s<n;s++)if(fwrite(&l[s],4,1,stdout)!=1||fwrite(&r[s],4,1,stdout)!=1||fwrite(&v[s],4,1,stdout)!=1||fwrite(&w[s],4,1,stdout)!=1)return 2;
        }
    }
}
