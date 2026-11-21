#!/usr/bin/env python3
"""Build and measure the isolated real Juno-slot replacement composition."""
from pathlib import Path
import datetime
import hashlib
import importlib.util
import json
import os
import re
import struct
import subprocess
import sys
from types import SimpleNamespace

HERE=Path(__file__).resolve().parent
TREE=HERE/'replacement-tree'
ROOT=HERE.parent.parent
TC=ROOT/'build_inputs/extracted/jieli-linux-toolchains-20260730.1'
SDK=ROOT/'build_inputs/extracted/fw-AC79_AIoT_SDK-AC79NN_SDK_V1.2.14_2026-10-13'
OUT=HERE/'replacement-native'
OUT.mkdir(exist_ok=True)
commands=json.loads((OUT/'COMMANDS.json').read_text()) if (OUT/'COMMANDS.json').exists() else []
previous={c['label']:c for c in commands}
env=dict(os.environ,JIELI_TOOLCHAIN=str(TC),AC79_SDK=str(SDK),X0X_GEN_CACHE=str(HERE.parent/'gen-cache'))

def run(args,label,check=True):
    args=list(map(str,args))
    old=previous.get(label)
    if old and old['argv']==args and (OUT/(label+'.log')).exists():
        # Resume the interrupted read-only analysis. Fixture source and argv
        # are unchanged; the earlier compiler/linker receipts remain intact.
        p=SimpleNamespace(returncode=old['returncode'],stdout=(OUT/(label+'.log')).read_text(),stderr='')
        if check and p.returncode:raise RuntimeError('Prior command failed: '+label)
        return p
    start=datetime.datetime.now(datetime.timezone.utc).isoformat()
    p=subprocess.run(list(map(str,args)),cwd=TREE,env=env,text=True,capture_output=True)
    (OUT/(label+'.log')).write_text(p.stdout+p.stderr)
    commands.append({'label':label,'argv':list(map(str,args)),'cwd':str(TREE),'start_utc':start,
                     'end_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'returncode':p.returncode})
    (OUT/'COMMANDS.json').write_text(json.dumps(commands,indent=2))
    if check and p.returncode:raise RuntimeError(label+': '+p.stderr[-1800:])
    return p

def inspect(path,label):
    sec=run([TC/'common/bin/objdump','-h',path],label+'-sections').stdout
    sym=run([TC/'common/bin/objdump','-t',path],label+'-symbols').stdout
    run([TC/'common/bin/objdump','-d',path],label+'-disassembly')
    sections={m.group(1):{'bytes':int(m.group(2),16),'vma':int(m.group(3),16)}
              for m in re.finditer(r'^\s*\d+\s+(\.\S+)\s+([0-9a-f]+)\s+([0-9a-f]+)',sec,re.M)}
    symbols=[{'address':int(m.group(1),16),'section':m.group(2),'bytes':int(m.group(3),16),'name':m.group(4)}
             for m in re.finditer(r'^([0-9a-f]+)\s+.*?(\.[A-Za-z_][\w.]*)\s+([0-9a-f]+)\s+(\S+)\s*$',sym,re.M)]
    undefined=[line.strip() for line in sym.splitlines() if '*UND*' in line and line.strip().split()[-1]!='00000000']
    soft=sorted(set(re.findall(r'\s(__(?:add|sub|mul|div|neg|cmp|eq|ne|lt|le|gt|ge|un|extendsf|truncdf|fixdf|fixunsdf|floatsidf|floatunsidf|floatdidf)\w*df\w*)$',sym,re.M)))
    sections['.text']['lma']=sections['.text']['vma']
    for section,symbol in [('.ram_text','_rt_load'),('.c1_text','_c1_load'),('.data','_data_load')]:
        match=re.search(r'^([0-9a-f]+).*\s'+symbol+r'$',sym,re.M)
        assert match,symbol
        sections[section]['lma']=int(match.group(1),16)
    load_end=max(sections[n]['lma']+sections[n]['bytes'] for n in ['.text','.ram_text','.c1_text','.data'])
    image_bytes=((load_end-0x02000120+3)//4)*4+4
    return {'elf':str(path),'elf_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'sections':sections,
            'symbols':symbols,'undefined':undefined,'soft_double_symbols':soft,'load_image_bytes':image_bytes}

sys.path.insert(0,str(TREE/'tools'))
spec=importlib.util.spec_from_file_location('fixture_build',TREE/'tools/build.py')
build=importlib.util.module_from_spec(spec);spec.loader.exec_module(build)
build.BOARD='smk37'
os.environ['JIELI_TOOLCHAIN']=str(TC)
roots=re.findall(r'^(?:uint32_t|size_t|void|int)\s+(dnbx_\w+)\s*\(', (HERE/'src/dnbx_core.h').read_text(),re.M)
flags=['-target','pi32v2','-mcpu=r3','-mfprev1','-ffp-contract=off','-ffunction-sections','-fdata-sections',
       '-fno-builtin','-fno-fast-math','-Wall','-Wno-unused-function','-std=c99',
       '-I'+str(TREE/'firmware/src/dsp'),'-idirafter',str(TC/'pi32v2/include')]
ld_flags=['-plugin',str(TC/'pi32v2/bin/LLVMgold.so'),'-plugin-opt=-inline-threshold=0',
          '-plugin-opt=-inlinehint-threshold=0','-plugin-opt=-inlinecold-threshold=0','-plugin-opt=-enable-tail-merge',
          '-plugin-opt=-tail-merge-size=1','-plugin-opt=-tail-merge-threshold=10000','-plugin-opt=-function-sections',
          '-plugin-opt=-data-sections','-plugin-opt=-fp-contract=off','--sort-section=alignment',
          '--gc-sections','-e','_start']

# Native sizeof/offsetof proof on the actual replacement headers, not a guessed
# state estimate. Existing selected FX is included unchanged.
names=['dnbx_part','juno_adapter','juno_chorus','juno_slot','backend','runtime_part','workspace','selected_fx','two_runtime_parts','two_selected_fx']
expr=['sizeof(dnbx_part)','sizeof(juno6_t)','sizeof(juno6_chorus_t)','sizeof(((original_runtime_backend*)0)->juno)',
      'sizeof(original_runtime_backend)','sizeof(original_runtime_part)','sizeof(original_runtime_workspace)',
      'sizeof(original_chicago_fx2_compact)','2*sizeof(original_runtime_part)','2*sizeof(original_chicago_fx2_compact)']
source='#include "firmware/src/app/original_runtime.h"\n#include "experiments/original_synth/chicago_fx2_compact.h"\nconst unsigned replacement_layout[]={'+','.join(expr)+'};\n'
(OUT/'layout.c').write_text(source)
run([TC/'pi32v2/bin/clang',*flags,'-Os','-I'+str(TREE),'-I'+str(TREE/'experiments/original_synth'),
     '-I'+str(TREE/'experiments/wobble'),'-I'+str(TREE/'build/gen'),'-c',OUT/'layout.c','-o',OUT/'layout.o'],'layout-compile')
run([TC/'common/bin/objcopy','-O','binary','-j','.rodata.replacement_layout',OUT/'layout.o',OUT/'layout.bin'],'layout-extract')
layout=dict(zip(names,struct.unpack('<'+'I'*len(names),(OUT/'layout.bin').read_bytes())))

links=[]
for model,boarddir in [('elite','smk37-elite'),('pro','smk37')]:
    # This recompiles the complete official closure with the edited header,
    # preventing stale offsetof assumptions in any unchanged source unit.
    p=run([sys.executable,'tools/build.py','--board','smk37','--model',model],model+'-official-full-build',False)
    if p.returncode and 'will not fit in region `XIP' not in p.stdout+p.stderr:
        raise RuntimeError('Full build did not reach the expected physical link: '+(p.stdout+p.stderr)[-2000:])
    build.MODEL=model;build.OUT=TREE/'build'/boarddir
    _,objs=build.app_compile_commands([],True,'0')
    assert all(p.is_file() for p in objs)
    physical=build.OUT/'app.ld'
    for opt in ['Os','Oz']:
        use=list(objs)
        if opt=='Oz':
            for stem in ['juno6','dnbx_core']:
                obj=OUT/(model+'-'+opt+'-'+stem+'.o')
                run([TC/'pi32v2/bin/clang',*flags,'-'+opt,'-flto','-c',TREE/'firmware/src/dsp'/(stem+'.c'),'-o',obj],model+'-'+opt+'-'+stem+'-compile')
                old=build.OUT/'original'/(stem+'.o')
                assert use.count(old)==1;use[use.index(old)]=obj
        for policy in ['reachable','all-api']:
            label=model+'-'+opt+'-'+policy
            target=OUT/(label+'.elf')
            cmd=[TC/'pi32v2/bin/ld',*ld_flags,'-T',physical,
                 *([a for root in roots for a in ['-u',root]] if policy=='all-api' else []),*use,'-o',target]
            p=run(cmd,label+'-physical-link',False)
            diagnostic=False
            if p.returncode:
                diagnostic=True
                diag=OUT/(label+'-DIAGNOSTIC-OVERFLOW.ld')
                t=physical.read_text();assert t.count('LENGTH = 0x86EE0')==1
                diag.write_text(t.replace('LENGTH = 0x86EE0','LENGTH = 0xA0000'))
                target=OUT/(label+'-DIAGNOSTIC-OVERFLOW.elf')
                dc=list(cmd);dc[dc.index('-T')+1]=diag;dc[dc.index('-o')+1]=target
                run(dc,label+'-diagnostic-link')
            detail=inspect(target,label)
            row={'model':model,'optimization':opt,'policy':policy,'physical_link_returncode':p.returncode,
                 'diagnostic_only':diagnostic,'physical_xip_capacity':552672,
                 'free_bytes':552672-detail['load_image_bytes'],'required_free_bytes':1024,
                 'xip_gate':'PASS' if 552672-detail['load_image_bytes']>=1024 else 'FAIL',
                 'retained_candidate_apis':sorted(s['name'] for s in detail['symbols'] if s['name'].startswith('dnbx_')),
                 **detail}
            links.append(row)
            (OUT/'RESULT.json').write_text(json.dumps({'provenance':json.loads((HERE/'replacement-PROVENANCE.json').read_text()),'native_layout':layout,'links':links},indent=2))
            print(json.dumps({k:v for k,v in row.items() if k not in ['sections','symbols','retained_candidate_apis']}),flush=True)
print(json.dumps({'native_layout':layout,'links':len(links)},indent=2))
