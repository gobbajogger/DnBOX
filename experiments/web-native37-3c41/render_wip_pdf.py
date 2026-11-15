"""New source-rendered WIP images/PDF. Never visits the denied browser origin."""
from pathlib import Path
import re,json,hashlib,io,xml.etree.ElementTree as ET
from PIL import Image
from reportlab.pdfgen import canvas
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib.utils import ImageReader
from reportlab.lib.pagesizes import landscape,A4
from reportlab.graphics import renderPDF
from reportlab.graphics.shapes import Drawing
from reportlab.lib.colors import Color
from fontTools.pens.reportLabPen import ReportLabPen
from fontTools.svgLib.path import parse_path
from native_session import NativeSession
BASE=Path(__file__).resolve().parent;OUT=BASE.parent/'output/pdf';ROOT=BASE/'source-restoration/native-3c41'
pdfmetrics.registerFont(TTFont('Barlow',str(ROOT/'assets/fonts/BarlowSemiCondensed-Medium.ttf')))
pdfmetrics.registerFont(TTFont('BarlowBold',str(ROOT/'assets/fonts/BarlowSemiCondensed-Bold.ttf')))
W,H=landscape(A4);c=canvas.Canvas(str(OUT/'DnBOX-WIP-real-37-key-adapter.pdf'),pagesize=(W,H));c.setTitle('DnBOX NEW website WIP - current-source native/smooth and physical controls')
proof=[]
def colour(s):
 a=list(map(float,re.findall(r'[\d.]+',s)));return tuple(v/255 for v in a[:3])
def svg(svg_text,x,y,size):
 root=ET.fromstring(svg_text);clips={}
 for e in root.iter():
  if e.tag.rsplit('}',1)[-1]=='clipPath':clips[e.attrib['id']]=list(e)
 def walk(e,fill='rgb(0,0,0)',stroke='none'):
  tag=e.tag.rsplit('}',1)[-1]
  if tag in ('defs','clipPath'):return
  c.saveState()
  for name,args in re.findall(r'(translate|scale)\(([^)]+)\)',e.attrib.get('transform','')):
   a=list(map(float,re.findall(r'-?[\d.]+',args)))
   if name=='translate':c.translate(a[0],a[1] if len(a)>1 else 0)
   else:c.scale(a[0],a[1] if len(a)>1 else a[0])
  cp=e.attrib.get('clip-path')
  if cp:
   p=c.beginPath()
   for r in clips[cp[5:-1]]:p.rect(float(r.attrib.get('x',0)),float(r.attrib.get('y',0)),float(r.attrib['width']),float(r.attrib['height']))
   c.clipPath(p,stroke=0,fill=0)
  fill=e.attrib.get('fill',fill);stroke=e.attrib.get('stroke',stroke)
  if fill!='none':c.setFillColorRGB(*colour(fill))
  if stroke!='none':c.setStrokeColorRGB(*colour(stroke));c.setLineWidth(float(e.attrib.get('stroke-width',1)))
  if 'fill-opacity' in e.attrib:c.setFillAlpha(float(e.attrib['fill-opacity']))
  if tag=='rect':c.rect(float(e.attrib.get('x',0)),float(e.attrib.get('y',0)),float(e.attrib['width']),float(e.attrib['height']),fill=fill!='none',stroke=stroke!='none')
  elif tag=='path' and e.attrib.get('d'):
   p=ReportLabPen(None);parse_path(e.attrib['d'],p);p.path.fillColor=Color(*colour(fill)) if fill!='none' else None;p.path.strokeColor=Color(*colour(stroke)) if stroke!='none' else None;p.path.strokeWidth=float(e.attrib.get('stroke-width',1));drawing=Drawing(240,240);drawing.add(p.path);renderPDF.draw(drawing,c,0,0)
  for child in e:walk(child,fill,stroke)
  c.restoreState()
 c.saveState();c.translate(x,y+size);c.scale(size/240,-size/240);walk(root);c.restoreState()
