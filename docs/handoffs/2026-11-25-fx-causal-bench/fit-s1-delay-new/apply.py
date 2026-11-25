from pathlib import Path
import sys
d=Path(__file__).parent;s=(d/'base.c').read_text();m=sys.argv[1]
p=d/'firmware/src/dsp/s1c/s1c.c'
start=s.index('/* Device memory model (smallest):')
end=s.index('#define DL_TICK',start)
a=s[start:end]
if 'f8' in m:
 a=a.replace('static float xl_read','''#if X0X_S1_DELAY_HISTORY_NATIVE
/* E5M2 right-sized float delay history, unlimited random-access and no
 * retrigger when crossing time ratios. ±1e-5..±3584 at binary32 source. */
static uint8_t f8_put(float v)
{
    s1c_fb x={.f=v};
    unsigned mag=(x.u&0x7fffffffu)+(1u<<20),exp=mag>>23;
    if(exp<109)return 0;
    return (uint8_t)((x.u>>24&128u)|(exp>138?123u:((exp-108u)<<2)|(mag>>21&3u)));
}
static float f8_get(uint8_t v)
{
    s1c_fb x={.u=v ? ((uint32_t)(v&128u)<<24)
                        |((uint32_t)((v&127u)>>2)+108u)<<23
                        |((uint32_t)(v&3u)<<21) : 0u};
    return x.f;
}
#endif
static float xl_read''')
 a=a.replace('    const uint16_t *b', '''#if X0X_S1_DELAY_HISTORY_NATIVE
    const uint8_t *b=(const uint8_t *)(const void *)d->m;
    return f8_get(b[i])*(1.f-f) + f8_get(b[j])*f;
#else
    const uint16_t *b''').replace('h16_get(b[j]) * f;', 'h16_get(b[j]) * f;\n#endif')
 a=a.replace('    ((uint16_t *)d->buf)[d->w] = h16_put(in + d->prev * fb);','''#if X0X_S1_DELAY_HISTORY_NATIVE
    ((uint8_t *)(void *)d->m)[d->w]=f8_put(in+d->prev*fb);
#else
    ((uint16_t *)d->buf)[d->w] = h16_put(in + d->prev * fb);
#endif''')
 s=s.replace('const int short_line=(s->pool_bytes/2)/5;', 'const int short_line=s->pool_bytes/5;').replace('uint16_t *pool=(uint16_t *)(void *)s->pool;', 'uint8_t *pool=s->pool;').replace('d->buf=(int16_t *)(void *)pool;d->len', 'd->m=(int8_t *)(void *)pool;d->len').replace('delay_reverse(uint16_t *a','delay_reverse(uint8_t *a').replace('while(i<j){uint16_t x=a[i]','while(i<j){uint8_t x=a[i]').replace('uint16_t *b=(uint16_t *)(void *)d->buf;', 'uint8_t *b=(uint8_t *)(void *)d->m;').replace('sum+=h16_get(b[--from]);','sum+=f8_get(b[--from]);').replace('b[--to]=h16_put(sum/(float)stride);','b[--to]=f8_put(sum/(float)stride);')
if 'allpass' in m:
 a=a.replace('static float xl_read(const', 'static float xl_read(')
 a=a.replace('    return f8_get(b[i])*(1.f-f) + f8_get(b[j])*f;', '''    if(del<1.f)return f8_get(b[i])*(1.f-f)+f8_get(b[j])*f;
    float x=f8_get(b[j]),hist;
    memcpy(&hist,d->stg,sizeof hist);
    const float z=f<1.e-5f?f8_get(b[i]):hist+(f/(2.f-f))*(x-d->prev);
    memcpy(d->stg,&x,sizeof x);
    return z;''')
if 'right' in m:
 a=a.replace('    if(del<1.f)', '    if(!d->bn || del<1.f)')
 s=s.replace('d->m=(int8_t *)(void *)pool;d->len=(k<2?2:1)*short_line;', 'd->m=(int8_t *)(void *)pool;d->len=(k<2?2:1)*short_line;d->bn=k!=0;')
if 'wide-right' in m:
 # Keep 2T+2T+T defined overall, but allocate 1 more sample per
 # nominal time unit to right, then use a shared reduced clock for
 # the right pair; select the lean experiment separately.
 raise SystemExit('per-path scratch is not this candidate')
s=s[:start]+a+s[end:]
a='if (dd > 1) { oL *= S1C_DL_WET; oRR *= S1C_DL_WET; }'
if 'nowet' in m:s=s.replace(a,'(void)dd;')
if 'midwet' in m:s=s.replace(a,'if(dd>1){oL*=1.23f;oRR*=1.23f;}')
p.write_text(s)
