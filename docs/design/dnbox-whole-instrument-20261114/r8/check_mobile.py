"""Verify mobile PDF at an actual390px fit width, no firmware/browser."""
from pathlib import Path
import os,json,hashlib,io,math,re
import fitz
from PIL import Image
R=Path(__file__).parent;S=Path(os.environ.get('DNBOX_R7_SOURCE',str(R.parent/'r7')))
pdf=R/'DnBOX-R8-mobile-comparisons.pdf';doc=fitz.open(pdf);pairs=json.loads((S/'comparisons.json').read_text());checks=[];renders=[]
def ck(n,v,d=None):checks.append({'name':n,'pass':bool(v),'detail':d})
def sha(b):return hashlib.sha256(b).hexdigest()
image_pages=[];texts=[];fit_pixel_matches=0;body_sizes=[]
for i,p in enumerate(doc):
 ck('Phone page geometry '+str(i+1),p.rect.width==390 and p.rect.height==844)
 pix=p.get_pixmap(matrix=fitz.Matrix(390/p.rect.width,390/p.rect.width),alpha=False);im=Image.frombytes('RGB',(pix.width,pix.height),pix.samples);ck('Actual fit-width raster390x844 '+str(i+1),im.size==(390,844))
 text=p.get_text();texts.append(text);bad=[]
 for block in p.get_text('dict')['blocks']:
  for line in block.get('lines',[]):
   for span in line['spans']:
    x0,y0,x1,y1=span['bbox']
    if x0<15.9 or x1>374.1 or y0<0 or y1>844:bad.append(span['text'])
    if 34<=y0 and y1<796:body_sizes.append(span['size'])
 ck('Phone text remains within page '+str(i+1),not bad,bad)
 images=p.get_images(full=True)
 if images:
  index=len(image_pages);pair=pairs[index];expected=[Image.open(S/'reference'/pair['before']).convert('RGB'),Image.open(S/'native'/pair['after']).convert('RGB')]
  extracted=[fitz.Pixmap(doc,x[0]).samples for x in images]
  ck('Unchanged embedded native pair '+pair['id'],len(images)==2 and sorted(extracted)==sorted(x.tobytes() for x in expected))
  per=[]
  for x in images:
   rects=p.get_image_rects(x[0]);rect=rects[0];coords=[rect.x0,rect.y0,rect.x1,rect.y1];integer=all(abs(v-round(v))<.01 for v in coords)
   same=False
   if integer and rect.width==240 and rect.height==240:
    crop=im.crop(tuple(round(v) for v in coords));same=crop.tobytes()==fitz.Pixmap(doc,x[0]).samples
   fit_pixel_matches+=same;per.append({'rect':coords,'native240':rect.width==240 and rect.height==240,'pixel_identical_at390fit':same})
  ck('Actual phone-fit native pixels '+pair['id'],all(x['native240'] and x['pixel_identical_at390fit'] for x in per),per);image_pages.append({'page':i+1,'id':pair['id']})
 if i+1 in [1,2,3,10,11,16,17,28,29,30,31,32,33]:
  out=R/'verification/fit-390'/f'page-{i+1:02d}.png';out.parent.mkdir(parents=True,exist_ok=True);pix.save(out);renders.append({'file':str(out.relative_to(R)),'sha256':sha(out.read_bytes()),'pixels':[390,844]})
ck('Fifteen distinct paired workflow pages',len(image_pages)==15)
ck('All30native images remain exact at actual390px fit',fit_pixel_matches==30)
ck('Owner body/minor text at least13px fit width',min(body_sizes)>=13,{'minimum':min(body_sizes),'body':15,'page_scale_at390':1.0})
links=doc[0].get_links();targets=[]
for x in links:
 kind,value=doc.xref_get_key(x['xref'],'Dest');m=re.search(r'\[\s*(\d+)\s+0\s+R',value);targets.append(int(m.group(1)) if m else None)
ck('Fifteen working in-document workflow links',len(links)==15 and targets==[doc[p['page']-1].xref for p in image_pages],{'target_page_xrefs':targets})
text=' '.join(' '.join(texts).split());ck('Predecessor limitations and pending R8 assessment remain explicit','Undo' in text and 'GATE clamps' in text and 'partial' in text)
patterns=[r'/(?:workspace|root)/',r'link_[0-9a-f]+',r'\b[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}\b',r'https://chatgpt\.com/',r'\bincumbent\b',r'\bparent\b']
ck('No private routing/paths/identifiers in owner PDF',not any(re.search(x,text,re.I) for x in patterns))
receipt={'classification':'Producer mobile PDF raster/pixel/link/text verification; not design acceptance or real iOS/device test','pdf_sha256':sha(pdf.read_bytes()),'source_commit':'9b82d94dbc259cee7a00dcc6285522c36b8db3ba','pages':len(doc),'fit_width_pixels':390,'main_body_font_pixels_at_fit':15,'minor_font_pixels_at_fit':13,'old_A4_body_font_pixels_at_fit':8.7*390/595.2756,'passed':sum(c['pass'] for c in checks),'total':len(checks),'checks':checks,'image_pages':image_pages,'representative_renders':renders,'limits':['Actual iOS device opening NOT_RUN','Native images unchanged; no firmware/proposal rerender','R7 known model defects remain; successor corrections separate']}
(R/'MOBILE-READABILITY-CHECKS.json').write_text(json.dumps(receipt,indent=2)+'\n');print(json.dumps({'pages':len(doc),'passed':receipt['passed'],'total':receipt['total'],'native_fit_pixel_matches':fit_pixel_matches,'failures':[c for c in checks if not c['pass']]}));assert receipt['passed']==receipt['total']
