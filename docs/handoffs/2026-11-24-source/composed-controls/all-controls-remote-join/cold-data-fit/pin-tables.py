from pathlib import Path
import json,sys
src,receipts,out=map(Path,sys.argv[1:])
s=src.read_text()
for zone, dict_name in (("slot3","cold_factory"),("fittab","exact_fittab")):
    anchor="    "+dict_name+"={\n"
    assert s.count(anchor)==1
    rows="".join("        "+repr(name)+": ("+str(n)+","+repr(digest)+"),\n"
                 for dest,name,n,digest in json.loads(receipts.read_text()) if zone==dest)
    s=s.replace(anchor,anchor+rows)
out.write_text(s)
