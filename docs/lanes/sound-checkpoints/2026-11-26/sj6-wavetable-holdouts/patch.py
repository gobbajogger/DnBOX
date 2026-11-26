from pathlib import Path
import sys
p=Path(sys.argv[1]);s=p.read_text()
mark='static float factory_oscillator('
i=s.index(mark)
s=s[:i]+'''#if defined(X0X_HOST) && DNBX_WAVE_FAMILY
#include "family.h"
#endif
'''+s[i:]
a='    float t = voice->phase, dt = voice->increment;'
b=a+'''
#if defined(X0X_HOST) && DNBX_WAVE_FAMILY
    if(stop<2u)
        return sj6_family(stop,t,dt);
#endif'''
assert s.count(a)==1;p.write_text(s.replace(a,b))
