#!/usr/bin/env python3
"""Focused, explicitly flagged compact-profile replacement comparison."""
# Reuse the reviewed command/ELF helpers without executing its build loop.
from pathlib import Path
_script=Path(__file__).with_name('replacement_native.py')
exec(compile(_script.read_text().split('# Native sizeof/offsetof proof')[0],str(_script),'exec'))

OUT=HERE/'compact-variant-native'
OUT.mkdir(exist_ok=True)
commands=[]
previous={}
SOURCE=ROOT/'hardware_candidate/size_variants/compact_profile'
SNAP=OUT/'src'
SNAP.mkdir(exist_ok=True)
expected={'dnbx_core.c':'3d8f898428b297478abb35692de56952451caed38cc02db53e16a19a54a7408b',
          'dnbx_core.h':'86acaf17ee9c48dac151919098ab12de3acf457fab2bd9fdf57ac966f36c0962'}
records=[]
for name,digest in expected.items():
    data=(SOURCE/name).read_bytes()
    assert hashlib.sha256(data).hexdigest()==digest,name
    assert data==(SOURCE/name).read_bytes(),name
    (SNAP/name).write_bytes(data)
    records.append({'source':str(SOURCE/name),'snapshot':str(SNAP/name),'sha256':digest,'bytes':len(data)})
for name in ['juno6.c','juno6.h']:
    data=(TREE/'firmware/src/dsp'/name).read_bytes()
    (SNAP/name).write_bytes(data)
    records.append({'source':str(TREE/'firmware/src/dsp'/name),'snapshot':str(SNAP/name),
                    'sha256':hashlib.sha256(data).hexdigest(),'bytes':len(data)})
(OUT/'PROVENANCE.json').write_text(json.dumps({'captured_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),
    'files':records,'scope':'Same real Juno replacement; only frozen candidate and explicit diagnostic profile changed; capability query retained'},indent=2))
if (SOURCE/'EQUIVALENCE.json').exists():
    (OUT/'builder-EQUIVALENCE.json').write_bytes((SOURCE/'EQUIVALENCE.json').read_bytes())

layout_names=['part','env','params','voice','work','adapter','chorus','macro_offset','hpf_offset','hp_state_offset','hp_input_offset']
layout_expr=['sizeof(dnbx_part)','sizeof(dnbx_envelope_state)','sizeof(dnbx_layer_params)','sizeof(dnbx_voice)',
             'sizeof(dnbx_work)','sizeof(juno6_t)','sizeof(juno6_chorus_t)','offsetof(juno6_t,macro)',
             'offsetof(juno6_t,hpf)','offsetof(juno6_t,hp_state)','offsetof(juno6_t,hp_input)']
(OUT/'layout.c').write_text('#include <stddef.h>\n#include "src/juno6.h"\nconst unsigned variant_layout[]={'+','.join(layout_expr)+'};\n')
run([TC/'pi32v2/bin/clang',*flags,'-I'+str(SNAP),'-Oz','-c',OUT/'layout.c','-o',OUT/'layout.o'],'layout-compile')
run([TC/'common/bin/objcopy','-O','binary','-j','.rodata.variant_layout',OUT/'layout.o',OUT/'layout.bin'],'layout-extract')
layout=dict(zip(layout_names,struct.unpack('<'+'I'*len(layout_names),(OUT/'layout.bin').read_bytes())))
assert layout==dict(part=2264,env=36,params=96,voice=192,work=112,adapter=2356,chorus=536,
                   macro_offset=2264,hpf_offset=2336,hp_state_offset=2340,hp_input_offset=2348),layout
links=[]
for profile in [1,0]:
    replacements={}
    for stem in ['dnbx_core','juno6']:
        obj=OUT/(stem+'-Oz-counters'+str(profile)+'.o')
        run([TC/'pi32v2/bin/clang',*flags,'-Oz','-DDNBX_DIAGNOSTIC_COUNTERS='+str(profile),'-flto',
             '-c',SNAP/(stem+'.c'),'-o',obj],stem+'-counters'+str(profile)+'-compile')
        replacements[stem]=obj
    for model,boarddir in [('elite','smk37-elite'),('pro','smk37')]:
        build.MODEL=model;build.OUT=TREE/'build'/boarddir
        _,objs=build.app_compile_commands([],True,'0')
        for stem,obj in replacements.items():
            old=build.OUT/'original'/(stem+'.o')
            assert objs.count(old)==1;objs[objs.index(old)]=obj
        label=model+'-Oz-counters'+str(profile)
        physical=build.OUT/'app.ld';target=OUT/(label+'.elf')
        cmd=[TC/'pi32v2/bin/ld',*ld_flags,'-T',physical,'-u','dnbx_diagnostic_counters_enabled',*objs,'-o',target]
        p=run(cmd,label+'-physical-link',False)
        if p.returncode:
            diag=OUT/(label+'-DIAGNOSTIC-OVERFLOW.ld')
            t=physical.read_text();assert t.count('LENGTH = 0x86EE0')==1
            diag.write_text(t.replace('LENGTH = 0x86EE0','LENGTH = 0xA0000'))
            target=OUT/(label+'-DIAGNOSTIC-OVERFLOW.elf')
            dc=list(cmd);dc[dc.index('-T')+1]=diag;dc[dc.index('-o')+1]=target
            run(dc,label+'-diagnostic-link')
        detail=inspect(target,label)
        row={'model':model,'optimization':'Oz','diagnostic_counters':profile,
             'physical_link_returncode':p.returncode,'diagnostic_only':bool(p.returncode),
             'free_bytes':552672-detail['load_image_bytes'],'gate_shortfall_bytes':detail['load_image_bytes']-(552672-1024),
             'xip_gate':'PASS' if detail['load_image_bytes']<=552672-1024 else 'FAIL',
             'explicit_extra_link_root':'dnbx_diagnostic_counters_enabled',**detail}
        links.append(row)
        (OUT/'RESULT.json').write_text(json.dumps({'native_layout':layout,'links':links,'provenance':records},indent=2))
        print(json.dumps({k:v for k,v in row.items() if k not in ['sections','symbols']}),flush=True)
print(json.dumps({'native_layout':layout,'completed_links':len(links)},indent=2))
