from pathlib import Path
import sys
p=Path(sys.argv[1])
s=p.read_text().split('__attribute__((visibility("default"))) void physical_replay',1)[0]
s=s.replace('static inline float physical_tick(', 'float physical_tick(')
Path(sys.argv[2]).write_text(s)
s=s.replace('static float clock_2x(OTA *s,float x,','static float clock_2x(float pole[4],float *condition,float x,')
s=s.replace('s->condition','(*condition)').replace('s->pole','pole')
off=s.index('float physical_tick(')
s=s[:off]+'''
typedef struct { Expansion up; float condition; Contraction right; } GapVoice;
typedef struct { GapVoice v[6]; float left_input_0; } GapA;
typedef struct { PathMemory left_01_23[4]; } GapB;
typedef struct { float left_input_1_3[3]; } GapC;
typedef struct { float left_input_4_5[2]; PathMemory left_4_5[2]; } Extension;
_Static_assert(sizeof(GapA)==220 && sizeof(GapB)==32 && sizeof(GapC)==12
            && sizeof(Extension)==24 && _Alignof(GapA)==_Alignof(float), "layout");
static float tick_split(GapVoice *s,float pole[4],PathMemory *left,float *left_input,
                        float x,const float *params,const float *coeff) {
        float a=ap_pair(x,s->up.input,&s->up.branch[0],coeff[0],coeff[1]);
        float b=ap_pair(x,s->up.input,&s->up.branch[1],coeff[2],coeff[3]);
        s->up.input=x;
        a=clock_2x(pole,&s->condition,a,coeff,params[0],params[1],params[2],params[3]);
        b=clock_2x(pole,&s->condition,b,coeff,params[0],params[1],params[2],params[3]);
        const float prev_b=*left_input,prev_a=s->right.input;
        *left_input=b;
        s->right.input=a;
        b=ap_pair(b,prev_b,left,coeff[0],coeff[1]);
        a=ap_pair(a,prev_a,&s->right.branch,coeff[2],coeff[3]);
        return 40.f*((a+b)*.5f);
}
float admit_split(GapA *a,GapB *b,GapC *c,Extension *e,float pole[4],
                  float x,const float *p,const float *co,unsigned mask,unsigned voice,float fallback){
    if(((mask>>voice)&1u)==0)return fallback;
    PathMemory *left=voice<4 ? &b->left_01_23[voice] : &e->left_4_5[voice-4];
    float *input=voice==0 ? &a->left_input_0 : voice<4 ? &c->left_input_1_3[voice-1] : &e->left_input_4_5[voice-4];
    return tick_split(&a->v[voice],pole,left,input,x,p,co);
}
'''
Path(sys.argv[3]).write_text(s)
