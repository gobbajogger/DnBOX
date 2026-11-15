"""Serialized read-only native-host presentation. No raw-memory or device API."""
from pathlib import Path
import subprocess,json,threading,hashlib
from native_html_core import decode_fonts,render_native,render_smooth,outline_fonts,raw_bytes,PIN
BASE=Path(__file__).resolve().parent;ROOT=BASE/'source-restoration/native-3c41'
class NativeSession:
 def __init__(self):
  self.lock=threading.Lock();self.closed=False;self.fonts=decode_fonts(ROOT/'build/gen/felucca_font.h');self.outlines=outline_fonts(ROOT,self.fonts)
  self.p=subprocess.Popen([str(BASE/'native-build/native-server')],stdin=subprocess.PIPE,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,bufsize=1)
 def event(self,command,*args,include_oracle=False):
  shapes={'snapshot':(), 'release':(), 'fixture':((0,19),), 'tick':((0,500),),'key':((0,26),(0,1)), 'physical':((0,36),(0,127)), 'pad':((0,15),(0,1)),'button':((0,13),(0,1)),'encoder':((0,6),(-8,8))}
  if command not in shapes or len(args)!=len(shapes[command]) or any(type(a) is not int or not low<=a<=high for a,(low,high) in zip(args,shapes.get(command,()))):raise ValueError('unsupported input')
  with self.lock:
   if self.closed:raise RuntimeError("native session is closed")
   self.p.stdin.write(' '.join(map(str,(command,*args)))+'\n');self.p.stdin.flush();line=self.p.stdout.readline()
   if not line:raise RuntimeError('native host stopped: '+self.p.stderr.read()[-400:])
   frame=json.loads(line)
  if 'error' in frame:raise ValueError(frame['error'])
  expected=bytes.fromhex(frame.pop('native_rgb565'));pixels,native=render_native(frame,self.fonts);actual=raw_bytes(pixels)
  if actual!=expected:
   first=next(i for i,(a,b) in enumerate(zip(actual,expected)) if a!=b);raise RuntimeError('native replay mismatch at byte '+str(first))
  frame['proof']={'native_bytes_equal':True,'bytes_compared':len(expected),'native_sha256':hashlib.sha256(expected).hexdigest(),'scope':'fresh C framebuffer versus software RGB565 primitive replay, not browser screenshot pixels'}
  frame['native_html']=native;frame['smooth_html']=render_smooth(frame,self.fonts,self.outlines)
  frame['text']=' '.join(c[5] for s in frame['sections'] for c in s['commands'] if c[0]==3)
  if include_oracle:frame['native_rgb565']=expected.hex()
  return frame
 def close(self):
  with self.lock:
   if self.closed:return
   self.closed=True
   if self.p.poll() is None:
    try:self.p.stdin.write('release\n');self.p.stdin.flush();self.p.stdout.readline()
    except (BrokenPipeError,ValueError):pass
    try:self.p.stdin.close()
    except (BrokenPipeError,OSError,ValueError):pass
    try:self.p.wait(timeout=3)
    except subprocess.TimeoutExpired:self.p.terminate();self.p.wait(timeout=3)
   for stream in (self.p.stdin,self.p.stdout,self.p.stderr):
    try:stream.close()
    except (BrokenPipeError,OSError,ValueError):pass
