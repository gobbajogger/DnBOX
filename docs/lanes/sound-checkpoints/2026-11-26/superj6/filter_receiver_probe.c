#include "filter_adsr.h"
#include "filter_panel.h"
#include "normal_cutoff.h"
#include <stdio.h>
#include <assert.h>

int main(void)
{
    sj6_filter_adsr env;
    sj6_filter_config(&env,44100.f,sj6_filter_attack_panel(.5168501734733582f),
       sj6_filter_decay_panel(.47146177291870117f),
       sj6_filter_sustain_panel(.08876017481088638f),
       sj6_filter_release_panel(.604790449142456f));
    sj6_filter_on(&env);
    const double actual[]={300.5797293186188,301.1592575311661,301.7385845184326};
    for(unsigned n=0;n<3;n++){
        const float shape=sj6_filter_prep(&env);
        const double fc=sj6_normal_cutoff(300.f,.09260699152946472f,.9971304535865784f,
                                         .6771653294563293f,shape,.13283398747444153f,65.40638732910156f);
        printf("source prep %u: shape=%.10g target=%.15f expected=%.15f\n",n+1,(double)shape,fc,actual[n]);
        assert(fc==actual[n]);
    }
    puts("PASS: composed FILTER envelope and actual oldest normal cutoff caller, exact preps");
}
