#!/usr/bin/env python3
"""Native input rejection and strict numeric child-environment isolation."""
import argparse
import hashlib
import os
import struct
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


def require(condition, message):
    if not condition:
        raise SystemExit(message)


def main():
    p=argparse.ArgumentParser()
    for name in ['experiment-root','controls-root','toolchain-root','output-root']:
        p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args();root=Path(__file__).resolve().parents[1]
    a.output_root.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='native-inputs.',dir=a.output_root)).resolve()
    metadata=json.loads((root/'NATIVE-TOOLCHAIN.json').read_text())
    def rejected(name,toolchain,controls,message):
        build=out/(name+'-build')
        command=[sys.executable,str(root/'run_native.py'),'--experiment-root',str(a.experiment_root.resolve()),'--controls-root',str(controls),'--toolchain-root',str(toolchain),'--build-root',str(build)]
        q=subprocess.run(command,capture_output=True,text=True)
        (out/(name+'.stdout')).write_text(q.stdout);(out/(name+'.stderr')).write_text(q.stderr)
        require(q.returncode!=0 and message in q.stderr and not build.exists(), name+' did not reject before build creation')
        return {'case':name,'exit':q.returncode,'rejected_before_build':True,'diagnostic':q.stderr.strip()}
    bad=out/'wrong-compiler';(bad/'common/bin').mkdir(parents=True);(bad/'common/bin/clang').write_bytes(b'not the approved compiler')
    results=[rejected('compiler_identity',bad,a.controls_root.resolve(),'native compiler identity mismatch')]
    bad=out/'changed-toolchain';(bad/'common/bin').mkdir(parents=True)
    (bad/'common/bin/clang').symlink_to(a.toolchain_root.resolve()/'common/bin/clang')
    for entry in metadata['headers']:
        dest=bad/entry['path'];dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(a.toolchain_root.resolve()/entry['path'],dest)
    (bad/metadata['headers'][0]['path']).write_bytes(b'changed native standard header')
    results.append(rejected('header_identity',bad,a.controls_root.resolve(),'native toolchain header mismatch'))
    controls=out/'changed-controls'
    for name in ['parameter_ports.hpp','parameter_ports.cpp','generated/parameter_map.hpp','generated/parameter_map.cpp']:
        dest=controls/name;dest.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(a.controls_root.resolve()/name,dest)
    (controls/'generated/parameter_map.hpp').write_bytes(b'changed structural header')
    results.append(rejected('control_identity',a.toolchain_root.resolve(),controls,'native dependency identity mismatch'))
    # Same-length independent output roots keep __FILE__ string sizes equal.
    # Compare real executable-section bytes, not recorded argv or whole-object
    # hashes containing intentionally distinct staged source paths.
    def executable_sections(path):
        data=path.read_bytes();require(data[:6]==b'\x7fELF\x01\x01','ELF32 target object required')
        h=struct.unpack_from('<16sHHIIIIIHHHHHH',data)
        rows=[struct.unpack_from('<IIIIIIIIII',data,h[6]+i*h[11]) for i in range(h[12])]
        names=rows[h[13]];strings=data[names[4]:names[4]+names[5]]
        result={}
        for row in rows:
            if row[2]&2 and row[2]&4:
                name=strings[row[0]:].split(b'\0',1)[0].decode('ascii')
                result[name]=hashlib.sha256(data[row[4]:row[4]+row[5]]).hexdigest()
        return result
    profiles=[]
    for name,override in [('profile-a',None),('profile-b','+-ffinite-math-only')]:
        command=[sys.executable,str(root/'run_native.py'),'--experiment-root',str(a.experiment_root.resolve()),'--controls-root',str(a.controls_root.resolve()),'--toolchain-root',str(a.toolchain_root.resolve()),'--build-root',str(out/name)]
        env=dict(os.environ);env.pop('CCC_OVERRIDE_OPTIONS',None)
        if override:env['CCC_OVERRIDE_OPTIONS']=override
        q=subprocess.run(command,capture_output=True,text=True,env=env)
        (out/(name+'.stdout')).write_text(q.stdout);(out/(name+'.stderr')).write_text(q.stderr)
        require(q.returncode==0,'native strict-profile build failed: '+name+' '+q.stderr)
        summary=json.loads(q.stdout);build=Path(summary['result_directory'])
        report=json.loads((build/'RESULT.json').read_text());require(report['runtime_objects']==17,'native profile object count')
        profile={unit['name']:executable_sections(build/(unit['name']+'.o')) for unit in report['units'] if unit['name']!='layout'}
        profiles.append(profile)
    require(profiles[0]==profiles[1],'ambient CCC_OVERRIDE_OPTIONS changed native executable sections')
    result={'scope':'native input rejection plus object-level strict numeric environment invariant; no target execution/fit','cases':results,'driver_override_invariant':True,'native_profile_builds':2,'runtime_objects_compared':17}
    (out/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'cases':len(results),'all_rejected_before_build':True,'driver_override_invariant':True,'result_directory':str(out)}))

if __name__=='__main__':main()
