#include "selector.h"
#include <assert.h>
#include <stdio.h>
static sj6_band check(float x)
{
    unsigned row=0;
    x*=64.f;
    if(x>=.49f)do{x*=.5f;++row;}while(row<6u&&x>=.49f);
    float a=row==6u?0.f:(x-.4f)/.09f;
    if(a<0.f)a=0.f;if(a>1.f)a=1.f;
    return (sj6_band){row,a};
}
int main(void)
{
    const unsigned lo=sj6_as_uint(.35f/64.f), hi=sj6_as_uint(.49f);
    unsigned c=0;
    for(unsigned u=lo;u<=hi;u++){
        const float s=sj6_as_float(u);
        const sj6_band a=check(s), b=sj6_wavetable_band(s);
        assert(a.row==b.row && sj6_as_uint(a.blend)==sj6_as_uint(b.blend));
        ++c;
    }
    assert(sj6_wavetable_band(0.f).blend==0 && sj6_wavetable_band(0.f).row==0);
    printf("Reference capped-halving resolver exactly equal for %u contiguous Float32 pitch words; at most constant work. No oscillator or table loaded.\n",c);
}
