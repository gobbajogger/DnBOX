"""Bounded native-size proposal previews; actual-current references stay separate."""
from pathlib import Path
import asyncio,base64,hashlib,json,sys
from playwright.async_api import async_playwright
async def main():
 source,out=map(Path,sys.argv[1:3]);out.mkdir(parents=True,exist_ok=True)
 raw=(source/'workbench.html').read_bytes();checks=[];frames=[];errors=[]
 async with async_playwright() as pw:
  b=await pw.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);p=await b.new_page();p.on('pageerror',lambda e:errors.append(str(e)));await p.route('**/*',lambda r:r.abort());await p.set_content(raw.decode());await p.wait_for_function('window.__ready')
  async def capture(name,script):
   await p.evaluate(script)
   data=base64.b64decode((await p.evaluate("document.getElementById('lcd').toDataURL('image/png')")).split(',')[1]);(out/(name+'.png')).write_bytes(data)
   bounds=await p.evaluate('structuredClone(bounds)');frames.append({'file':name+'.png','bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'bounds':bounds})
   checks.append({'name':name+' text fits native240','pass':all(x['x']>=-0.01 and x['x']+x['width']<=240.01 and x['y']+x['height']<=240 for x in bounds)})
  await capture('clear-909-question',"proposal.reset('drum');proposal.buttonDown('BT');proposal.buttonDown('SEQ REC');proposal.buttonUp('SEQ REC');proposal.buttonUp('BT')")
  checks.append({'name':'Clear retains original YES/NO actions and adds pattern/shared scope before confirmation','pass':await p.evaluate("bounds.some(x=>x.text==='SCALE YES')&&bounds.some(x=>x.text==='PATCH NO')&&bounds.some(x=>x.text.includes('SHARED FX'))&&proposal.state.overlay==='ASK'&&!proposal.state.rec")})
  await capture('held-PART-LIST-base-edit',"proposal.reset('chicago');proposal.event('SEQ');proposal.physicalDown(9);proposal.event('PARA');proposal.event('SCALE');proposal.event('K5',1)")
  checks.append({'name':'LIST preview carries original held owner but no step lock','pass':await p.evaluate("proposal.state.held===3&&proposal.state.overlay==='LIST'&&!Object.keys(proposal.state.motionLanes).length")})
  await p.evaluate("proposal.reset('home');proposal.configure({tempo:275});proposal.buttonDown('PATCH');proposal.event('K2',1);proposal.buttonUp('PATCH')")
  checks.append({'name':'PATCH tempo upper clamp retains HOME and original release','pass':await p.evaluate("proposal.state.tempo===275&&proposal.state.view==='HOME'")})
  await p.evaluate("proposal.configure({tempo:20});proposal.buttonDown('PATCH');proposal.event('K2',-1);proposal.buttonUp('PATCH')")
  checks.append({'name':'PATCH tempo lower clamp retains HOME and original release','pass':await p.evaluate("proposal.state.tempo===20&&proposal.state.view==='HOME'")})
  await b.close()
 result={'classification':'Producer Chromium proposal-only native renders and bounded dispatch checks; no firmware/device/independent acceptance','html_sha256':hashlib.sha256(raw).hexdigest(),'frames':frames,'checks':checks,'errors':errors,'passed':sum(x['pass'] for x in checks),'total':len(checks)}
 (out/'VISUAL-CHECKS.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({k:result[k] for k in ['html_sha256','passed','total','errors']}));assert result['passed']==result['total'] and not errors
asyncio.run(main())