def native(frame,x,y,size):
 raw=bytes.fromhex(frame['native_rgb565']);rgb=bytearray()
 for i in range(0,len(raw),2):
  v=raw[i]<<8|raw[i+1];rgb.extend(((v>>11)*255//31,((v>>5)&63)*255//63,(v&31)*255//31))
 c.drawImage(ImageReader(Image.frombytes('RGB',(240,240),bytes(rgb))),x,y,width=size,height=size,mask='auto')
def text(x,y,s,size=12,bold=False,col=(.83,.89,.88)):
 c.setFillColorRGB(*col);c.setFont('BarlowBold' if bold else 'Barlow',size);c.drawString(x,y,s)
def page(title,subtitle,num):
 c.setFillColorRGB(.09,.12,.14);c.rect(0,0,W,H,fill=1,stroke=0)
 text(26,H-32,'DnBOX  /  NEW WEBSITE WIP',14,True);text(26,H-68,title,27,True)
 text(26,H-91,subtitle,12)
 c.setStrokeColorRGB(.28,.36,.39);c.line(26,44,W-26,44)
 text(26,29,'Actual source 3c41ef4b | new real37 host candidate | source-rendered PDF, not a browser screenshot',9)
 text(26,16,'WIP only. No live WASM, audio, device or browser-pixel acceptance. Current firmware screens, not proposed morph glyphs.',9)
 text(W-38,21,str(num),10)
def keybed(frame,x,y,w,focus=None):
 scale=w/660;whites=[0,2,4,5,7,9,11];keys=[];white=0
 for i in range(37):
  black=i%12 not in whites
  if black:keys.append((i,white*30-9,18,True))
  else:keys.append((i,white*30,30,False));white+=1
 for i,kx,kw,black in sorted(keys,key=lambda k:k[3]):
  fill=(.06,.08,.09) if black else (.91,.94,.92)
  if frame.get('physical',{}).get('held',0)&(1<<i):fill=(.94,.66,.35)
  c.setFillColorRGB(*fill);c.setStrokeColorRGB(.30,.38,.40);c.setLineWidth(.6)
  hh=(86 if black else 128)*scale;c.rect(x+kx*scale,y+128*scale-hh,kw*scale,hh,fill=1,stroke=1)
  k=i-5
  if 0<=k<27 and frame['leds']['keys']&(1<<k):
   c.setStrokeColorRGB(.63,.85,.47);c.setLineWidth(1.5);c.rect(x+kx*scale+1,y+128*scale-hh+1,kw*scale-2,hh-2,fill=0,stroke=1)
 text(x,y-12,'C3',8);text(x+w-12,y-12,'C6',8)
def pads(frame,x,y,w,focus=None):
 gap=4;side=(w-gap*7)/8;white=[0,2,4,6,7,9,11,12,14,16,18,19,21,23,24,26]
 for i in range(16):
  xx=x+(i%8)*(side+gap);yy=y+(1-i//8)*(side+gap);active=i==focus
  c.setFillColorRGB(*((.94,.66,.35) if active else (.15,.21,.24)));c.setStrokeColorRGB(.42,.53,.57);c.roundRect(xx,yy,side,side,3,fill=1,stroke=1)
  if frame['state']['gridPadAvailable'] and frame['leds']['keys']&(1<<white[i]):c.setStrokeColorRGB(.63,.85,.47);c.setLineWidth(2);c.roundRect(xx+2,yy+2,side-4,side-4,2,fill=0,stroke=1)
  text(xx+side/2-3,yy+side/2-3,str(i+1),10,col=(.10,.17,.15) if active else (.85,.90,.88))
def remember(f,name):proof.append({'name':name,'native_sha256':f['proof']['native_sha256'],'bytes_compared':f['proof']['bytes_compared'],'native_state':f['state'],'physical':f.get('physical'),'live':f.get('live')});return f
s=NativeSession()
try:
 juno=remember(s.event('fixture',8,include_oracle=True),'Juno source reference')
 chicago=remember(s.event('fixture',4,include_oracle=True),'Chicago source reference')
 miami=remember(s.event('fixture',7,include_oracle=True),'Miami source reference')
 s.event('fixture',0);pad=remember(s.event('pad',0,1,include_oracle=True),'Pad1 press -> real909 C UI')
 s.event('tick',350);s.event('button',5,1);glo=remember(s.event('button',5,0,include_oracle=True),'GLOBE while original pad held')
 s.event('pad',0,0);s.event('button',5,1);s.event('button',5,0);key=remember(s.event('physical',5,100,include_oracle=True),'Physical F3 -> actual C UI')
 s.event('fixture',8);low=remember(s.event('physical',0,100,include_oracle=True),'Lowest physical C3 -> source gate')
 high=remember(s.event('physical',36,100,include_oracle=True),'Highest physical C6 overlaps held C3 -> source gate')
 restored=remember(s.event('physical',36,0,include_oracle=True),'Release highest, lower key owner remains')
 s.event('release')
finally:s.close()
page('The screen follows your hands.','Current-source Juno: native RGB565 at left, original-outline smooth HTML/SVG at right.',1)
text(28,438,'37 physical keys / C3-C6',13,True);keybed(juno,28,340,340)
text(28,302,'16 pads / top1-8, bottom9-16',13,True);pads(juno,28,212,276)
text(28,181,'All 37 physical keys retain their white/black identity.',11);text(28,162,'All 37 now use the actual production key transport.',10)
text(28,146,'Green: actual native-host LED bits',10)
text(28,125,'Real source geometry and glyph identity.',14,True);text(28,104,'Smooth rendering is not device-pixel-identical.',11)
native(juno,410,240,180);svg(juno['smooth_html'],610,240,180)
text(410,221,'Native pixels / 240 logical',11);text(610,221,'Smooth original outlines',11)
text(410,174,'115,200 C framebuffer bytes agree with replay.',12)
text(410,153,'No screenshot overlay. No invented UI values.',12)
text(410,125,'Displayed Juno state: '+juno['proof']['native_sha256'][:20],10)
c.showPage()
page('One physical gesture. The real source response.','A recorded C-host trace. No live instrument or connected device is implied.',2)
for col,(frame,title,focus_key,focus_pad) in enumerate([(pad,'1  Press pad1',None,0),(glo,'2  Open GLOBE while held',None,0),(key,'3  Press physical F3',5,None)]):
 x=28+col*265;text(x,444,title,15,True);keybed(frame,x,367,238,focus_key);pads(frame,x,294,178,focus_pad);svg(frame['smooth_html'],x,87,180)
 text(x,66,'Actual C view%d / page%d'%(frame['state']['view'],frame['state']['page']),10)
c.showPage()
page('The actual Chicago and Miami screens.','Current-source baseline. The future morph-glyph proposal is a separate design, not silently substituted.',3)
for i,(f,title) in enumerate([(chicago,'CHICAGO'),(miami,'MIAMI')]):
 x=28+i*404;text(x,443,title,18,True);native(f,x,230,180);svg(f['smooth_html'],x+198,230,180)
 text(x,208,'Native RGB565 reference',11);text(x+198,208,'Original-outline smooth',11)
 text(x,173,'Source byte proof: '+f['proof']['native_sha256'][:22],10)
text(28,124,'Renderer/core v2 reviewed. New 37-key candidate tests passed; changed-input review pending.',13)
text(28,103,'Browser, full site/admin/install, actual sample audition and device checks remain separate.',13)
c.showPage()
page('The outer keys now reach the real source.','Not a recolour: production SMK key events, queued epochs, native UI ownership and actual engine gate readback.',4)
for col,(f,title) in enumerate([(low,'1  Lowest physical key: C3'),(high,'2  Add highest key: C6'),(restored,'3  Release C6: C3 remains')]):
 x=28+col*265;text(x,444,title,14,True);keybed(f,x,363,238);svg(f['smooth_html'],x,98,180)
 held=f['physical']['held'];gate=f['live'][0]
 text(x,326,'C physical held bitmap: '+str(held),10)
 text(x,307,'Engine gate '+str(gate['gate'])+' / UI held '+str(gate['uiHeldCount'])+' / UI pitch '+str(gate['uiPitch']),10)
 text(x,77,'Fresh C frame '+f['proof']['native_sha256'][:18],9)
c.showPage();c.save()
(OUT/'REAL37-PROVENANCE.json').write_text(json.dumps({'kind':'NEW source-rendered WIP PDF; not a browser screenshot','source':'3c41ef4bedf9701f549ce2752b73306e2fab1eb8','reviewed_core_manifest_sha256':'b5ec35a108f9f8cee201a55bb07d2c0ca4b6e193ed80fa2c5bbe3fd6787c66b8','states':proof,'browser':'Actual Juno comparison observed earlier; no denied-origin retry. PDF has its own render/visual check.'},indent=2)+'\n')
p=OUT/'DnBOX-WIP-real-37-key-adapter.pdf';print('NEW_PDF',p.stat().st_size,hashlib.sha256(p.read_bytes()).hexdigest())
