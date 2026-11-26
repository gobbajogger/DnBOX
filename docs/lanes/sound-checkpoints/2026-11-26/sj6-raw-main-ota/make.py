from pathlib import Path
import sys
p=Path('/tmp/phaser-warm-own.Y0XNbQ/tree/firmware/src/dsp/dnbx_core.c')
import hashlib
assert hashlib.sha256(p.read_bytes()).hexdigest()=='70d0aa0ea1cf2192e50cd51b45dffe01533cde368c0a1778959d39312b0ce759'
s=p.read_text().split('static inline float sj6_ota_stage')[1].split('#endif')[0]
Path(sys.argv[1]).write_text('''#include "dnbx_core.h"
#include "fastmath.h"
static inline float sj6_ota_stage'''+s+'''
__attribute__((visibility("default"))) void witness(const float*x,float*y,unsigned n) {
 dnbx_layer_voice v={0};
 for(unsigned i=0;i<n;i++) y[i]=sj6_ota_filter(&v,x[i],.75f,0.f);
}
''')
