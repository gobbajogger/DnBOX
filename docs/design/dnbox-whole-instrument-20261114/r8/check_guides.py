"""Final successor DOM smoke of the seven retained connected paths."""
from pathlib import Path
import asyncio,json,hashlib,base64
from playwright.async_api import async_playwright
R=Path(__file__).parent
async def main():
 checks=[];traces=[];errors=[];frames=[]
 async with async_playwright() as pw:
  b=await pw.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);p=await b.new_page();p.on('pageerror',lambda e:errors.append(str(e)));await p.route('**/*',lambda r:r.abort());await p.set_content((R/'workbench.html').read_text());await p.wait_for_function('window.__ready');await p.locator('[data-tab="model"]').click()
  guides=await p.evaluate('D.guides')
  for i,guide in enumerate(guides):
   await p.locator('#guideChoice').select_option(str(i));await p.locator('#startGuide').click()
   for j,_ in enumerate(guide['stages']):
    await p.locator('#nextGuide').click();s=await p.evaluate('proposal.state');traces.append({'guide':guide['id'],'stage':j+1,'view':s['view'],'part':s['sourcePart'],'pattern':s['pattern'],'held':s['held'],'playing':s['playing'],'rec':s['rec']})
   s=await p.evaluate('proposal.state');key=guide['id'];ok={'64':s['stepPage']==3 and s['held']==-1,'motion':not s['motionLanes'],'copy':s.get('lastCopy') is not None,'cue':s['pendingPatterns']==[-1]*5,'record':s['patternData'][0]['bass'][1]['notes'][2]['note']==56,'panic':not s['panicLatched'],'gen':s['view']=='GEN' and not s['playing'] and s['held']==-1 and bool(s['genUndo'])}[key];checks.append({'name':key+' retained guide endpoint','pass':bool(ok)})
  await p.evaluate("proposal.reset('gen-known');proposal.physicalDown(9)")
  target=R/'verification/native-r8';target.mkdir(parents=True,exist_ok=True)
  for gate in ['TIE','REST','NOTE']:
   await p.evaluate("proposal.event('K6',1)");draw=await p.evaluate('proposal.render()');s=await p.evaluate('proposal.state');raw=base64.b64decode(await p.locator('#lcd').evaluate("e=>e.toDataURL().split(',')[1]"));name='held-gate-'+gate.lower()+'.png';(target/name).write_bytes(raw)
   frames.append({'file':'verification/native-r8/'+name,'bytes':len(raw),'sha256':hashlib.sha256(raw).hexdigest(),'gate':s['patternData'][0]['bass'][1]['notes'][2]['gate'],'bounds':draw['bounds'],'classification':'R8 proposal actual Chromium240canvas; not firmware/device'})
   checks.append({'name':'Native held '+gate+' readout and bounds','pass':s['patternData'][0]['bass'][1]['notes'][2]['gate']==gate and any(x['text']==gate for x in draw['bounds']) and all(x['x']>=0 and x['x']+x['width']<=240.1 and x['y']>=0 and x['y']+x['height']<=240 for x in draw['bounds'])})
  await p.evaluate('proposal.physicalUp(9,800)');checks.append({'name':'No browser exception','pass':not errors,'detail':errors});await b.close()
 d={'classification':'Producer final local Chromium proposal-only guide check; not firmware/audio/device/independent acceptance','html_sha256':hashlib.sha256((R/'workbench.html').read_bytes()).hexdigest(),'passed':sum(c['pass'] for c in checks),'total':len(checks),'checks':checks,'staged_states':len(traces),'traces':traces};(R/'GUIDE-CHECKS.json').write_text(json.dumps(d,indent=2)+'\n');(R/'R8-NATIVE-CORRECTION-FRAMES.json').write_text(json.dumps({'html_sha256':d['html_sha256'],'frames':frames},indent=2)+'\n');print(json.dumps({k:v for k,v in d.items() if k not in ['traces']}));assert d['passed']==d['total']
asyncio.run(main())
