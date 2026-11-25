from pathlib import Path
import hashlib,re,struct,sys
d=Path(sys.argv[1])/'experiments/wobble'
t=(d/'miami_decimator_tables_private.h').read_text()
def parsed(name):
    v=re.search(name+r'\[128\]\s*=\s*\{([^}]+)\}',t)[1]
    return [int(x) for x in v.split(',') if x.strip()]
freq,wet=map(parsed,('miami_decimator_frequency','miami_decimator_wet'))
fsha=hashlib.sha256(struct.pack('<128H',*freq)).hexdigest()
wsha=hashlib.sha256(struct.pack('<128h',*wet)).hexdigest()
assert fsha=='fd33324fa771553c8c45e339c372526a1b160a623cf5e5e252e005ced09d472f'
assert wsha=='a7d0f57804777b16ea8d658964f84e29a9846084af4e9c145116ba9ba4daa903'
predict=lambda i:1486+((i*(2351*i-39))>>11)
bits=bytearray(16)
for i,n in enumerate(freq[:123]):
    bit=n-predict(i)
    assert bit in (0,1)
    bits[i>>3]|=bit<<(i&7)
assert [predict(i)+((bits[i>>3]>>(i&7))&1) for i in range(123)] + freq[123:] == freq
assert [i*258 if i<127 else 32767 for i in range(128)]==wet
def row(ty,name,data): return f'static const {ty} {name}[{len(data)}]={{'+','.join(map(str,data))+'};\n'
(d/'miami_decimator_compact_generated.h').write_text('/* Local verified generated data; never commit. */\n'+row('uint8_t','frequency_corrections',bits)+row('uint16_t','frequency_tail',freq[123:]))
print('Miami decimator wet 128/128 exact '+wsha+'; frequency 128/128 exact '+fsha+'; stored 26 B, arithmetic on init/set only')
