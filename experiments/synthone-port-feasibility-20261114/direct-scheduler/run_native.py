#!/usr/bin/env python3
"""Compile the exact freestanding runtime closure, not firmware or a fit claim."""
import argparse
from datetime import datetime, timezone
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess
import tempfile

COMPILER_SHA256 = 'c45189ee624de0430deb0db02e6dce171fb5538e7fe323e3964dc8ac6236240f'
TOOLCHAIN_INPUTS_SHA256 = '56cfa3d45003c12b33bad6d4e8aff1f4dab0f33d1774faa5419f18ab98a6174e'
LOCAL = ['scheduler.hpp', 'scheduler.cpp', 'fixed_array.hpp','reader/direct_reader.h','reader/direct_reader.c','reader/morph_compute.h','reader/morph_compute.c','reader/runtime_note.hpp','reader/runtime_note.cpp','reader/generate.py','reader/DERIVATION.json']
LAYOUT = '''#include "scheduler.hpp"
struct LayoutAccess : s1scheduler::RuntimeState {
    static constexpr unsigned notes() { return sizeof(((LayoutAccess*)0)->objects_); }
    static constexpr unsigned descriptors() { return sizeof(((LayoutAccess*)0)->descriptors_); }
};
extern "C" __attribute__((used,section(".s1layout"))) const unsigned s1_layout[] = {
  0x53314c31,2,sizeof(void*),sizeof(float),sizeof(double),
  sizeof(s1scheduler::State),sizeof(s1scheduler::RuntimeState),sizeof(s1scheduler::Scheduler),
  sizeof(synthone_ports::Ports),sizeof(S1NoteState),sizeof(s1scheduler::Held),
  LayoutAccess::notes(),LayoutAccess::descriptors(),alignof(s1scheduler::Scheduler),sizeof(s1_morph_binding),alignof(s1_morph_binding)
};
'''


