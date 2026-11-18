#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static void advance(int n){int32_t out[512];while(n){int k=n>256?256:n;engine_render(out,k);n-=k;}}
int main(void){
 setup(V_PART,PART_808);seq.playing=0;engine_guard_enable(0);advance(2048);
 pref_t sound=PR(R_ENG,T_808,D8_CP,4),attack=PR(R_ENG,T_808,D8_CP,3);
 pref_set(sound,1);pref_set(attack,127);advance(256);engine_drum(PART_808,D8_CP,1);advance(256);
 drum_curve_t a,b;CHECK(engine_drum808_param_curve(D8_CP,3,&a));if(a.elapsed<0)fprintf(stderr,"MA inactive after trigger: off=%u target=%d level=%g rise=%g\n",seq.part_off,pref_get(sound),(double)a.level,(double)a.rise_end);CHECK(a.elapsed>=0);
 pref_set(sound,0);advance(256);engine_drum(PART_808,D8_CP,1);advance(256);
 CHECK(engine_drum808_param_curve(D8_CP,3,&b));
 CHECK(b.delay==0 && b.elapsed>a.elapsed+10); /* MA epoch, not newly triggered CP. */
 CHECK(engine_drum808_curve(D8_CP,&a));CHECK(a.delay>0 && a.elapsed<b.elapsed);
 CHECK(!engine_drum808_param_curve(-1,3,&a));CHECK(!engine_drum808_param_curve(D8_CP,4,&a));
 puts("PASS fixed MARACAS audio owner and independent hit epoch after CLAP adoption");return 0;
}
