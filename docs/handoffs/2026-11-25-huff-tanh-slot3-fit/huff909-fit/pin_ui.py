from pathlib import Path
import hashlib,re,struct
base=Path('tree')
dirs=base/'firmware/src/app'
keydict={'DV_XY_PACKED':(dirs/'ui_drum_voice.inc',1385,'B'),
         'DV_POLY_PACKED':(dirs/'ui_drum_voice.inc',243,'B'),
         'UI_PICTOGRAM_LOW':(dirs/'ui_pictograms.h',333,'B'),
         'UI_PICTOGRAM_HIGH':(dirs/'ui_pictograms.h',42,'B'),
         'brk_retrigger_groups':(base/'firmware/src/dsp/break_retrigger_packed.h',975,'B'),
         'original_chicago_eq_sin':(base/'experiments/original_synth/chicago_dynamics_private.h',512,'f'),
         'original_chicago_eq_cos':(base/'experiments/original_synth/chicago_dynamics_private.h',512,'f')}
for board in ('smk37','smk37-elite'):
    sources=keydict
    result={}
    for tag,(source,size,fmt) in sources.items():
        t=source.read_text()
        if tag=='arc8_turns':
            raw=re.findall(r'^\s+PIXEL\([^,]+,[^,]+,[^,]+,\s*(0x[0-9a-f]+)u\)',t,re.M)
        else:
            body=re.search(r'\b'+tag+r'[^=\n]*=\s*\{(.*?)\};',t,re.S)[1]
            include=re.fullmatch(r'\s*#include\s+"([^"]+)"\s*',body)
            if include:
                body=(source.parent/include[1]).read_text()
            body=re.sub(r'/\*.*?\*/|//[^\n]*','',body,flags=re.S)
            raw=re.findall(r'0x[0-9a-fA-F]+|\b\d+\b',body)
        if fmt=='f':
            raw=[v.strip().removesuffix('f') for v in body.split(',') if v.strip()]
        packed=b''.join(struct.pack('<'+fmt,float.fromhex(x) if fmt=='f' else int(x,0)) for x in raw)
        assert len(packed)==size,(tag,len(packed),size)
        result[tag]=(size,hashlib.sha256(packed).hexdigest())
    if board=='smk37':
        pro=result
    else:
        assert result==pro
for k,v in pro.items():
    print(f"        '.rodata.{k}': {v},")
print('total exact bytes',sum(n for n,_ in pro.values()))
