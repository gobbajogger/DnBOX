"""Transition-only HOST test bridge; do not enable in an acceptance image."""
from pathlib import Path
def one(path,a,b):
 p=Path('firmware/src/app')/path
 s=p.read_text();assert s.count(a)==1,(path,a)
 p.unlink();p.write_text(s.replace(a,b))
one('original_runtime.c',
'''if(p->classic_i_authored && p->dsp.juno.chorus.factory_mode==JUNO6_CHORUS_I) {''',
'''if(p->classic_i_authored && (p->dsp.juno.chorus.factory_mode==JUNO6_CHORUS_I
#if defined(X0X_HOST) && defined(X0X_SJ6_CLASSIC_SWITCH_WITNESS)
               || p->dsp.juno.chorus.factory_mode>=JUNO6_CHORUS_OFF
#endif
              )) {''')
one('original_presets.inc',
'''            memset(typed,SJ6_CLASSIC_I_DEFAULT,SJ6_CLASSIC_I_COUNT);
            sj6_classic_i_apply(sound,v);
        }
#endif''',
'''            memset(typed,SJ6_CLASSIC_I_DEFAULT,SJ6_CLASSIC_I_COUNT);
            sj6_classic_i_apply(sound,v);
        }
#if defined(X0X_HOST) && defined(X0X_SJ6_CLASSIC_SWITCH_WITNESS)
        if (prior!=chorus->factory_mode && (chorus_turns&3u) &&
            live137(sound,v) && s1_typed_tuple_editable() && original_bass[v].classic_i_authored)
            (void)classic_wrapper_enter(&original_bass[v].classic_i_effect,
                                        chorus->factory_mode-JUNO6_CHORUS_OFF);
#endif
#endif''')
