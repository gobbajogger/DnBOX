/* SPDX-License-Identifier: GPL-3.0-only */
#include "juno6.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
enum {N=8192};
static float pcm[3][2][N];
#define C(x) do {if(!(x)){fprintf(stderr,"FAIL line %d\n",__LINE__);return 1;}}while(0)
static int render(juno6_t s,unsigned n)
{
    if(!juno6_note_on(&s,57,100))return 0;
    for(unsigned i=0;i<N;i+=256)
        if(!juno6_render_stereo(&s,pcm[n][0]+i,pcm[n][1]+i,256))return 0;
    return 1;
}
int main(void)
{
    for(unsigned owner=0;owner<2;++owner){
        juno6_t s,a,b,save;
        juno6_init(&s,44100); s.core.layer[0].params.pan=-1.f; s.core.layer[1].params.pan=1.f;
        dnbx_layer_params left=s.core.layer[0].params, right=s.core.layer[1].params;
        float raw,p,q;
        C(dnbx_attack_from_unit(.25f,&p)==DNBX_OK);
        C(dnbx_attack_from_unit(.5f,&q)==DNBX_OK);
        C(juno6_set(&s,JUNO6_APP_PARAM_BASE+119u,.25f));
        C(juno6_apply_sj6_primary_attack(&s,.5f,0));
        C(s.core.layer[0].params.amp.attack_s==q && s.core.layer[1].params.amp.attack_s==p);
        a=s;
        C(juno6_apply_sj6_primary_attack(&s,.5f,1));
        C(s.core.layer[0].params.amp.attack_s==q && s.core.layer[1].params.amp.attack_s==q);
        C(dnbx_read_raw(&s.core,36,&raw)==DNBX_OK && raw==.5f);
        C(dnbx_read_raw(&s.core,119,&raw)==DNBX_OK && raw==.5f);
        b=s;
        C(juno6_set(&s,JUNO6_APP_PARAM_BASE+119u,.25f));
        C(s.core.layer[0].params.amp.attack_s==q && s.core.layer[1].params.amp.attack_s==p);
        C(dnbx_read_raw(&s.core,36,&raw)==DNBX_OK && raw==.5f);
        C(dnbx_read_raw(&s.core,119,&raw)==DNBX_OK && raw==.25f);
        C(!memcmp(&left.filter,&s.core.layer[0].params.filter,sizeof left.filter));
        C(!memcmp(&right.filter,&s.core.layer[1].params.filter,sizeof right.filter));
        C(right.amp.decay_s==s.core.layer[1].params.amp.decay_s && right.amp.sustain==s.core.layer[1].params.amp.sustain && right.amp.release_s==s.core.layer[1].params.amp.release_s);
        C(render(a,0)&&render(b,1)&&render(s,2));
        C(!memcmp(pcm[0][0],pcm[1][0],sizeof pcm[0][0]));
        C(memcmp(pcm[0][1],pcm[1][1],sizeof pcm[0][1]));
        C(!memcmp(pcm[0],pcm[2],sizeof pcm[0]));
        C(juno6_apply_sj6_primary_attack(&s,.5f,1));
        C(render(s,2) && !memcmp(pcm[1],pcm[2],sizeof pcm[1]));
        save=s;
        C(!juno6_apply_sj6_primary_attack(&s,NAN,1) && !memcmp(&save,&s,sizeof s));
        C(!juno6_apply_sj6_primary_attack(&s,.5f,2) && !memcmp(&save,&s,sizeof s));
        C(dnbx_set_filter_link(&s.core,1)==DNBX_OK); save=s;
        C(!juno6_apply_sj6_primary_attack(&s,.5f,1) && !memcmp(&save,&s,sizeof s));
        C(dnbx_set_filter_link(&s.core,0)==DNBX_OK && juno6_note_on(&s,57,100)); save=s;
        C(!juno6_apply_sj6_primary_attack(&s,.5f,1) && !memcmp(&save,&s,sizeof s));
        printf("PASS owner %u: Attack 36 independently or to 119; later 119 is secondary only; actual stereo divergence and exact restore; active and filter-linked refused\n",owner);
    }
    return 0;
}
