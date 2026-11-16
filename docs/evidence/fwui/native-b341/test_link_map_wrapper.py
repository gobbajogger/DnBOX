"""Check the evidence wrapper cannot label a different target's link as SMK."""
import ast,sys
from pathlib import Path
source=Path(sys.argv[1]).read_text();module=ast.parse(source);fn=next(x for x in module.body if isinstance(x,ast.FunctionDef) and x.name=='tc')
calls=[];out=Path('/tmp/expected/build/smk37-elite');elf=out/'x0x.elf'
env={'Path':Path,'out':out,'elf':elf,'links':[],'original':lambda *args,**kwargs:calls.append(args)}
exec(compile(ast.Module(body=[fn],type_ignores=[]),str(sys.argv[1]),'exec'),env)
try:env['tc']('pi32v2/bin/ld','-o',Path('/tmp/expected/build/x0x.elf'))
except AssertionError:pass
else:raise AssertionError('wrong target was not rejected')
assert calls==[],calls
args=('pi32v2/bin/ld','--gc-sections','-o',elf);env['tc'](*args)
assert calls[-1][:-2]==args and calls[-1][-2:]==('-Map',out/'x0x.map')
args=('pi32v2/bin/ld','-o',out/'loader.elf');env['tc'](*args);assert calls[-1]==args
print('PASS3: wrong-target rejected before execution; intended link changes only map flag; loader untouched')
