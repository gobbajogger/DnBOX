from pathlib import Path
import sys
f=Path(sys.argv[1]);s=f.read_text()
a='D9_INLINE float d9_triangle_diode(float x, float drive, float gain)'
assert s.count(a)==1
h='''/* The only callers are fixed 1.6x and 2.0x diodes with |input| <= 1.
 * Both lookup taps stay below the first packed-tail cell at index 400. */
D9_INLINE float d9_fixed_triangle_table(float a)
{
#if X0X_TANH_SPLIT_LAB && X0X_TANH_LAB_SPLIT > 257
    _Static_assert(X0X_TANH_LAB_SPLIT > 2 * ((int)X0X_TANH_SCALE + 1) + 1, "re-prove fixed triangle tap range");
    const int32_t i=(int32_t)a;
    const float fr=a-(float)i;
    const float t0=x0x_tanh_lab_head[(unsigned)i],t1=x0x_tanh_lab_head[(unsigned)i+1u];
    return t0+(t1-t0)*fr;
#else
    return d9_tanh_table(a);
#endif
}
'''
s=s.replace(a,h+a)
assert s.count('d9_tanh_table(fm_fabsf(u) * X0X_TANH_SCALE)')==1
assert s.count('d9_tanh_table(fm_fabsf(coordinate))')==1
s=s.replace('d9_tanh_table(fm_fabsf(u) * X0X_TANH_SCALE)','d9_fixed_triangle_table(fm_fabsf(u) * X0X_TANH_SCALE)').replace('d9_tanh_table(fm_fabsf(coordinate))','d9_fixed_triangle_table(fm_fabsf(coordinate))')
f.write_text(s)
