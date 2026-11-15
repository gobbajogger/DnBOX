"""Fresh source-native renderer, not a recovery of the previous lost implementation.
Native replay preserves RGB565 and C glyph coverage; smooth SVG is a separate presentation.
"""
from pathlib import Path
from html import escape
import re,hashlib
PIN='3c41ef4bedf9701f549ce2752b73306e2fab1eb8'
SOURCE_HASHES={'firmware/src/app/ui.c':'6083a134562af8b774c268b858b7d5ddf132cfa1a30440e2e6f7a944b9f12846','firmware/src/gfx.c':'c561f3cf12e6e4757a2185c21bf6cdf8a2a98303c63a3e6fc2162f9917cefd68','firmware/src/ui_theme.h':'2576702222d15a535dc943184ea103b86a29c4f621824e26c215e7199c19883e','build/gen/felucca_font.h':'270ceb95465897e11dbd5a75fa8aaaeb9bc0ef64acaef5f9733c504c17b8a85a'}
def verify_source(root):
 for p,h in SOURCE_HASHES.items():
  if hashlib.sha256((Path(root)/p).read_bytes()).hexdigest()!=h:raise ValueError('source pin drift: '+p)
 return True

def decode_fonts(path):
 s=Path(path).read_text();arrays={}
 for name,body in re.findall(r'static const uint(?:8|16)_t (FONT_\w+)\[\d+\] = \{(.*?)\};',s,re.S):
  arrays[name]=[int(v,0) for v in re.findall(r'0x[0-9a-fA-F]+|\d+',body)]
 fonts={}
 for name,h,pad,first,last in re.findall(r'felucca_font_t FONT_(\w+) = \{\s*(\d+),\s*(\d+),\s*(\d+),\s*(\d+),',s):
  h,pad,first,last=map(int,(h,pad,first,last));data=arrays['FONT_'+name+'_DATA'];glyphs={}
  for i,ch in enumerate(range(first,last+1)):
   off=arrays['FONT_'+name+'_OFF'][i];adv=arrays['FONT_'+name+'_ADV'][i];bw=arrays['FONT_'+name+'_BW'][i]
   r=data[off]|data[off+1]<<8|data[off+2]<<16;off+=3
   x0,y0,w,hh=r&63,(r>>6)&63,(r>>12)&63,r>>18
   need=((w+1)//2)*hh;raw=[]
   while len(raw)<need:
    token=data[off];off+=1
    if token&128:
     distance=data[off];off+=1
     if not 0<distance<=len(raw):raise ValueError('invalid font backreference')
     for _ in range((token&127)+3):raw.append(raw[-distance])
    else:
     n=token+1;raw.extend(data[off:off+n]);off+=n
   if len(raw)!=need or x0+w>bw or y0+hh>h:raise ValueError('invalid font bounds')
   pixels=[0]*(bw*h)
   for y in range(hh):
    for x in range(w):pixels[(y+y0)*bw+x+x0]=(raw[y*((w+1)//2)+x//2]>>(0 if x&1 else 4))&15
   glyphs[chr(ch)]={'advance':adv,'width':bw,'pixels':pixels}
  fonts[name]={'height':h,'pad':pad,'first':first,'last':last,'glyphs':glyphs}
 return fonts

def glyph_char(font,ch):
 n=ord(ch)
 if 97<=n<=122 and font['last']<97:n-=32
 if not font['first']<=n<=font['last'] or 127<=n<160:n=63
 return chr(n)

FONT_ROLES={'T':('SemiBold',10),'XS':('SemiBold',12),'S':('Medium',15),'B':('Bold',15),'M':('SemiBold',23),'L':('Bold',31)}
def outline_fonts(root,fonts):
 from fontTools.ttLib import TTFont
 from fontTools.pens.svgPathPen import SVGPathPen
 from PIL import ImageFont,features
 result={}
 for role,font in fonts.items():
  weight,px=FONT_ROLES[role];path=Path(root)/('assets/fonts/BarlowSemiCondensed-'+weight+'.ttf')
  f=TTFont(path);gs=f.getGlyphSet();cmap=f.getBestCmap();units=f['head'].unitsPerEm
  pil=ImageFont.truetype(str(path),px);asc,_=pil.getmetrics();top=max(0,pil.getbbox('A8|(')[1]-1)
  substitutions={}
  if features.check('raqm') and 'GSUB' in f:
   t=f['GSUB'].table
   for record in t.FeatureList.FeatureRecord:
    if record.FeatureTag=='tnum':
     for index in record.Feature.LookupListIndex:
      lookup=t.LookupList.Lookup[index]
      if lookup.LookupType!=1:raise ValueError('unsupported original tnum substitution')
      for sub in lookup.SubTable:substitutions.update(sub.mapping)
  glyphs={}
  for ch,g in font['glyphs'].items():
   alias=ch
   if role in {'XS','S','B'}:
    if ch in '`{|}~':alias='?'
    elif ch.islower() and ch not in set('abdkmnprstxz'):alias=ch.upper()
   if g['advance']==0:glyphs[ch]={'path':'','scale':px/units,'baseline':asc-top,'glyph':None};continue
   name=cmap.get(ord(alias),cmap[63]);name=substitutions.get(name,name)
   pen=SVGPathPen(gs);gs[name].draw(pen)
   glyphs[ch]={'path':pen.getCommands(),'scale':px/units,'baseline':asc-top,'glyph':name}
  result[role]=glyphs
 return result

def colour(c):return 'rgb(%d,%d,%d)'%(((c>>11)*255//31),(((c>>5)&63)*255//63),((c&31)*255//31))
def blend(c,b,a,den):
 return (((((c>>11)*a+(b>>11)*(den-a))//den)<<11)|(((((c>>5)&63)*a+((b>>5)&63)*(den-a))//den)<<5)|(((c&31)*a+(b&31)*(den-a))//den))
def _check(frame):
 if frame.get('source')!=PIN:raise ValueError('wrong native source pin')
 for s in frame['sections']:
  if any(type(s[k]) is not int for k in ('x','y','w','h','background')):raise ValueError('noninteger geometry')
  if min(s['x'],s['y'])<0 or min(s['w'],s['h'])<=0 or s['x']+s['w']>240 or s['y']+s['h']>240:raise ValueError('section outside display')

def _runs(points):
 # Keep draw order. Merge only consecutive adjacent same-colour points on a row.
 out=[]
 for x,y,c in points:
  if out and out[-1][1]==y and out[-1][3]==c and out[-1][0]+out[-1][2]==x:out[-1][2]+=1
  else:out.append([x,y,1,c])
 return out

def _span_runs(points):
 return ''.join('<i aria-hidden="true" style="position:absolute;left:%dpx;top:%dpx;width:%dpx;height:1px;background:%s"></i>'%(x,y,w,colour(c)) for x,y,w,c in _runs(points))

def _section_replay(section,fonts):
 w,h=section['w'],section['h'];pixels=[section['background']]*(w*h);nodes=[]
 for cmd in section['commands']:
  if not cmd:raise ValueError('empty command')
  kind=cmd[0];points=[]
  def point(x,y,c,cy0,cy1,a=1,den=1,cl=0,cr=None):
   if cr is None:cr=w
   if 0<=x<w and cl<=x<cr and 0<=y<h and cy0<=y<cy1 and a:
    idx=y*w+x;v=blend(c,pixels[idx],min(a,den),den);pixels[idx]=v;points.append((x,y,v))
  if kind==0:
   _,x,y,rw,rh,c,cy0,cy1=cmd
   for yy in range(max(y,cy0,0),min(y+rh,cy1,h)):
    for xx in range(max(x,0),min(x+rw,w)):point(xx,yy,c,cy0,cy1)
  elif kind==1:
   _,x,y,x1,y1,c,cy0,cy1=cmd;dx=abs(x1-x);sx=1 if x<x1 else -1;dy=-abs(y1-y);sy=1 if y<y1 else -1;err=dx+dy
   for _ in range(2000):
    point(x,y,c,cy0,cy1)
    if x==x1 and y==y1:break
    e=2*err
    if e>=dy:err+=dy;x+=sx
    if e<=dx:err+=dx;y+=sy
   else:raise ValueError('line exceeds native guard')
  elif kind==2:
   _,x,y,c,cy0,cy1=cmd;point(x,y,c,cy0,cy1)
  elif kind==4:
   _,x,y,c,a,cy0,cy1=cmd;point(x,y,c,cy0,cy1,a,16)
  elif kind==3:
   _,x,y,c,role,text,cl,cr,cy0,cy1=cmd
   font=fonts[role]
   for ch in text:
    g=font['glyphs'][glyph_char(font,ch)];bw=g['width']
    for yy in range(font['height']):
     for xx in range(bw):point(x-font['pad']+xx,y+yy,c,cy0,cy1,g['pixels'][yy*bw+xx],15,cl,cr)
    x+=g['advance']
  else:raise ValueError('unknown draw primitive: '+str(kind))
  content=_span_runs(points)
  if kind==3:content='<span class="native-text" role="text" aria-label="'+escape(cmd[5],quote=True)+'">'+content+'</span>'
  nodes.append(content)
 return pixels,''.join(nodes)

def render_native(frame,fonts):
 _check(frame);pixels=[0]*57600;nodes=[]
 for s in frame['sections']:
  local,content=_section_replay(s,fonts)
  for yy in range(s['h']):pixels[(s['y']+yy)*240+s['x']:(s['y']+yy)*240+s['x']+s['w']]=local[yy*s['w']:(yy+1)*s['w']]
  section_background=s['background']
  nodes.append('<section data-native-section style="position:absolute;overflow:hidden;left:%dpx;top:%dpx;width:%dpx;height:%dpx;background:%s">%s</section>'%(s['x'],s['y'],s['w'],s['h'],colour(section_background),content))
 return pixels,'<div class="native-screen" role="img" aria-label="Actual source-native display" style="position:relative;width:240px;height:240px">'+''.join(nodes)+'</div>'

def render_smooth(frame,fonts,outlines):
 _check(frame);nodes=[];serial=0
 for s in frame['sections']:
  local=[s['background']]*(s['w']*s['h']);body=[];section_clip='s'+str(serial);serial+=1
  body.append('<defs><clipPath id="%s"><rect width="%d" height="%d"/></clipPath></defs>'%(section_clip,s['w'],s['h']))
  body.append('<rect width="%d" height="%d" fill="%s"/>'%(s['w'],s['h'],colour(s['background'])))
  for cmd in s['commands']:
   kind=cmd[0]
   if kind==3:
    _,x,y,c,role,text,cl,cr,cy0,cy1=cmd;clip='t'+str(serial);serial+=1
    body.append('<defs><clipPath id="%s"><rect x="%d" y="%d" width="%d" height="%d"/></clipPath></defs>'%(clip,cl,cy0,max(0,cr-cl),max(0,cy1-cy0)))
    group=[];font=fonts[role]
    for ch in text:
     alias=glyph_char(font,ch);outline=outlines[role][alias];scale=outline['scale']
     group.append('<path d="%s" transform="translate(%d %.6f) scale(%.8f %.8f)"/>'%(outline['path'],x,y+outline['baseline'],scale,-scale));x+=font['glyphs'][alias]['advance']
    body.append('<g role="text" aria-label="%s" fill="%s" clip-path="url(#%s)">%s</g>'%(escape(text,quote=True),colour(c),clip,''.join(group)))
   elif kind in (0,1,2,4):
    # Native replay supplies precise clipping/opaque undercolours for bitmap-only primitives.
    # Native line endpoints are preserved as an antialiased path in this presentation.
    if kind==1:
     _,x,y,x1,y1,c,cy0,cy1=cmd;clip='l'+str(serial);serial+=1
     body.append('<defs><clipPath id="%s"><rect y="%d" width="%d" height="%d"/></clipPath></defs>'%(clip,cy0,s['w'],max(0,cy1-cy0)))
     body.append('<path d="M%g %gL%g %g" stroke="%s" stroke-width="1" fill="none" clip-path="url(#%s)"/>'%(x+.5,y+.5,x1+.5,y1+.5,colour(c),clip))
    elif kind==0:
     _,x,y,w,h,c,cy0,cy1=cmd;xx=max(x,0);yy=max(y,cy0,0);ww=max(0,min(x+w,s['w'])-xx);hh=max(0,min(y+h,cy1,s['h'])-yy)
     body.append('<rect x="%d" y="%d" width="%d" height="%d" fill="%s"/>'%(xx,yy,ww,hh,colour(c)))
    else:
     if kind==2:_,x,y,c,cy0,cy1=cmd;a=16
     else:_,x,y,c,a,cy0,cy1=cmd
     if 0<=x<s['w'] and max(0,cy0)<=y<min(s['h'],cy1):body.append('<rect x="%d" y="%d" width="1" height="1" fill="%s" fill-opacity="%.5f"/>'%(x,y,colour(c),min(a,16)/16))
   else:raise ValueError('unknown draw primitive: '+str(kind))
  nodes.append('<g transform="translate(%d %d)" clip-path="url(#%s)">%s</g>'%(s['x'],s['y'],section_clip,''.join(body)))
 return '<svg class="smooth-screen" viewBox="0 0 240 240" role="img" aria-label="Smooth source-native presentation, not pixel-identical device output" xmlns="http://www.w3.org/2000/svg">'+''.join(nodes)+'</svg>'

def raw_bytes(pixels):return b''.join(int(v).to_bytes(2,'big') for v in pixels)
def fnv32(raw):
 v=2166136261
 for b in raw:v=((v^b)*16777619)&0xffffffff
 return '%08x'%v
