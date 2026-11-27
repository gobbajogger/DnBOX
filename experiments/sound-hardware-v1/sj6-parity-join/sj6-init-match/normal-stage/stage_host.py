import json,sys
from pathlib import Path
D=Path(__file__).resolve().parent
p=Path(sys.argv[1]);p.mkdir(exist_ok=True)
raw=(D.parent/'render.c').read_text()
raw=raw.replace('static original_runtime_part p;','''int dnbox_init_normal_lab;
static unsigned taps;
static FILE *tapfile;
static const double coeff[5]={'''+','.join(json.loads((D/'result.json').read_text())['cases'][0]['coeff_hex'])+'''};
float dnbox_init_normal_sample(float x,float *x1,float *x2,float *y1,float *y2)
{
    double z=fma(coeff[0],x,coeff[1]*(double)*x1);
    z=fma(coeff[2],*x2,z);
    z=fma(-coeff[3],*y1,z);
    z=fma(-coeff[4],*y2,z);
    float y=(float)z;
    *x2=*x1;*x1=x;*y2=*y1;*y1=y;
    if(tapfile&&taps<2048u){
        /* With one voice, two layers are called in stable alternating order.
         * Chord occurs later, outside this first-note signed tap. */
        if(taps++%2u==0u){
           float pair[2]={x,y};
           if(fwrite(pair,sizeof(float),2,tapfile)!=2) abort();
        }
    }
    return y;
}
static original_runtime_part p;''').replace(' REQUIRED(argc==1);\n (void)argv;',''' REQUIRED(argc==3 && (argv[1][0]=='0'||argv[1][0]=='1') && !argv[1][1]);
 dnbox_init_normal_lab=argv[1][0]=='1';
 if(dnbox_init_normal_lab){tapfile=fopen(argv[2],"wb");REQUIRED(tapfile);}''')
raw=raw.replace(' unsigned f=0,e=0;',''' /* One enabled saw and explicit fixed Normal filter controls, no Room,
     Unison or master adjustment. Not a claim of factory Init defaults. */
 for(unsigned layer=0;layer<2;++layer){
     dnbx_layer_params layerp;
     REQUIRED(dnbx_get_layer(&p.dsp.juno.synth.core,layer,&layerp)==DNBX_OK);
     layerp.cutoff_hz=4000.f;layerp.resonance=0.f;layerp.filter_env_hz=0.f;
     if(layer)layerp.gain=0.f;
     REQUIRED(dnbx_set_layer(&p.dsp.juno.synth.core,layer,&layerp)==DNBX_OK);
 }
 unsigned f=0,e=0;''')
raw=raw.replace('pass unmodified compact Init, frames=', 'pass dry Init Normal fixture, frames=').replace(' return 0;\n}',' if(tapfile) REQUIRED(taps==2048u && fclose(tapfile)==0);\n return 0;\n}')
(p/'render.c').write_text(raw)
