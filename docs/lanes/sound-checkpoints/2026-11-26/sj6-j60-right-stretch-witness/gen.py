from pathlib import Path
import sys
i,o=sys.argv[1:];s=Path(i).read_text()
def change(a,b):
 global s
 assert s.count(a)==1,a
 s=s.replace(a,b)
s=s.replace('.533294672f*p','.2516540875455158f*p').replace('.533294672f*s->post0[ch]','.2516540875455158f*s->post0[ch]')
a=s.index('    unsigned d=(unsigned)delay;');b=s.index('\n}',a)
s=s[:a]+'''    /* Preserve the READ position of the compact path: delay-derived
       older/younger pair, expressed in original ascending-coordinate order. */
    unsigned d=(unsigned)delay;
    unsigned newer=(s->write-d)&1023u;
    double f=(double)(delay-(float)d);
    double p0=((double)newer-f); if(p0<0.)p0+=1024.;
    double va=(d<=s->filled)?fr[ch][newer]:0.;
    double vb=(d<s->filled)?fr[ch][(newer-1u)&1023u]:0.;
    double val=(double)((float)va+((float)vb-(float)va)*(float)f);
    /* Holding exact written Double history fixed, separate coordinate
       substitution from complete original interpolation, never fitted. */
    if(opt==1 || opt==2) {
      p0=ref[ch][at*7];
      unsigned j=(unsigned)p0;
      va=fr[ch][j];
      vb=fr[ch][(j+1)&1023u];
      f=p0-(double)j;
      val=(opt==1)?fma(f,vb-va,va):ref[ch][at*7+6];
    }
    double *p=got[ch]+at*5;
    p[0]=p0; p[1]=p0-floor(p0); p[2]=va; p[3]=vb; p[4]=val;
    return (float)val;'''+s[b:]
change('        c->mode_history[ch][s->write]=j60_quantize(h);','''        (void)h;
        fr[ch][s->write]=(opt>=4)?(double)j60_quantize(h)*(1./4096.):auth[ch][at];''')
change('displacement*.967567563f*fm_fabsf(ph)', 'displacement*(((opt==3 || opt==5) && ch==1)?1.f:.967567563f)*fm_fabsf(ph)')
Path(o).write_text(s)
