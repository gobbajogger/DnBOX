"""Affected GEN transaction checks against frozen R7 or working successor."""
from pathlib import Path
import asyncio,json,hashlib,sys,os
from playwright.async_api import async_playwright
R=Path(__file__).parent
BASE=Path(os.environ.get('DNBOX_R7_SOURCE',str(R.parent/'r7')))
CANDIDATE=Path(os.environ.get('DNBOX_R8_SOURCE',str(R)))
async def main():
 mode=sys.argv[1];html=(BASE/'workbench.html').read_text();old=(BASE/'generator-proposal.js').read_text();new=old if mode=='r7' else (CANDIDATE/'generator-proposal.js').read_text()
 assert hashlib.sha256(html.encode()).hexdigest()=='131db6ae9aa436274735d9dca3602f7d902c3c1a32453a302d118ed6f70a2659';assert html.count(old)==1
 base_js=(BASE/'proposal.js').read_text();working_js=base_js if mode=='r7' else (CANDIDATE/'proposal.js').read_text();html=html if mode=='r7' else (CANDIDATE/'workbench.html').read_text();assert html.count(working_js)==1 and html.count(new)==1;checks=[]
 async with async_playwright() as pw:
  b=await pw.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);p=await b.new_page();await p.route('**/*',lambda r:r.abort());await p.set_content(html);await p.wait_for_function('window.__ready')
  async def ev(s):return await p.evaluate(s)
  async def ck(name,js):
   d=await ev(js);checks.append({'name':name,'pass':bool(d['pass']),'detail':d})
  hist="window.hist=function(redo=false){proposal.buttonDown('PATCH');proposal.buttonDown(redo?'SEQ PLAY':'SEQ REC');proposal.buttonUp(redo?'SEQ PLAY':'SEQ REC');proposal.buttonUp('PATCH')}"
  await ev(hist)
  await ev("proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);proposal.physicalDown(12);proposal.event('K5',1);proposal.physicalUp(9,800);proposal.physicalUp(12,800);hist();hist()")
  await ck('Two held-cell edits retain two Undo transactions',"(()=>{const s=proposal.state,b=s.patternData[0].bass[1];return {pass:b.notes[2].note===55&&b.notes[4].note===57,notes:[b.notes[2].note,b.notes[4].note],undo:s.genUndo.length,redo:s.genRedo.length}})()")
  await ev("proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);proposal.event('SEQ REC');proposal.physicalUp(9,800);hist()")
  await ck('REC entry commits correction before retiring focus',"(()=>{const s=proposal.state,b=s.patternData[0].bass[1];return {pass:b.notes[2].note===55&&s.rec&&s.view==='GEN'&&!Object.keys(s.physicalOwners).length,notes:b.notes[2].note,rec:s.rec,undo:s.genUndo.length,redo:s.genRedo.length}})()")
  if mode!='r7':
   await ev("hist(true)");await ck('Redo after REC keeps record/transport/view',"(()=>{const s=proposal.state;return {pass:s.patternData[0].bass[1].notes[2].note===56&&s.rec&&!s.playing&&s.view==='GEN'}})()")
   for old_first in [True,False]:
    await ev("proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);proposal.physicalDown(12);proposal.event('K5',1)")
    await ev('proposal.physicalUp('+str(9 if old_first else 12)+',800);proposal.physicalUp('+str(12 if old_first else 9)+',800);hist();hist();hist(true);hist(true)')
    await ck('Redo restores both edits; original release order '+str(old_first),"(()=>{const s=proposal.state,b=s.patternData[0].bass[1];return {pass:b.notes[2].note===56&&b.notes[4].note===58&&s.genUndo.length===2&&s.genRedo.length===0&&s.held===-1&&!Object.keys(s.physicalOwners).length}})()")
   await ev("proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);proposal.physicalCancel(9);hist()")
   await ck('Physical cancellation preserves correction history and retires owner',"(()=>{const s=proposal.state;return {pass:s.patternData[0].bass[1].notes[2].note===55&&s.held===-1&&!Object.keys(s.physicalOwners).length&&s.genRedo.length===1}})()")
   await ev("proposal.reset('gen-known');proposal.padDown(2);proposal.event('K5',1);proposal.cancelPad(2);hist()")
   await ck('Pad cancellation retains intentional edit transaction',"(()=>{const s=proposal.state;return {pass:s.patternData[0].bass[1].notes[2].note===55&&!s.physicalPadsHeld.length&&s.genRedo.length===1}})()")
   await ev("proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);proposal.event('PARA');proposal.physicalUp(9,800);hist()")
   await ck('View transition commits once without retargeting old release',"(()=>{const s=proposal.state;return {pass:s.patternData[0].bass[1].notes[2].note===55&&s.genUndo.length===0&&s.genRedo.length===1&&s.view==='PART'&&!Object.keys(s.physicalOwners).length}})()")
   await ev("proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);hist();proposal.physicalUp(9,800);hist(true)")
   await ck('Undo while held and release cannot duplicate correction',"(()=>{const s=proposal.state;return {pass:s.patternData[0].bass[1].notes[2].note===56&&s.genUndo.length===1&&s.genRedo.length===0}})()")
   await ev("proposal.reset('gen-known');proposal.physicalDown(9);proposal.padDown(2);proposal.event('K5',1);proposal.physicalUp(9,800)");await ck('Aliased key release keeps remaining pad edit owner',"(()=>{const s=proposal.state;return {pass:s.held===3&&s.physicalPadsHeld.includes(2)&&s.genUndo.length===0}})()")
   await ev('proposal.release(800,2);hist()');await ck('Final alias release commits exactly once',"(()=>{const s=proposal.state;return {pass:s.patternData[0].bass[1].notes[2].note===55&&s.genRedo.length===1&&!s.physicalPadsHeld.length}})()")
   await ev("proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);proposal.buttonDown('SEQ REC');proposal.buttonUp('SEQ REC');proposal.physicalUp(9,800);hist()")
   await ck('Physical REC button route also commits original edit',"(()=>{const s=proposal.state;return {pass:s.patternData[0].bass[1].notes[2].note===55&&s.rec&&s.genRedo.length===1}})()")
   await ev("proposal.reset('gen-known');proposal.physicalDown(9);proposal.physicalDown(12);proposal.physicalUp(9,800);proposal.physicalUp(12,800)")
   await ck('Selection-only handoffs create no data history',"(()=>{const s=proposal.state;return {pass:s.genUndo.length===0&&s.genRedo.length===0}})()")
   await ev("proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);proposal.event('SCALE');proposal.physicalUp(9,800);proposal.event('PATCH');hist()")
   await ck('List transition commits once and old release does not retarget',"(()=>{const s=proposal.state;return {pass:s.patternData[0].bass[1].notes[2].note===55&&s.genUndo.length===0&&s.genRedo.length===1&&!Object.keys(s.physicalOwners).length&&s.view==='GEN'}})()")
   await ev("proposal.reset('gen-known');window.p2Before=JSON.stringify(proposal.state.patternData[1].bass[1]);proposal.physicalDown(9);proposal.event('K5',1);proposal.event('K4',1);proposal.boundary('declared model cue adoption');proposal.physicalUp(9,800);hist()")
   await ck('Cue adoption commits original pattern and preserves destination',"(()=>{const s=proposal.state;return {pass:s.patternData[0].bass[1].notes[2].note===55&&JSON.stringify(s.patternData[1].bass[1])===p2Before&&s.genUndo.length===0&&s.genRedo.length===1&&s.pattern===2&&!Object.keys(s.physicalOwners).length}})()")
   await ev("proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K2',-1);proposal.physicalUp(9,800);hist()")
   await ck('GEN chance-only Undo removes its newly allocated lane',"(()=>{const s=proposal.state;return {pass:!s.motionLanes['0:3:PROB:3:0']&&s.genRedo.length===1}})()")
   await ev('hist(true)');await ck('Chance Redo restores the original cell value',"(()=>{const s=proposal.state;return {pass:s.motionLanes['0:3:PROB:3:0']?.values[2]===95&&s.genUndo.length===1}})()")
   await ev("proposal.reset('gen-known');proposal.configure({motionLanes:{unrelated:{pattern:0,part:2,key:'2:MIAMI:2',values:Array(64).fill(42)}}});window.otherLane=JSON.stringify(proposal.state.motionLanes.unrelated);proposal.physicalDown(9);proposal.event('K2',-1);hist();proposal.physicalUp(9,800)")
   await ck('Held chance Undo and release do not recommit or touch unrelated motion',"(()=>{const s=proposal.state;return {pass:!s.motionLanes['0:3:PROB:3:0']&&s.genUndo.length===0&&s.genRedo.length===1&&JSON.stringify(s.motionLanes.unrelated)===otherLane}})()")
   await ev("proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K2',-1);proposal.event('K5',1);proposal.physicalDown(12);proposal.event('K2',-1);proposal.event('K5',1);proposal.physicalUp(9,800);proposal.physicalUp(12,800);hist();hist()")
   await ck('Two focus transactions restore their notes and chance together',"(()=>{const s=proposal.state,b=s.patternData[0].bass[1];return {pass:b.notes[2].note===55&&b.notes[4].note===57&&!s.motionLanes['0:3:PROB:3:0']&&s.genRedo.length===2}})()")
  for view in ['GEN','SEQ']:
   for direction in [1,-1]:
    await ev("proposal.reset('gen-known');"+("proposal.event('SEQ');" if view=='SEQ' else '')+"proposal.physicalDown(9);")
    await ev(';'.join('proposal.event(\'K6\','+str(direction)+')' for _ in range(3)))
    await ck(view+' repeated GATE turns cycle in direction '+str(direction),"(()=>{const s=proposal.state,c=s.patternData[0].bass[1].notes[2];return {pass:c.gate==='NOTE'&&s.held===3&&s.press.input===2,gate:c.gate}})()")
   for flag,knob in [('accent','K7'),('slide','K8')]:
    for direction,turns in [(1,2),(-1,1)]:
     await ev("proposal.reset('gen-known');"+("proposal.event('SEQ');" if view=='SEQ' else '')+"proposal.physicalDown(9);"+';'.join('proposal.event(\''+knob+'\','+str(direction)+')' for _ in range(turns)))
     expected='false' if turns==2 else 'true'
     await ck(view+' '+flag+' XOR turns '+str(direction)+'x'+str(turns),"(()=>{const s=proposal.state,c=s.patternData[0].bass[1].notes[2];return {pass:c."+flag+"==="+expected+"&&s.held===3&&s.press.input===2,value:c."+flag+"}})()")
   if mode!='r7':
    await ev("proposal.reset('gen-known');bassData().notes[2].end=7;bassData().notes[2].accent=true;bassData().notes[2].slide=true;"+("proposal.event('SEQ');" if view=='SEQ' else '')+"proposal.physicalDown(9);proposal.event('K6',1)")
    await ck(view+' GATE clears endpoint and retains accent/slide',"(()=>{const s=proposal.state,c=s.patternData[0].bass[1].notes[2];return {pass:c.gate==='TIE'&&c.end===0&&c.accent&&c.slide}})()")
    await ev('proposal.physicalUp(9,800)');await ck(view+' original edited-cell release keeps note and retires owner',"(()=>{const s=proposal.state;return {pass:s.patternData[0].bass[1].notes[2].gate==='TIE'&&s.held===-1&&!Object.keys(s.physicalOwners).length}})()")
  await b.close()
 out={'classification':'Producer targeted model/DOM checks; no independent, firmware, audio or device proof','mode':mode,'base_html_sha256':'131db6ae9aa436274735d9dca3602f7d902c3c1a32453a302d118ed6f70a2659','generator_sha256':hashlib.sha256(new.encode()).hexdigest(),'base_renderer_sha256':hashlib.sha256(working_js.encode()).hexdigest(),'executed_html_sha256':hashlib.sha256(html.encode()).hexdigest(),'checks':checks,'passed':sum(x['pass'] for x in checks),'total':len(checks)};(R/('CORRECTION-CHECKS.json' if mode=='r8' else 'PREDECESSOR-NEGATIVE-CHECKS.json')).write_text(json.dumps(out,indent=2)+'\n');print(json.dumps(out));assert (out['passed']==0 if mode=='r7' else out['passed']==out['total'])
asyncio.run(main())
