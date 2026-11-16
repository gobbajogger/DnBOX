"""Evidence-only wrapper: original build.py, with only ld -Map added."""
import importlib.util,sys,hashlib,json,os
from pathlib import Path
root=Path.cwd();model=os.environ['MODEL'];out=root/'build'/('smk37-elite' if model=='elite' else 'smk37');elf=out/'x0x.elf'
before=hashlib.sha256(elf.read_bytes()).hexdigest()
spec=importlib.util.spec_from_file_location('native_build',root/'tools/build.py');build=importlib.util.module_from_spec(spec);spec.loader.exec_module(build);original=build.tc;links=[]
def tc(*args,**kwargs):
    if args[0]=='pi32v2/bin/ld' and '-o' in args and str(args[args.index('-o')+1]).endswith('/x0x.elf'):
        assert Path(args[args.index('-o')+1]).resolve()==elf.resolve(), 'unexpected board/output; refuse link'
        map_path=out/'x0x.map';args=(*args,'-Map',map_path);links.append([str(x) for x in args])
    return original(*args,**kwargs)
build.tc=tc;sys.argv=['build.py','--board','smk37','--model',model];result=build.main()
assert result in (None,0), 'original build failed; map receipt not qualified'
assert build.BOARD=='smk37' and build.MODEL==model and build.OUT.resolve()==out.resolve(), 'unexpected target'
after=hashlib.sha256(elf.read_bytes()).hexdigest();assert len(links)==1,links
report={'model':model,'wrapperOnlyAddsLinkMap':True,'elfBeforeSHA256':before,'elfAfterSHA256':after,'elfByteIdentical':before==after,'mapSHA256':hashlib.sha256((out/'x0x.map').read_bytes()).hexdigest(),'linkCommand':links[0]}
(out/'map-receipt.json').write_text(json.dumps(report,indent=2)+'\n');print('MAP_RECEIPT',json.dumps(report))
assert before==after,'map-only relink changed ELF; do not reuse earlier binary evidence'
