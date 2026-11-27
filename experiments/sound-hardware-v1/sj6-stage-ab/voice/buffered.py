"""Exercise real BufferedAudioUnit outer closure on two known oscillator lanes."""
from pathlib import Path
import os
p=Path('sj6-mix-boundary-recheck/wrapper.py')
s=p.read_text()
s=s.replace(" def render(n):", """ outblock, owner=e.calloc(64),e.calloc(256)
 assert e.call(0x10040be9c,owner,0)==1
 e.w64(outblock+32,owner)
 e.w32(0x1007a0f38,48);e.w32(0x1007a0f3c,8);e.w32(0x1007a0f40,16);e.w32(0x1007a0f44,40)
 e.wr(owner+48,b'\\x01');e.w64(owner+40,closure)
 e.w64(closure+16,0x1003ccecc)
 from unicorn.arm64_const import UC_ARM64_REG_X16
 e.handlers['___chkstk_darwin']=lambda:None
 e.handlers['_objc_retainBlock']=lambda:None
 e.handlers['_objc_release']=lambda:None
 e.handlers['_objc_retain']=lambda:None
 def render(n):""")
s=s.replace("e.wd(t,float(d.now));e.call(0x1003ccecc,closure,0,d.abl,t,n,0)", "e.wd(t,float(d.now));e.call(0x1003d59dc,outblock,0,t,n,0,d.abl,0,0)")
s=s.replace("e.wr(d.bufL,bytes(4*n));e.wr(d.bufR,bytes(4*n))","e.wr(d.bufL,struct.pack('<f',.81)*n);e.wr(d.bufR,struct.pack('<f',-.44)*n)")
s=s.replace("Real current AKAudioUnitBase processEvents closure","Real current outer BufferedAudioUnit closure then AKAudioUnitBase processEvents closure")
if os.environ.get("SJ6_ABL_FALLBACK"):
 s=s.replace(" def render(n):", """ backing=e.calloc(1024)
 e.w64(owner+16,backing)
 def render(n):
  e.w64(d.abl+16,0);e.w64(d.abl+32,0)
  e.wr(backing,struct.pack('<f',.38)*(2*n))""")
 s=s.replace("for addr in (d.bufL,d.bufR)", "for addr in (e.r64(d.abl+16),e.r64(d.abl+32))")
 s=s.replace("  d.now+=n", "  assert e.r64(d.abl+16)==backing and e.r64(d.abl+32)==backing+4*n\n  d.now+=n")
 s=s.replace("constructed closure storage,", "constructed closure storage, null output mData grants real fallback buffer path,")
exec(compile(s,str(p),'exec'))
