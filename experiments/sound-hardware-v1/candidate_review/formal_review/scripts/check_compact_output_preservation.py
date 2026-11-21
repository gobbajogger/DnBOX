#!/usr/bin/env python3
"""Reproduce the old compact collision, then check the current no-overwrite guard.

Imports test-runner Python only. Both calls stop before any compiler or DSP work.
"""
import argparse,hashlib,importlib.util,json,pathlib,sys
sys.dont_write_bytecode=True

def digest(p):return hashlib.sha256(p.read_bytes()).hexdigest()

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--package',required=True,type=pathlib.Path)
    parser.add_argument('--output',required=True,type=pathlib.Path)
    args=parser.parse_args();package=args.package.expanduser().resolve();out=args.output.expanduser().resolve()
    out.mkdir();sys.path.insert(0,str(package/'candidate_review'))
    results=[]
    for label,rel in [('historical','candidate_review/history/verify_compact_profile.py'),('current','candidate_review/verify_compact_profile.py')]:
        source=package/rel
        spec=importlib.util.spec_from_file_location('compact_'+label,source)
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        root=out/label;root.mkdir();retained=root/'retained';retained.mkdir()
        marker=retained/'marker';marker.write_text('keep\n')
        declaration=root/'COMPACT-ACCEPTANCE-DECLARATION.json'
        declaration.write_text('{"preserve":"original"}\n')
        before=digest(declaration)
        module.HERE=root;module.BASE=package;module.OUT=retained
        sys.argv=[str(source),'--label','retained']
        outcome=None
        try:module.main()
        except FileExistsError:outcome='FileExistsError'
        except SystemExit as exc:outcome='SystemExit:'+str(exc.code)
        after=digest(declaration)
        ok=(before!=after and outcome=='FileExistsError') if label=='historical' else (before==after and outcome=='SystemExit:2')
        ok &= marker.read_text()=='keep\n' and len(list(retained.iterdir()))==1
        results.append({'case':label,'source':rel,'source_sha256':digest(source),
            'outcome':outcome,'declaration_before_sha256':before,'declaration_after_sha256':after,
            'shared_declaration_changed':before!=after,'retained_directory_unchanged':True,'pass':ok})
    result={'purpose':__doc__,'cases':results,'all_pass':all(x['pass'] for x in results)}
    (out/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
    if not result['all_pass']:raise SystemExit(1)

if __name__=='__main__':main()
