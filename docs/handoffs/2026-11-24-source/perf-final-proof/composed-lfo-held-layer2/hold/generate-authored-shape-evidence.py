from hashlib import sha256
from pathlib import Path
from collections import Counter
import struct,sys
src,archive,output=map(Path,sys.argv[1:])
assert sha256(archive.read_bytes()).hexdigest()=="ab702e00b4cc425c9406b9b2b44e68ba0f40e1125d591fcaa422cc6a8c6272b8"
sys.path.insert(0,str(src/'experiments/sound-hardware-v1/sj6-factory'))
from factory import load_factory
from compile_bank import compile_bank
words=[];vibrato=[]
for _,(_,bank) in sorted(load_factory(archive).items()):
 for row in sorted(bank.values(),key=lambda r:r['position']):
  hits=[x['value'] for x in row['parameterValues'] if x['identifier']=='neonlfo2Waveform']; assert len(hits)==1
  words.append(struct.unpack('<I',struct.pack('<f',hits[0]))[0])
  first={}
  for item in row['parameterValues']:first.setdefault(item['identifier'],item['value'])
  vibrato.append(tuple(struct.unpack('<I',struct.pack('<f',first[k]))[0] for k in ('neonvibratoSemitones','neonvibratoRate')))
assert len(words)==640 and set(words)=={0,0x3e800000,0x3f000000,0x3f400000,0x3f800000}
(output/'authored_shape.inc').write_text('static const uint32_t authored_shape[]={'+','.join(f'0x{x:08x}u' for x in words)+'};\n')
(output/'authored_vibrato.inc').write_text('static const uint32_t authored_words[][2] = {\n'+''.join(f'    {{0x{x:08x}u, 0x{y:08x}u}},\n' for x,y in vibrato)+'};\n')
print('ID98 raw words',Counter(words))
compile_bank(archive,output/'bank_data.inc')
