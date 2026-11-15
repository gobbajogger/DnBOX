"""Catches GEN scalar dispatch stealing SAVE+lower-knob and leaking SAVE on release."""
from pathlib import Path
import asyncio,hashlib,json,sys
from playwright.async_api import async_playwright

async def main():
 source,output=map(Path,sys.argv[1:3]);raw=(source/'workbench.html').read_bytes();checks=[];errors=[]
 async with async_playwright() as pw:
  browser=await pw.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);p=await browser.new_page();p.on('pageerror',lambda e:errors.append(str(e)));await p.route('**/*',lambda r:r.abort());await p.set_content(raw.decode());await p.wait_for_function('window.__ready')
  for page in [0,1]:
   for knob in ['K5','K6','K7','K8']:
    for direction in [1,-1]:
     await p.evaluate("proposal.reset('gen-known');"+("proposal.event('ARP');" if page else '')+"window.genBefore=JSON.stringify(proposal.state.patternData[0].bass[1]);window.motionBefore=JSON.stringify(proposal.state.motionLanes);window.traceBefore=proposal.trace.length;proposal.buttonDown('BT');proposal.event('"+knob+"',"+str(direction)+");window.consumedAtTurn=proposal.state.usedButtons.includes('BT');window.turnState=proposal.state;proposal.buttonUp('BT')")
     detail=await p.evaluate("(()=>{const s=proposal.state;return {pass:JSON.stringify(s.patternData[0].bass[1])===genBefore&&JSON.stringify(s.motionLanes)===motionBefore&&s.genUndo.length===0&&s.genRedo.length===0&&consumedAtTurn&&!s.heldSave&&s.buttons.length===0&&!proposal.trace.slice(traceBefore).some(e=>e.event==='BT'),configuration:s.patternData[0].bass[1].gen,consumedAtTurn,undo:s.genUndo.length,redo:s.genRedo.length,rec:s.rec,playing:s.playing,heldSave:s.heldSave,buttons:s.buttons,releaseTrace:proposal.trace.slice(traceBefore),note:s.note}})()")
     checks.append({'name':f'GEN page{page} SAVE+{knob} direction{direction} consumes no-motion gesture/release without data/history edit','pass':detail['pass'],'actual':detail})
  await p.evaluate("proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);proposal.physicalUp(9,800);window.editBefore=JSON.stringify(proposal.state.patternData[0].bass[1]);proposal.buttonDown('BT');proposal.event('K5',1);proposal.buttonUp('BT')")
  detail=await p.evaluate("(()=>{const s=proposal.state;return {pass:s.genUndo.length===1&&s.genRedo.length===0&&JSON.stringify(s.patternData[0].bass[1])===editBefore,note:s.patternData[0].bass[1].notes[2].note,undo:s.genUndo.length,redo:s.genRedo.length}})()")
  checks.append({'name':'SAVE+GEN knob preserves earlier intentional history','pass':detail['pass'],'actual':detail})
  await p.evaluate("proposal.reset('gen-known');proposal.event('SCALE');window.listBefore=JSON.stringify(proposal.state.patternData[0].bass[1]);proposal.buttonDown('BT');proposal.event('K5',1);window.listConsumed=proposal.state.usedButtons.includes('BT');proposal.buttonUp('BT')")
  detail=await p.evaluate("(()=>{const s=proposal.state;return {pass:s.overlay==='LIST'&&JSON.stringify(s.patternData[0].bass[1])===listBefore&&listConsumed&&!s.heldSave&&s.genUndo.length===0,overlay:s.overlay,consumed:listConsumed,undo:s.genUndo.length}})()")
  checks.append({'name':'LIST retains underlying GEN SAVE+lower-knob precedence','pass':detail['pass'],'actual':detail})
  await p.evaluate("proposal.reset('chicago');window.partCell=proposal.control(0,0);window.baseBefore=JSON.stringify(proposal.state.values);window.testLaneId='0:3:'+partCell.key;proposal.configure({motionLanes:{[testLaneId]:{pattern:0,part:3,key:partCell.key,values:Array(64).fill(50),applied:null,recReq:false,holdReq:false,holdLeft:0,lastRec:null,quiet:0}}});proposal.buttonDown('BT');proposal.event('K5',1);window.partConsumed=proposal.state.usedButtons.includes('BT');proposal.buttonUp('BT')")
  detail=await p.evaluate("(()=>{const s=proposal.state;return {pass:!s.motionLanes[testLaneId]&&JSON.stringify(s.values)===baseBefore&&partConsumed&&!s.heldSave,remainingLanes:Object.keys(s.motionLanes),consumed:partConsumed}})()")
  checks.append({'name':'Existing PART SAVE+knob still clears its motion lane without editing base','pass':detail['pass'],'actual':detail})
  await browser.close()
 result={'classification':'Producer MODEL/DOM source-contract checks; no firmware, independent, actual-save or device proof','input_html_sha256':hashlib.sha256(raw).hexdigest(),'source_contract':'3c41 ui.c4667–4672 consumes lower physical SAVE+knob; motion_forget2803ff ignores nonR_ENG including GEN refs','checks':checks,'errors':errors,'passed':sum(c['pass'] for c in checks),'total':len(checks)};output.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'html_sha256':result['input_html_sha256'],'passed':result['passed'],'total':result['total'],'failed':[c['name'] for c in checks if not c['pass']],'errors':errors,'output':str(output)}))
 if errors:raise SystemExit(2)
 if result['passed']!=result['total']:raise SystemExit(1)
asyncio.run(main())
