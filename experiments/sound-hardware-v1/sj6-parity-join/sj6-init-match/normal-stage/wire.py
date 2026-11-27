import sys
from pathlib import Path
a,b=map(Path,sys.argv[1:]);s=a.read_text()
k='''                lv->filter_first=fm_clampf(lv->filter_first+a*(x-lv->filter_first-p->resonance*.8f*lv->filter_second),-4,4);
                lv->filter_second=fm_clampf(lv->filter_second+a*(lv->filter_first-lv->filter_second),-4,4);
                mix[l]+=velocity*amp*lv->filter_second;'''
assert s.count(k)==1
v='''#if defined(X0X_HOST) && DNBOX_INIT_NORMAL_LAB
                extern int dnbox_init_normal_lab;
                extern float dnbox_init_normal_sample(float x,float *,float *,float *,float *);
                if(dnbox_init_normal_lab && p->oscillator_mode==DNBX_OSC_SUPERJ6) {
                    /* History is mutually exclusive with OTA. A fresh
                       voice clears all four words in the existing loader. */
                    mix[l]+=dnbox_init_normal_sample(x*(velocity*amp),&lv->filter_first,
                               &lv->filter_second,&lv->ota_tail[0],&lv->ota_tail[1]);
                } else
#endif
                {
'''+k+'''
                }'''
# For flag off preprocessing must remove added braces as well.
v=v.replace('''                } else
#endif
                {
''','''                } else {
#endif
''').replace(k+'\n                }',k+'\n#if defined(X0X_HOST) && DNBOX_INIT_NORMAL_LAB\n                }\n#endif')
b.write_text(s.replace(k,v))
