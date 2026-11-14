"""Readable phone companion from immutable R7 pixels/text; no UI rerender."""
from pathlib import Path
from xml.sax.saxutils import escape
import hashlib,io,json,os,re
from reportlab.pdfgen import canvas
from reportlab.platypus import BaseDocTemplate,PageTemplate,Frame,Paragraph,Image,Spacer,PageBreak,KeepTogether
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.colors import HexColor
R=Path(__file__).parent
SOURCE=Path(os.environ.get('DNBOX_R7_SOURCE',str(R.parent/'r7')))
W,H=390,844
INK=HexColor('#173126');MUTED=HexColor('#405d4d')
BODY=ParagraphStyle('body',fontName='Helvetica',fontSize=15,leading=19.5,textColor=INK,spaceAfter=9)
SMALL=ParagraphStyle('small',parent=BODY,fontSize=13,leading=17,spaceAfter=6)
TITLE=ParagraphStyle('title',parent=BODY,fontName='Helvetica-Bold',fontSize=20,leading=24,spaceAfter=10)
SUB=ParagraphStyle('sub',parent=BODY,fontName='Helvetica-Bold',fontSize=17,leading=21,spaceAfter=9)
LINK=ParagraphStyle('link',parent=BODY,fontSize=15,leading=19.5,spaceAfter=7)
SHORT=['First beat and drum identity','Juno values and editing','Miami native controls','64-step correction','Motion write and override','Five-part copy','Pattern cues and mutes','Stopped-note modifiers','Song bar authoring','Panic and recovery','Loopy: no host','BREAK: empty-state limits','Drum MIX','GEN controls','GEN held-note correction']
def plain(t):
 for a,b in [('→',' to '),('−','-'),('–','-'),('×',' x '),('…','...'),('≥','>='),('≤','<=')]:t=t.replace(a,b)
 t=re.sub(r'\b(current|original|native|physical|Physical|captured|default|Actual|source|Native|unchanged|at)(?=\d)',r'\1 ',t)
 t=re.sub(r'(?<=\d)(?=(?:px|pt|ms|Hz|bpm)\b)',' ',t)
 t=t.replace('240 x240','240 x 240').replace('Native240','Native 240').replace('source3c41','source 3c41')
 t=re.sub(r'(?<=\d)(?=(?:tracks?|columns?|steps?|bars?|voices?|patterns?|loops?|controls?|semitones?|press(?:es)?|turns?|pages?|cells?|rows?|frames?)\b)',' ',t)
 t=re.sub(r',(?=\S)',', ',t)
 return t
def par(t,style=BODY):return Paragraph(escape(plain(t)),style)
def field(label,t):return Paragraph('<b>'+escape(label)+'.</b> '+escape(plain(t)),BODY)
class Heading(Paragraph):
 def __init__(self,t,key=None):super().__init__(escape(plain(t)),TITLE);self.bookmark=key;self.label=t
class Doc(BaseDocTemplate):
 def afterFlowable(self,f):
  if isinstance(f,Heading) and f.bookmark:
   self.canv.bookmarkPage(f.bookmark);self.canv.addOutlineEntry(f.label,f.bookmark,level=0,closed=False)
def make_canvas(*args,**kwargs):
 kwargs['invariant']=1
 return canvas.Canvas(*args,**kwargs)
def onpage(c,doc):
 c.setFillColor(MUTED);c.setFont('Helvetica-Bold',12);c.drawString(16,H-19,'DnBOX R8 · WORKFLOW STUDY')
 c.setFont('Helvetica',12);c.drawString(16,30,'R8 source; R7 stills reused.')
 c.drawString(16,14,'Proposal; full overhaul incomplete.');c.drawRightString(W-16,14,str(doc.page))
def native(path):
 im=Image(str(path),width=240,height=240);im.hAlign='CENTER';return im
