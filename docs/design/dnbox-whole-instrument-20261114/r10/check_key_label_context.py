"""Inspection labels vs modeled MIDI routing; no physical pitch/listening claim."""
from pathlib import Path
import asyncio,json,hashlib,sys
from playwright.async_api import async_playwright
async def main():
 source,out=map(Path,sys.argv[1:3]);raw=(source/'workbench.html').read_bytes();checks=[]
 async with async_playwright() as pw:
  b=await pw.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);p=await b.new_page();await p.route('**/*',lambda r:r.abort());await p.set_content(raw.decode());await p.wait_for_function('window.__ready')
  for name,setup,key,note,label in [('PART outerkey octave0',"proposal.reset('chicago')",0,36,'C2'),('PART outerkey octave1',"proposal.reset('chicago');proposal.event('OCTAVE+')",0,48,'C3'),('GEN REC centralkey octave0',"proposal.reset('gen-known');proposal.buttonDown('SEQ REC');proposal.buttonUp('SEQ REC')",5,41,'F2')]:
   await p.evaluate(setup+';proposal.physicalDown('+str(key)+')');d=await p.evaluate("(()=>{const s=proposal.state,b=document.querySelector('[data-physical=\""+str(key)+"\"]');return {note:s.liveNotes["+str(key)+"]?.note,text:b.textContent,aria:b.getAttribute('aria-label')}})()");checks.append({'name':name,'pass':d['note']==note and label in d['text'],'want':{'note':note,'model_label':label},'actual':d})
  await b.close()
 d={'classification':'Producer software inspection-label/endpoint consistency only; controller-map octave names and actual pitch remain distinct','html_sha256':hashlib.sha256(raw).hexdigest(),'checks':checks,'passed':sum(c['pass'] for c in checks),'total':len(checks)};out.write_text(json.dumps(d,indent=2)+'\n');print(json.dumps({k:d[k] for k in ['html_sha256','passed','total']}))
 if d['passed']!=d['total']:raise SystemExit(1)
asyncio.run(main())
