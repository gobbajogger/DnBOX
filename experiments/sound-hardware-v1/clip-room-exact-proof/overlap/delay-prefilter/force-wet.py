from pathlib import Path
p=Path('firmware/src/dsp/s1c/s1c.c');s=p.read_text()
w=Path('experiments/sound-hardware-v1/clip-room-exact-proof/wet-fixture.patch').read_text()
add='\n'.join(x[1:] for x in w.splitlines() if x.startswith('+') and not x.startswith('+++'))
k='void s1c_render(s1c *s, float *outl, float *outr, unsigned n)\n{\n    unsigned f, i;\n'
assert s.count(k)==1
p.write_text(s.replace(k,k+add+'\n'))