def build():
 manifest=json.loads((SOURCE/'ARTIFACT-MANIFEST.json').read_text());entries={x['file']:x for x in manifest['files']};pairs=json.loads((SOURCE/'comparisons.json').read_text())
 assert hashlib.sha256((SOURCE/'ARTIFACT-MANIFEST.json').read_bytes()).hexdigest()=='f29b41196d1f317306d970765dd1d487f1c8cd3f1c34e1046e8aeb1a4007f0da'
 for p in pairs:
  for name in ['reference/'+p['before'],'native/'+p['after']]:assert hashlib.sha256((SOURCE/name).read_bytes()).hexdigest()==entries[name]['sha256']
 path=R/'DnBOX-R8-mobile-comparisons.pdf'
 doc=Doc(str(path),pagesize=(W,H),leftMargin=16,rightMargin=16,topMargin=36,bottomMargin=48,title='DnBOX R8 mobile workflow comparisons',author='DnBOX design study')
 doc.addPageTemplates(PageTemplate(id='phone',frames=[Frame(16,48,W-32,H-84,leftPadding=0,rightPadding=0,topPadding=0,bottomPadding=0)],onPage=onpage))
 story=[Heading('Native comparisons, readable on a phone','index'),par('Each workflow has a comparison page and larger-text notes. Tap an entry below. The LCD images keep their original240 x240 pixels; no design render is changed.'),par('Actual: current3c41 /0.0.26 Elite. After: exact R7 proposal stills reused for R8. Corrected R8 interactions run separately. Historical Settings remains separate.',SMALL)]
 story.append(par('R7 review found Undo and held-control defects. R8 corrects them in the model; independent affected assessment is pending. Whole design remains incomplete.',SMALL))
 for title,p in zip(SHORT,pairs):story.append(Paragraph('<link href="#pair-'+p['id']+'" color="#245c40"><u>'+escape(title)+'</u></link>',LINK))
 story += [par('The earlier R7 A4 PDF is a desktop/zoom reference. This layout is checked at390px fit width; real iOS opening remains untested.',SMALL),PageBreak()]
 for title,p in zip(SHORT,pairs):
  story += [Heading(title,'pair-'+p['id']),par(p.get('before_caption','CURRENT 3c41 · ELITE 0.0.26'),SMALL),native(SOURCE/'reference'/p['before']),Spacer(1,8),par(p.get('after_caption','PROPOSED · REUSED R7 PIXELS'),SMALL),native(SOURCE/'native'/p['after']),Spacer(1,8),field('Purpose',p['why']),par('Notes follow: retained information, controls, action cost and limitations.',SMALL),PageBreak(),Heading(title+' — notes')]
  for label,key in [('Keep','retained'),('Change / reconsider','changed'),('Controls','sequence'),('Action cost','count'),('Limits / tradeoff','cost')]:story.append(field(label,p[key]))
  if p['id'].startswith('gen-') or p['id']=='record':story.append(field('Predecessor finding / R8 correction','R7 loses history at focus/REC boundaries and its GATE clamps instead of cycling; accent/slide grammar also differs. R8 corrects these source rules and retains original releases. Targeted producer checks pass; independent assessment is pending. Native full-project history remains unmodeled.'))
  story += [par('Native240 x240 PNGs are embedded unchanged. Current and proposed states differ only as explicitly described. Power bars are a proposal inspection fixture.',SMALL),PageBreak()]
 story += [Heading('What this does and does not establish','scope'),field('Current strengths','Useful lower controls, fast engine pages, established native ADSR/Juno glyphs, bright large readouts, original control owners and source pad/button meanings remain. Piano keys have no LEDs.'),field('Meaningful proposal','Direct select-only GEN cell correction removes SEQ and one ARP return. Normal generator controls return on release. Three prototype omissions are corrected; those repairs are not new firmware features.'),field('Evidence','The actual-before images run unchanged3c41 Elite. Accepted d158 has matching relevant UI source and was not executed here. Miami and no-host Loopy preserve native body/lower pixels. GEN held comparison deliberately changes current SEQ to proposed GEN for the same corrected cell. No R7 or R8 firmware exists.'),field('Status','Independent R7 review is complete: useful WIP, partial fidelity, whole overhaul incomplete and not accepted. R8 corrects its bounded Undo and native-control grammar defects in the prototype. Exact R8 independent assessment remains pending. Browser/source-math checks are separate from emulator audio, physical use and durable storage. This mobile PDF is a presentation companion.'),PageBreak(),Heading('The full instrument still needs work','remaining'),field('Sound and live use','Full FX/controller work, valid populated BREAK, real Loopy/SMC peer, ownership, clock loss/rejoin and physical recognition/pressure use remain unfinished.'),field('Recording and recovery','Running/poly/duration/expressive capture, complete correction/replay/repeated takes and durable save/reload still need evidence. Preset/held-note decisions precede selected capture strategy and lifecycle. Settings shared status and held-note continuity remain open.'),field('Power','The actual platform exposes0–3bars, USB-host power or unknown. It has no calibrated percentage or genuine charging signal. The proposal does not fabricate them.'),field('Limits','Standard PDF parse, native-pixel identity and390px-width render/readability checks are separate from actually opening it on an iOS device. Device opening remains untested. No implementation, device operation or release is implied.'),Paragraph('<link href="https://github.com/gobbajogger/X0X37-for-Loopy-Pro/tree/9b82d94dbc259cee7a00dcc6285522c36b8db3ba/docs/design/dnbox-whole-instrument-20261114/r7" color="#245c40"><u>Original R7 visual source and evidence</u></link>',BODY)]
 doc.build(story,canvasmaker=make_canvas)
 receipt={'file':path.name,'bytes':path.stat().st_size,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'source_commit':'9b82d94dbc259cee7a00dcc6285522c36b8db3ba','source_manifest_sha256':'f29b41196d1f317306d970765dd1d487f1c8cd3f1c34e1046e8aeb1a4007f0da','target_revision':'R8 corrective prototype','source_images':'All30 native images reuse exact R7 pixels; no redraw or R8 frame-execution claim','page_points':[W,H],'body_points':15,'minor_points':13,'footer_points':12,'classification':'Mobile presentation companion; not new UI or independent acceptance; real iOS NOT_RUN'}
 (R/'BUILD-RECEIPT.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps(receipt))
if __name__=='__main__':build()
