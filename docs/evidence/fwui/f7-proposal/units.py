import re,json,hashlib
from pathlib import Path
p=Path('synthone-implementation/exact-dependencies-56079/master/AudioKitSynthOne/DSP/Kernel/S1DSPKernel.hpp'); b=p.read_bytes();rows=[]
for line,text in enumerate(b.decode().splitlines(),1):
 m=re.search(r'\{\s*(\w+),\s*([^,]+),\s*([^,]+),\s*([^,]+),\s*"[^"]+"\s*,\s*"([^"]+)"\s*,\s*kAudioUnitParameterUnit_(\w+),\s*(true|false)',text)
 if m: rows.append(dict(position=len(rows),identifier=m[1],minimumExpression=m[2].strip(),defaultExpression=m[3].strip(),maximumExpression=m[4].strip(),sourceLabel=m[5],audioUnitUnit=m[6],usePortamento=m[7]=='true',line=line))
assert len(rows)==150,len(rows)
out={'status':'SOURCE_METADATA_NOT_DISPLAY_FORMATTER_QUALIFICATION','sourcePath':str(p),'sourceSHA256':hashlib.sha256(b).hexdigest(),'warnings':['Source row136 repeats compressorMasterMakeupGain identifier; positional metadata must not silently alias ID134.','Source delayInputCutoffTrackingRatio unit Hertz conflicts with ratio semantics; do not automatically turn source AU metadata into final displayed unit.','Dependent tempo-synced rate values require state-aware formatting.'],'parameters':rows}
Path('fwui-evidence/f7-proposal/synthone-native-units.json').write_text(json.dumps(out,indent=2)+'\n')
from collections import Counter
print(Counter(x['audioUnitUnit'] for x in rows))
