from pathlib import Path
import hashlib,re,struct,sys
dst=Path(sys.argv[1])/'experiments/original_synth'
text=(dst/'motion_tables_private.h').read_text()
values=re.search(r'original_miami_cap\[129\]\s*=\s*\{([^}]+)\}',text)[1]
tokens=[w.strip() for w in values.split(',') if w.strip()]
words=[struct.pack('<f',float.fromhex(x.rstrip('f'))) for x in tokens]
assert len(words)==129
assert hashlib.sha256(b''.join(words)).hexdigest()=='30877e172ddc7d9f3a92e902178322d38fbccae746cbb94c9fe8a280146c0b14'
compact=words[88:110]
assert [compact[min(max(i-88,0),21)] for i in range(129)]==words
assert hashlib.sha256(b''.join(compact)).hexdigest()=='91184fe7c2b383f2cc1b84356679c9360f38ca7171479039b62635b71286c060'
(dst/'miami_cap_compact_generated.h').write_text('/* Local generated exact cap; do not commit. */\nconst float original_miami_cap_compact[22]={'+','.join(tokens[88:110])+'};\n')
print('Miami stock cap: 129/129 exact binary32; full 30877e17… compact 91184fe7…')
