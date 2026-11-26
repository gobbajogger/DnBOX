"""Remove the dead-path 909 comparison retained in the shared kernel by LTO."""
from pathlib import Path
import sys,hashlib
p=Path(sys.argv[1]);raw=p.read_bytes()
assert hashlib.sha256(raw).hexdigest()=='cacc7827e14ac67872c05b8e59580c274cf9270f855f14ac19ad6c3d1c20bdba'
s=raw.decode()
a=s.index('extern void plat_cpu2_sync(void);')
b=s.index('static void d9_render(',a)
s=s[:a]+s[b:]
s=s.replace('uint16_t audible_tracks,\n                      unsigned wanted, d9_future_t *volatile *pending)','uint16_t audible_tracks)')
a=s.index('            d9_future_t *future=0;')
b=s.index('\n        }\n    }\n}',a)
s=s[:a]+'            d9_render_smp(s, &bus, n);'+s[b:]
s=s.replace('0x7FFu,0,0','0x7FFu').replace('int n, uint16_t audible_tracks, int cleared, unsigned v, d9_future_t *volatile *pending)','int n, uint16_t audible_tracks, int cleared)')
s=s.replace('n, audible_tracks,v,pending','n, audible_tracks').replace('n, audible_tracks,1,0,0','n, audible_tracks,1').replace('n, audible_tracks,0,0,0','n, audible_tracks,0')
a=s.index('void drum909_render_st_future(')
b=s.index('void drum909_render_st_masked(',a)
s=s[:a]+s[b:]
assert 'd9_verify_' not in s and 'd9_future_t' not in s
p.write_text(s)
