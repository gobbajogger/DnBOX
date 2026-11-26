"""Exact consumed per-phase raw word cross-check. No private samples emitted."""
from loaded import *
import ctypes as c,sys
lib=c.CDLL(sys.argv[1])
lib.witness_bank.argtypes=[c.c_void_p]
lib.sample.argtypes=[c.c_float,c.c_float,c.c_void_p];lib.sample.restype=c.c_float
votes={}
for key in (45,57,63,64,65,69,75,76,77,81,93,105):
 e=Emu(str(BIN));d=init(e,key)
 raw=extract(e);p=c.create_string_buffer(raw);lib.witness_bank(p)
 rows=[];q={}
 def pre(*_):q.update(p=e.s(10),d=e.s(9),m=e.s(12))
 def zero(*_):
    if q['m']<=0:rows.append((q['p'],q['d'],e.s(2),False))
 def blend(*_):rows.append((q['p'],q['d'],e.s(0),True))
 for addr,fn in ((0x1003b6874,pre),(0x1003b6900,zero),(0x1003b6968,blend)):
    e.uc.hook_add(UC_HOOK_CODE,fn,begin=addr,end=addr)
 d.render(128);d.render(128);assert len(rows)==1024 and not e.unknown
 got=np.array([lib.sample(p,d,None) for p,d,y,b in rows],dtype='f4')
 want=np.array([y for p,d,y,b in rows],dtype='f4')
 votes[key]=dict(words=len(rows),blended_words=sum(b for p,d,y,b in rows),
                 bit_diff=int(np.count_nonzero(got.view('u4')!=want.view('u4'))),
                 max_abs=float(np.max(abs(got-want))))
print(json.dumps(dict(bin_sha=pin(BIN),bank_sha=bank_sha,cases=votes),indent=2))
assert not any(x['bit_diff'] for x in votes.values())
