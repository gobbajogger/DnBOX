from pathlib import Path
d=Path(__file__).parent;p=d/'firmware/src/dsp/s1c/s1c.c';s=p.read_text()
a=s.index('static uint8_t f8_put');b=s.index('#endif',a)
s=s[:a]+'''/* E4M1 in aligned 4-sample cells. Every tap can read arbitrary history
 * in at most two byte loads, preserving the existing warm regrid order. */
static unsigned f8_put(float v)
{
   s1c_fb x={.f=v};
   unsigned m=(x.u&0x7fffffffu)+(1u<<21),e=m>>23;
   if(e<115)return 0;
   unsigned q=e>129?31u:((e-114u)<<1)|(m>>22&1u);
   return ((x.u>>26)&32u)|q;
}
static float f8_get(unsigned v)
{
   s1c_fb b={.u=v ? ((v&32u)<<26)|(((v&31u)>>1)+114u)<<23|((v&1u)<<22):0};
   return b.f;
}
static unsigned get6(const uint8_t *b,unsigned p)
{
   const unsigned x=p*6u, byte=x>>3, sh=x&7u;
   unsigned w=b[byte];
   if(sh>2u)w|=(unsigned)b[byte+1u]<<8;
   return (w>>sh)&63u;
}
static void put6(uint8_t *b,unsigned p,unsigned v)
{
   const unsigned x=p*6u, byte=x>>3, sh=x&7u;
   b[byte]=(uint8_t)((b[byte]&~(63u<<sh))|(v<<sh));
   if(sh>2u)b[byte+1u]=(uint8_t)((b[byte+1u]&~(63u>>(8u-sh)))|(v>>(8u-sh)));
}
'''+s[b:]
s=s.replace('f8_get(b[i])','f8_get(get6(b,i))').replace('f8_get(b[j])','f8_get(get6(b,j))')
s=s.replace('((uint8_t *)(void *)d->m)[d->w]=f8_put(in+d->prev*fb);','put6((uint8_t *)(void *)d->m,d->w,f8_put(in+d->prev*fb));')
s=s.replace('const int short_line=s->pool_bytes/5;', 'const int short_line=4*(s->pool_bytes/15);')
s=s.replace('pool+=d->len;', 'pool+=3*(d->len/4);')
s=s.replace('while(i<j){uint8_t x=a[i];a[i++]=a[j];a[j--]=x;}','while(i<j){unsigned x=get6(a,i);put6(a,i++,get6(a,j));put6(a,j--,x);}')
s=s.replace('f8_get(b[--from])','f8_get(get6(b,--from))').replace('b[--to]=f8_put(sum/(float)stride);','put6(b,--to,f8_put(sum/(float)stride));').replace('if(to)memset(b,0,(size_t)to*sizeof *b);','for(int j=0;j<to;j++)put6(b,j,0);')
p.write_text(s)
