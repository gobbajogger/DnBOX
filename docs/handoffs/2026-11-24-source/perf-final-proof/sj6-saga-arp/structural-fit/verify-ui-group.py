"""Prove the linked normal-panel strings and geometry in the disposable seal."""
import ast,re,sys
from pathlib import Path
checkout,out=map(Path,sys.argv[1:])
source=(checkout/'firmware/src/app/ui.c').read_text()
header=(checkout/'firmware/src/app/ui_lit_companion_lab.h').read_text()
strings={int(i):ast.literal_eval(lit).encode('ascii')+b'\0' for i,lit in re.findall(
    r'^static const char LU_(\d+)\[\].* = ("[^"\n]+");$',header,re.M)}
corner=bytes(int(a,16) for a in re.findall(r'0x([0-9a-f]{2})',source.split('static const uint8_t CORNER_PACKED[204] = {')[1].split('};')[0]))
assert len(corner)==204
for model in ('elite','pro'):
    w=out/model; m=(w/'lab.map').read_text()
    body=(w/'lab_cold.bin').read_bytes()
    region=m[m.index('\n.lab_cold '):m.index('\n.lab_fit ')]
    a=int(re.search(r'^\.lab_cold\s+0x([a-f0-9]+)',region,re.M)[1],16)
    consumed=0; live=[]
    for i,x,z in re.findall(r'^ \.lab_ui_copy\.c(\d+)\s+0x([a-f0-9]+)\s+0x([a-f0-9]+)',region,re.M):
        at=int(x,16)-a; size=int(z,16)
        assert at>=32 and at+size<=len(body)
        assert body[at:at+size]==strings[int(i)]
        consumed+=size;live.append(int(i))
    assert live and len(live)==len(set(live))
    x,z=re.search(r'^ \.rodata.CORNER_PACKED\s+0x([a-f0-9]+)\s+0x([a-f0-9]+)',region,re.M).groups()
    assert int(z,16)==204 and body[int(x,16)-a:int(x,16)-a+204]==corner
    refs=[int(z,16) for z in re.findall(r'^ \.rodata.describe_pages\.refs(?:\.\d+)?\s+0x[0-9a-f]+\s+0x([a-f0-9]+)',region,re.M)]
    print(model,len(live),'exact linked strings',consumed,'bytes, exact packed corner',len(corner),
          'page descriptors',len(refs),'bytes',sum(refs),'combined group',consumed+len(corner)+sum(refs))
