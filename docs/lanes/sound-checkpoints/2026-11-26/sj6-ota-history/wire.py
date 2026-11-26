from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
start=s.index('typedef struct')
end=s.index('static float clock_2x')
s=s[:start]+'''typedef struct {float mid, out;} PathMemory;
typedef struct {float input; PathMemory branch[2];} Expansion;
typedef struct {float input; PathMemory branch;} Contraction;
typedef struct {Expansion up; Contraction down[2]; float pole[4],condition;} OTA;
_Static_assert(sizeof(OTA)==64, "exact history layout");
static inline float ap_pair(float x,float previous_x,PathMemory *s,float a,float b) {
    const float previous_mid=s->mid;
    const float mid=fmaf(x-previous_mid,a,previous_x);
    const float y=fmaf(mid-s->out,b,previous_mid);
    s->mid=mid; s->out=y; return y;
}
'''+s[end:]
a='''        float a=ap(ap(x,s->up[0][0],coeff[0]),s->up[0][1],coeff[1]);
        float b=ap(ap(x,s->up[1][0],coeff[2]),s->up[1][1],coeff[3]);'''
b='''        float a=ap_pair(x,s->up.input,&s->up.branch[0],coeff[0],coeff[1]);
        float b=ap_pair(x,s->up.input,&s->up.branch[1],coeff[2],coeff[3]);
        s->up.input=x;'''
assert s.count(a)==1;s=s.replace(a,b)
a='''        b=ap(ap(b,s->down[0][0],coeff[0]),s->down[0][1],coeff[1]);
        a=ap(ap(a,s->down[1][0],coeff[2]),s->down[1][1],coeff[3]);'''
b='''        const float prev_b=s->down[0].input,prev_a=s->down[1].input;
        s->down[0].input=b;
        s->down[1].input=a;
        b=ap_pair(b,prev_b,&s->down[0].branch,coeff[0],coeff[1]);
        a=ap_pair(a,prev_a,&s->down[1].branch,coeff[2],coeff[3]);'''
assert s.count(a)==1;s=s.replace(a,b)
Path(sys.argv[2]).write_text(s)
