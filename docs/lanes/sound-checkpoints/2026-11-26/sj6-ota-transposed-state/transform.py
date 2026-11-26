from pathlib import Path
import sys

source = Path(sys.argv[1]).read_text()
a = source.index('typedef struct')
b = source.index('static float clock_2x', a)
source = source[:a] + '''/* Fixed-coefficient transposed first-order all-pass. Coefficient
 * replacement with persistent history is not equivalent to direct form. */
typedef struct {float up[2][2],down[2][2],pole[4],condition;} OTA;
_Static_assert(sizeof(OTA)==52, "transposed history layout");
static inline float ap_pair(float x,float z[2],float a,float b) {
    float mid=fmaf(a,x,z[0]);
    z[0]=fmaf(-a,mid,x);
    float y=fmaf(b,mid,z[1]);
    z[1]=fmaf(-b,y,mid);
    return y;
}
''' + source[b:]
a = source.index('static inline float physical_tick')
b = source.index('__attribute__', a)
source = source[:a] + '''static inline float physical_tick(OTA *s, float x,const float *params,const float *coeff) {
    float a=ap_pair(x,s->up[0],coeff[0],coeff[1]);
    float b=ap_pair(x,s->up[1],coeff[2],coeff[3]);
    a=clock_2x(s,a,coeff,params[0],params[1],params[2],params[3]);
    b=clock_2x(s,b,coeff,params[0],params[1],params[2],params[3]);
    b=ap_pair(b,s->down[0],coeff[0],coeff[1]);
    a=ap_pair(a,s->down[1],coeff[2],coeff[3]);
    return 40.f*((a+b)*.5f);
}
''' + source[b:]
Path(sys.argv[2]).write_text(source)
