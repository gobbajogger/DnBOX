from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
lo=s.index('static float clock_2x')
hi=s.index('static inline float physical_tick',lo)
s=s[:lo]+'''void prepare_tile(const float *source, float *dst){
    const float alpha=source[0]/(1.f+source[0]);
    dst[0]=alpha; dst[1]=1.f-alpha;
    dst[2]=source[1]; dst[3]=source[2]; dst[4]=source[3];
}
static float clock_2x(OTA *s,float x,const float *q,const float *c) {
    const float dv=(x-s->condition)*c[2];
    const float lp=s->condition+dv;
    s->condition=dv+lp;
    float z=(x-lp)*q[4],lo2=0;
    for(unsigned i=0;i<4;i++){
        const float last=s->pole[i];
        z=fmaf(c[0],z,c[1]*last);
        s->pole[i]=fmaf(2.f,z,-last);
        if(i==1)lo2=z;
    }
    const float lo4=z*c[4];
    if(c[3]==1.f)return lo4;
    if(c[3]==0.f)return lo2;
    return fmaf(c[3],lo4-lo2,lo2);
}
'''+s[hi:]
s=s.replace('coeff,params[0],params[1],params[2],params[3]','coeff,params')
s=s.replace('    OTA s={0};','    OTA s={0}; float c[5]; prepare_tile(params,c);')
s=s.replace('physical_tick(&s,x[i],params,coeff)','physical_tick(&s,x[i],c,coeff)')
s+='''\nfloat physical_tile_tick(OTA *s,float x,const float *tile,const float*coeff){return physical_tick(s,x,tile,coeff);}'''
Path(sys.argv[2]).write_text(s)
