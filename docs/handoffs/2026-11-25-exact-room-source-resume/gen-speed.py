from pathlib import Path
import hashlib,re,struct,sys
d=Path(sys.argv[1])/'experiments/original_synth'
p=d/'modulation_private.h'
vals=[int(v) for v in re.search(r'mod_speed\[128\]\s*=\s*\{([^}]+)',p.read_text())[1].split(',')]
assert len(vals)==128
expected='2d128d118a72a401d6bc539cf46e3d4d7bd363cdb0b9161f32d4800c792a957c'
digest=hashlib.sha256(struct.pack('<128I',*vals)).hexdigest()
assert digest==expected,digest
head=struct.pack('<39H',*vals[:39])
middle=b''.join(v.to_bytes(3,'little') for v in vals[39:90])
predict=lambda i:2705732+42756*(i*(i-1)//2)+64134*i+((86*(i*(i-1)//2)+156*i)>>8)
tail=bytearray(5)
for i,n in enumerate(vals[90:]):
    b=predict(i)-n
    assert b in [0,1]
    tail[i>>3]|=b<<(i&7)
def get(i):
 if i<39:return int.from_bytes(head[2*i:2*i+2],'little')
 if i<90:return int.from_bytes(middle[(i-39)*3:(i-39)*3+3],'little')
 j=i-90
 return predict(j)-((tail[j>>3]>>(j&7))&1)
assert [get(i) for i in range(128)]==vals
def row(name,data): return f'static const uint8_t {name}[{len(data)}]={{'+','.join(map(str,data))+'};\n'
(d/'chicago_mod_speed_compact_generated.h').write_text('/* Local verified generated data; never commit. */\n'+row('mod_speed_lo',head)+row('mod_speed_mid',middle)+row('mod_speed_tail',tail))
print('Chicago mod speed 128/128 exact '+digest+'; '+str(len(head)+len(middle)+len(tail))+' bytes')
