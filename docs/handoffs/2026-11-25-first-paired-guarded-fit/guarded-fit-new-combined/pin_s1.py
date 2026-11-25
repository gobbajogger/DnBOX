from pathlib import Path
from subprocess import run
from hashlib import sha256
import tempfile,re,json
sdk=Path('../prereqs/jieli-linux-toolchains-20260730.1').resolve()
app=Path('tree/firmware/src/app').resolve()
actual=(app/'s1c_part.c').read_text()
defines=actual[actual.index('#define S1_CATALOG_ROWS'):actual.index('static const s1_patch_descriptor patch_parameters')]
a=actual.index('static const uint8_t patch_slot_code')
slot=actual[a:actual.index('};',a)+2]
a=actual.index('static const float patch_domain')
domain=actual[a:actual.index('};',a)+2]
enum=re.search(r'enum \{ P_CUT = 16,.*?\};',actual)[0]
hdr='#include <stdint.h>\n#include "s1_patch.h"\n#include "../dsp/s1c/s1c_front.h"\n'
flags=['-DX0X_S1_BEND_RANGE_PROBE=1','-DX0X_S1C=1','-DX0X_SYNTHONE_PATCH=1']
pins={}
with tempfile.TemporaryDirectory(prefix='s1-readonly-abi-') as d:
    root=Path(d)
    c=root/'pin.c'; obj=root/'pin.o'
    c.write_text(hdr+enum+'\n'+defines+'\n'+slot+'\n'+domain+'\nconst void *const pins[]={patch_slot_code,patch_domain};\n')
    run([sdk/'pi32v2/bin/clang','-target','pi32v2','-fdata-sections','-Oz','-I'+str(app),*flags,'-c',c,'-o',obj],check=True)
    for tag,n in [('patch_domain',144),('patch_slot_code',150)]:
        out=root/'exact'
        run([sdk/'common/bin/objcopy','-O','binary','-j','.rodata.'+tag,obj,out],check=True)
        b=out.read_bytes()
        assert len(b)==n
        pins['.rodata.'+tag]=[n,sha256(b).hexdigest()]
print(json.dumps(pins))
