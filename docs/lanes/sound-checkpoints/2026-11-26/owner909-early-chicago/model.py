"""Scoped two-core phase graph, not DSP, wall timing or PCM qualification."""
from functools import cache
# Assume one admitted whole kit; actual status/readiness is checked by engine.c.
# 808-C1 prefix has the old output-window owner and precedes the 909 on C1.
# The C0 808 metal paths may use A workspace only after preparation is complete.
deps={
 'P8':set(),'K9':{'P8'},'AP':set(),'BP':{'AP'},
 '8P':{'P8','BP'},'8C':{'8P','K9'},'AC':{'AP','8C'},'BC':{'BP','AC'},
 'BRK':{'BC'},'TAIL':{'BRK','K9'}}
@cache
def orders(done=frozenset()):
 if len(done)==len(deps):return 1
 return sum(orders(done|{k}) for k,d in deps.items() if k not in done and d<=done)
def topo(done,out):
 if len(done)==len(deps):
  assert [x for x in out if x in ['K9','8C','AC','BC','BRK','TAIL']]==['K9','8C','AC','BC','BRK','TAIL']
  assert out.index('AP')<out.index('BP')<out.index('8P')
  return
 for k,d in deps.items():
  if k not in done and d<=done:topo(done|{k},out+[k])
topo(set(),[])
# Source stage wait cannot start before both early jobs; optional C1 rejection explicitly
# leaves eng_c1_mask=0 and early_bass_ready=0 so no local job is omitted.
assert orders()>1
print(f'{orders()} legal ownership interleavings preserve ordered sum and A→B callback order (model only)')
