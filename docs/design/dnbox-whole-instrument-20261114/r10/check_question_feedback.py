"""Native-size question feedback: current source acknowledges Redo while ASK stays."""
from pathlib import Path
import asyncio,json,hashlib,base64,io,sys
from PIL import Image
from playwright.async_api import async_playwright
async def main():
 source,out=map(Path,sys.argv[1:3]);out.mkdir(parents=True,exist_ok=True);raw=(source/'workbench.html').read_bytes();checks=[];frames=[]
 async with async_playwright() as pw:
  browser=await pw.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);p=await browser.new_page();await p.route('**/*',lambda r:r.abort());await p.set_content(raw.decode());await p.wait_for_function('window.__ready')
  await p.evaluate("proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('OCTAVE+');proposal.physicalUp(9,800);proposal.buttonDown('PATCH');proposal.buttonDown('SEQ REC');proposal.buttonUp('SEQ REC');proposal.buttonUp('PATCH');S.patternData[5].drums[0].steps[0]=[2];proposal.buttonDown('BT');proposal.padDown(5);proposal.release(80,5);proposal.buttonUp('BT');proposal.buttonDown('PATCH')")
  async def capture(name):
   b=base64.b64decode((await p.evaluate("document.getElementById('lcd').toDataURL('image/png')")).split(',')[1]);(out/(name+'.png')).write_bytes(b);d=await p.evaluate('({bounds:structuredClone(bounds),state:proposal.state})');frames.append({'file':name+'.png','bytes':len(b),'sha256':hashlib.sha256(b).hexdigest(),'bounds':d['bounds']});return b,d
  before,detail=await capture('question-PATCH-held')
  hints=[x for x in detail['bounds'] if 'REDO' in x['text'] and x['y']>=130];checks.append({'name':'HeldPATCH cue describes Redo and fits nativeLCD','pass':bool(hints) and all(x['x']>=0 and x['x']+x['width']<=240 for x in hints),'bounds':hints})
  await p.evaluate("proposal.buttonDown('SEQ PLAY');proposal.buttonUp('SEQ PLAY');proposal.buttonUp('PATCH')")
  after,detail=await capture('question-after-Redo-original-releases');s=detail['state'];ack=[x for x in detail['bounds'] if x['y']<20 and x['text'].startswith('REDO')]
  checks.append({'name':'Redo acknowledgement names editedowner/pattern while question remains','pass':bool(ack) and any('CHICAGO' in x['text'] and 'P1' in x['text'] for x in ack) and s['overlay']=='ASK' and s['view']=='GEN' and s['patternData'][0]['bass'][1]['notes'][2]['note']==67 and not s['playing'] and not s['buttons'],'bounds':ack})
  im1=Image.open(io.BytesIO(before)).convert('RGB');im2=Image.open(io.BytesIO(after)).convert('RGB');checks.append({'name':'Redo acknowledgement preserves right transport/record/power pixels','pass':im1.crop((180,0,240,20)).tobytes()==im2.crop((180,0,240,20)).tobytes()})
  checks.append({'name':'All visible question text fits native240LCD','pass':all(x['x']>=-0.01 and x['x']+x['width']<=240.01 and x['y']+x['height']<=240 for x in detail['bounds'])})
  await browser.close()
 r={'classification':'Producer localChromium native240proposal render/feedback checks, not currentfirmware/device/independent proof','html_sha256':hashlib.sha256(raw).hexdigest(),'checks':checks,'frames':frames,'passed':sum(c['pass'] for c in checks),'total':len(checks)};(out/'FEEDBACK-CHECKS.json').write_text(json.dumps(r,indent=2)+'\n');print(json.dumps({k:r[k] for k in ['html_sha256','passed','total']}))
 if r['passed']!=r['total']:raise SystemExit(1)
asyncio.run(main())
