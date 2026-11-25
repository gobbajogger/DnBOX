"""Native-only runtime structure packing. Never changes wire/project bytes."""
from pathlib import Path
p=Path('firmware/src/dsp/synthone/sequencer/S1Sequencer.h')
s=p.read_text()
a=s.index('/* The S1Parameter values')
b=s.index('} S1SequencerParams;',a)
body=s[a:b]
import re
fields=[]
for name in ('arpInterval','arpTotalSteps','seqGate','patternRange','clocked'):
    pat=r'    uint8_t '+name+r';[^\n]*\n'
    m=re.findall(pat,body)
    assert len(m)==1,(name,len(m))
    fields+=m
    body=re.sub(pat,'',body,count=1)
for name in ('X0X_SJ6_SEQ_SETTINGS_NATIVE','X0X_S1_SCALE_NATIVE'):
    st=body.index('#if '+name+'\n')
    depth=0
    end=None
    cursor=st
    for line in body[st:].splitlines(keepends=True):
        tag=line.lstrip()
        if tag.startswith('#if'):depth+=1
        if tag.startswith('#endif'):
            depth-=1
            if not depth:
                end=cursor+len(line);break
        cursor+=len(line)
    assert end is not None,name
    fields.append(body[st:end]);body=body[:st]+body[end:]
p.write_text(s[:a]+body+''.join(fields)+s[b:])