def require(value, message):
    if not value:
        raise SystemExit(message)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def elf_sections(path):
    data=path.read_bytes()
    require(data[:6]==b'\x7fELF\x01\x01', 'expected native ELF32 little-endian object')
    header=struct.unpack_from('<16sHHIIIIIHHHHHH',data)
    offset,entry_size,count,names_index=header[6],header[11],header[12],header[13]
    require(entry_size==40 and offset+count*40<=len(data) and names_index<count, 'invalid ELF section table')
    rows=[struct.unpack_from('<IIIIIIIIII',data,offset+i*40) for i in range(count)]
    names=rows[names_index];strings=data[names[4]:names[4]+names[5]]
    result={}
    for row in rows:
        name=strings[row[0]:].split(b'\0',1)[0].decode('ascii')
        if row[1]!=8:
            require(row[4]+row[5]<=len(data), 'ELF section exceeds object')
        result[name]={'type':row[1],'flags':row[2],'bytes':row[5],
                      'data':data[row[4]:row[4]+row[5]] if row[1]!=8 else b''}
    return header[2],result


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--experiment-root',type=Path,required=True)
    p.add_argument('--controls-root',type=Path,required=True)
    p.add_argument('--toolchain-root',type=Path,required=True)
    p.add_argument('--build-root',type=Path)
    a=p.parse_args();started_at=datetime.now(timezone.utc).isoformat();root=Path(__file__).resolve().parent
    bases={'experiment':a.experiment_root.resolve(),'controls':a.controls_root.resolve()}
    tc=a.toolchain_root.resolve();compiler=tc/'common/bin/clang'
    metadata_bytes=(root/'NATIVE-TOOLCHAIN.json').read_bytes()
    require(digest(metadata_bytes)==TOOLCHAIN_INPUTS_SHA256, 'native toolchain manifest mismatch')
    toolchain_inputs=json.loads(metadata_bytes)
    require(digest(compiler.read_bytes())==COMPILER_SHA256 and compiler.stat().st_size==toolchain_inputs['compiler']['bytes'], 'native compiler identity mismatch')
    pinned_headers={}
    for entry in toolchain_inputs['headers']:
        header=(tc/entry['path']).resolve();data=header.read_bytes()
        require(len(data)==entry['bytes'] and digest(data)==entry['sha256'], 'native toolchain header mismatch: '+entry['path'])
        pinned_headers[header]=entry['sha256']
    manifest=json.loads((root/'DEPENDENCIES.json').read_text());verified={};source_records=[]
    for group,base in bases.items():
        verified[group]={}
        for e in manifest['groups'][group]:
            data=(base/e['path']).read_bytes()
            require(len(data)==e['bytes'] and digest(data)==e['sha256'], 'native dependency identity mismatch: '+group+'/'+e['path'])
            verified[group][e['path']]=data
    local={name:(root/name).read_bytes() for name in LOCAL}
    build=a.build_root.resolve() if a.build_root else root/'build';build.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='pi32v2.',dir=build));closure=out/'closure';expected={}
    for group,entries in dict(verified,scheduler=local).items():
        for name,data in entries.items():
            dest=closure/group/name;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(data)
            expected[dest.resolve()]=digest(data)
            source_records.append({'path':group+'/'+name,'bytes':len(data),'sha256':digest(data)})
    core=closure/'experiment/audiokit-4.9.2/AudioKit/Core'
    includes=[closure/'scheduler',closure/'experiment/probe',closure/'experiment/shared-graph',closure/'controls',core/'Soundpipe',core/'SoundpipeExtension',core/'Soundpipe/lib/dr_wav']
    resource=tc/'common/lib/clang/4.0.1/include';target_include=tc/'pi32v2/include'
    flags=['-target','pi32v2','-mcpu=r3','-mfprev1','-Os','-ffunction-sections','-fdata-sections','-ffp-contract=off','-fno-fast-math','-fno-builtin','-DNO_LIBSNDFILE','-nostdinc']
    inc=['-I'+str(x) for x in includes]+['-isystem',str(resource),'-isystem',str(target_include)]
    # Compiler-driver controls such as CCC_OVERRIDE_OPTIONS can override argv.
    # Inherit nothing: no user config, loader injection or numeric-mode controls.
    temporary=out/'compiler-temp';temporary.mkdir()
    env={'PATH':os.defpath,'LC_ALL':'C','LANG':'C','TMPDIR':str(temporary)}
    derived=out/'reader-derived'
    subprocess.run([__import__('sys').executable,str(closure/'scheduler/reader/generate.py'),'--experiment-root',str(closure/'experiment'),'--output-dir',str(derived)],check=True,env=env,stdout=subprocess.DEVNULL)
    for name in ['runtime_note.cpp','morph_compute.c','DERIVATION.json']:
        require((derived/name).read_bytes()==(closure/'scheduler/reader'/name).read_bytes(),'native reader derivation mismatch: '+name)
    units=[('direct_reader',closure/'scheduler/reader/direct_reader.c','c'),('runtime_morph',closure/'scheduler/reader/morph_compute.c','c'),('runtime_note',closure/'scheduler/reader/runtime_note.cpp','c++'),('scheduler',closure/'scheduler/scheduler.cpp','c++'),('ports',closure/'controls/parameter_ports.cpp','c++'),('parameter_map',closure/'controls/generated/parameter_map.cpp','c++'),('voice',closure/'experiment/probe/portable_voice.cpp','c++')]
    units += [(name,core/'Soundpipe/modules'/(name+'.c'),'c') for name in ['base','ftbl','adsr','crossfade','osc','fosc','noise','moogladder','butbp','buthp','port','phasor']]
    units += [('morph',core/'SoundpipeExtension/modules/oscmorph2d.c','c')]
    layout=out/'layout.cpp';layout.write_text(LAYOUT);expected[layout.resolve()]=digest(layout.read_bytes())
    results=[];system_headers={}
    for name,source,language in units+[('layout',layout,'c++')]:
        obj,dep=out/(name+'.o'),out/(name+'.d')
        command=[str(compiler),'-x',language,'-std=c++11' if language=='c++' else '-std=gnu99']+flags+(['-fno-exceptions','-fno-rtti'] if language=='c++' else [])+inc+['-MD','-MF',str(dep),'-c',str(source),'-o',str(obj)]
        q=subprocess.run(command,capture_output=True,text=True,env=env)
        (out/(name+'.stdout')).write_text(q.stdout);(out/(name+'.stderr')).write_text(q.stderr)
        require(q.returncode==0, 'native compile failed: '+name+'; see '+str(out/(name+'.stderr')))
        dependencies=[]
        for token in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1]):
            path=Path(token).resolve();data=path.read_bytes();sha=digest(data)
            if path in expected:
                require(sha==expected[path], 'native source changed during compilation')
                kind='staged'
            else:
                require(path in pinned_headers and sha==pinned_headers[path] and (resource in path.parents or target_include in path.parents), 'unlisted or changed native dependency: '+str(path))
                kind='toolchain-header';system_headers[str(path.relative_to(tc))]=sha
            dependencies.append({'path':str(path.relative_to(out)) if kind=='staged' else str(path.relative_to(tc)), 'kind':kind,'sha256':sha})
        machine,sections=elf_sections(obj)
        sizes={key:sum(s['bytes'] for s in sections.values() if s['flags']&2 and pred(s)) for key,pred in [('executable',lambda s:s['flags']&4),('readonly',lambda s:not s['flags']&5),('writable',lambda s:s['flags']&1 and s['type']!=8),('bss',lambda s:s['flags']&1 and s['type']==8)]}
        results.append({'name':name,'command':command,'exit':q.returncode,'source_sha256':digest(source.read_bytes()),'object_sha256':digest(obj.read_bytes()),'elf_machine':machine,'allocated_section_bytes':sizes,'dependencies':dependencies})
    _,sections=elf_sections(out/'layout.o');raw=sections['.s1layout']['data'];require(len(raw)==16*4,'native layout size mismatch')
    values=struct.unpack('<16I',raw);require(values[:2]==(0x53314c31,2),'native layout signature mismatch')
    labels=['pointer','float','double','State','RuntimeState','Scheduler','Ports','S1NoteState','Held','note_objects_7','table_descriptors_53','Scheduler_alignment','direct_binding','direct_binding_alignment']
    result={'started_at':started_at,'completed_at':datetime.now(timezone.utc).isoformat(),'scope':'Freestanding runtime native objects and compiler-evaluated layout only; no executable/firmware link, table placement, resource-floor, CPU or device acceptance','compiler_sha256':COMPILER_SHA256,'compiler_child_environment':env,'source_manifest_sha256':digest((root/'SOURCE-MANIFEST.json').read_bytes()),'compiled_sources':source_records,'toolchain_headers':system_headers,'units':results,'runtime_objects':len(units),'layout':dict(zip(labels,values[2:])),'not_run':['native executable/firmware link','combined Juno/app resources','table storage/read workload','CPU/device qualification']}
    (out/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({'runtime_objects':len(units),'layout':result['layout'],'result_directory':str(out),'link':'NOT_RUN'}))

if __name__=='__main__':main()
