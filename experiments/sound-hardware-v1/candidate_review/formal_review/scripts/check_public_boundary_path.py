#!/usr/bin/env python3
"""Observe the old/new library path selection without loading any native code."""
import argparse,hashlib,importlib.util,json,pathlib,sys
sys.dont_write_bytecode=True
class PathObserved(Exception):pass

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--package',type=pathlib.Path,required=True)
    p.add_argument('--output',type=pathlib.Path,required=True)
    args=p.parse_args();package=args.package.expanduser().resolve();out=args.output.expanduser().resolve()
    out.mkdir();sys.path.insert(0,str(package/'candidate_review'));cases=[]
    for label,rel in [('historical','candidate_review/history/check_compact_public_boundaries.py'),('current','candidate_review/check_compact_public_boundaries.py')]:
        src=package/rel;spec=importlib.util.spec_from_file_location('boundary_'+label,src)
        m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
        root=out/label;root.mkdir();fresh=root/'fresh';fresh.mkdir();retained=root/'compact_profile_review';retained.mkdir()
        for name in ('accepted.so','diagnostic.so','compact.so'):(fresh/name).write_bytes(b'')
        observed=[]
        def probe(path):observed.append(str(path));raise PathObserved()
        m.bind=probe;m.ROOT=root;m.OUT=retained
        sys.argv=[str(src),'--review-dir',str(fresh)]
        try:m.main()
        except PathObserved:pass
        expected=retained/'accepted.so' if label=='historical' else fresh/'accepted.so'
        cases.append({'case':label,'source':rel,'source_sha256':hashlib.sha256(src.read_bytes()).hexdigest(),
            'requested_fresh_directory':str(fresh),'first_selected_library':observed,
            'selected_fresh':observed==[str(fresh/'accepted.so')],'pass':observed==[str(expected)]})
    result={'purpose':__doc__,'native_code_loaded':False,'cases':cases,'all_pass':all(x['pass'] for x in cases)}
    (out/'RESULT.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
    if not result['all_pass']:raise SystemExit(1)
if __name__=='__main__':main()
