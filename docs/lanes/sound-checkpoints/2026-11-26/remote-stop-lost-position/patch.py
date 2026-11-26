from pathlib import Path
import hashlib,sys
a,b=map(Path,sys.argv[1:]);raw=a.read_bytes()
assert hashlib.sha1(f'blob {len(raw)}\0'.encode()+raw).hexdigest()=='24cf61a06cd2306f548fc7b04e73438bf380582c'
s=raw.decode()
key='''function fwProtocol(event){
 if(event.kind===NOTIFY_KIND.POSITION){'''
assert s.count(key)==1
s=s.replace(key,'''function fwProtocol(event){
 // STOP is independently authoritative even if its separate cleared
 // POSITION notification was lost. Never infer a column from TRANSPORT.
 if(event.kind===NOTIFY_KIND.TRANSPORT&&event.fields.playing===0){
  fwPositions=null;fwLastStep=null;fwClearColumns();
  $('play')?.classList.remove('beat');
 }
 if(event.kind===NOTIFY_KIND.POSITION){''')
b.write_text(s)
