from pathlib import Path
import sys
p=Path(sys.argv[1]);t=p.read_text()
a=t.index('static int residual_at');b=t.index('static float cubic_at',a)
t=t[:a]+'''static int zig(unsigned x){return (int)(x>>1)^-(int)(x&1u);}
static inline float inverse_span(unsigned shift)
{
    union { uint32_t u; float f; } x={.u=(127u-shift)<<23};
    return x.f;
}
'''+t[b:]
t=t.replace('''            aa=(float)(org+(int)u*step+residual_at(b+5,bw,u));
            bb=(float)(org+(int)v*step+residual_at(b+5,bw,v));''','''            int eu=0,ev=0;
            if(bw){
                /* Exact checked data widths <=10; indices differ by one or coincide
                 * at a reflection. Union of both fields is <=27 bits. */
                unsigned j=u<v?u:v, bit=j*bw, sh=bit&7u;
                const uint8_t *x=b+5+(bit>>3);
                uint32_t w=x[0] | (uint32_t)x[1]<<8 | (uint32_t)x[2]<<16 | (uint32_t)x[3]<<24;
                unsigned mask=(1u<<bw)-1u;
                int low=zig((w>>sh)&mask), high=zig((w>>(sh+bw))&mask);
                eu=u==j?low:high;ev=v==j?low:high;
            }
            aa=(float)(org+(int)u*step+eu);
            bb=(float)(org+(int)v*step+ev);''')
t=t.replace('float f=(float)u/(float)span,g=(float)v/(float)span;','float inv=inverse_span(shift),f=(float)u*inv,g=(float)v*inv;')
p.write_text(t)
