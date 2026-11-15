"""Reproduce inherited question/redo priority and preserve caller/release contracts.
This is the real proposal in local Chromium; no native firmware/storage executes.
"""
from pathlib import Path
import asyncio,hashlib,json,sys
from playwright.async_api import async_playwright
async def main():
 source,output=map(Path,sys.argv[1:3]);raw=(source/'workbench.html').read_bytes();checks=[];traces=[];errors=[]
 async with async_playwright() as pw:
  browser=await pw.chromium.launch(executable_path='/usr/bin/chromium',headless=True,args=['--no-sandbox']);p=await browser.new_page();p.on('pageerror',lambda e:errors.append(str(e)));await p.route('**/*',lambda r:r.abort());await p.set_content(raw.decode());await p.wait_for_function('window.__ready')
  await p.evaluate("""window.hist=(redo)=>{proposal.buttonDown('PATCH');proposal.buttonDown(redo?'SEQ PLAY':'SEQ REC');proposal.buttonUp(redo?'SEQ PLAY':'SEQ REC');proposal.buttonUp('PATCH')};window.redoFixture=()=>{proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);proposal.physicalUp(9,800);hist(false)};window.openQuestion=(caller=null)=>proposal.configure({overlay:'ASK',askOverlay:caller,askType:'COPY',copyRequest:{source:[0,0,0,0,0],dest:5},list:2});window.snap=()=>{const s=proposal.state;return {note:s.patternData[0].bass[1].notes[2].note,rec:s.rec,playing:s.playing,overlay:s.overlay,askType:s.askType,copyRequest:s.copyRequest,view:s.view,list:s.list,page:s.page,undo:s.genUndo.length,redo:s.genRedo.length,buttons:s.buttons,held:s.held,physical:Object.keys(s.physicalOwners),clipOn:s.clipOn,clipRequests:s.clipRequests}}""")
  async def ck(name,script,expected):
   if script:await p.evaluate(script)
   s=await p.evaluate('snap()');checks.append({'name':name,'pass':all(s[k]==v for k,v in expected.items()),'expected':expected,'actual':s})
  await p.evaluate("redoFixture();openQuestion('LIST');proposal.buttonDown('PATCH')")
  for label,code in [('PATCH held',''),('PLAY down',"proposal.buttonDown('SEQ PLAY')"),('PLAY up',"proposal.buttonUp('SEQ PLAY')"),('PATCH up',"proposal.buttonUp('PATCH')")]:
   if code:await p.evaluate(code)
   traces.append({'stage':label,'state':await p.evaluate('snap()')})
  await ck('ASK PATCH+PLAY performs Redo without cancelling question or changing caller/transport', '', {'note':56,'rec':False,'playing':False,'overlay':'ASK','askType':'COPY','copyRequest':{'source':[0,0,0,0,0],'dest':5},'view':'GEN','list':2,'undo':1,'redo':0,'buttons':[]})
  for rec,playing in [(False,True),(True,False),(True,True)]:
   setup="redoFixture();openQuestion('LIST');proposal.configure("+json.dumps({'rec':rec,'playing':playing})+");hist(true)"
   await ck('ASK Redo preserves existingREC/transport '+str((rec,playing)),setup,{'note':56,'rec':rec,'playing':playing,'overlay':'ASK','view':'GEN','undo':1,'redo':0,'buttons':[]})
  await ck('ASK Redo consumes PATCH release even when PATCH is released before PLAY',"redoFixture();openQuestion('LIST');proposal.buttonDown('PATCH');proposal.buttonDown('SEQ PLAY');proposal.buttonUp('PATCH');proposal.buttonUp('SEQ PLAY')",{'note':56,'rec':False,'playing':False,'overlay':'ASK','view':'GEN','undo':1,'redo':0,'buttons':[]})
  await ck('Empty Redo still leaves ASK/caller/transport intact',"proposal.reset('gen-known');openQuestion('LIST');hist(true)",{'note':55,'rec':False,'playing':False,'overlay':'ASK','view':'GEN','undo':0,'redo':0,'buttons':[]})
  for caller in [None,'LIST']:
   for playing in [False,True]:
    setup="redoFixture();openQuestion("+json.dumps(caller)+");proposal.configure("+json.dumps({'playing':playing})+");proposal.buttonDown('SEQ PLAY');proposal.buttonUp('SEQ PLAY')"
    await ck('OrdinaryASK PLAY answersNO then toggles once; caller '+str(caller)+' running '+str(playing),setup,{'note':55,'rec':False,'playing':not playing,'overlay':caller,'askType':None,'copyRequest':None,'view':'GEN','list':2,'undo':0,'redo':1,'buttons':[]})
  await ck('ASK PATCH+REC is masked before PATCH release',"redoFixture();openQuestion('LIST');proposal.buttonDown('PATCH');proposal.buttonDown('SEQ REC');proposal.buttonUp('SEQ REC')",{'note':55,'rec':False,'playing':False,'overlay':'ASK','view':'GEN','undo':0,'redo':1,'buttons':['PATCH']})
  await ck('Unused PATCH release answersNO to originalLIST caller without undo',"proposal.buttonUp('PATCH')",{'note':55,'rec':False,'playing':False,'overlay':'LIST','askType':None,'copyRequest':None,'view':'GEN','list':2,'undo':0,'redo':1,'buttons':[]})
  await ck('Ordinary REC alone is still masked by ASK',"redoFixture();openQuestion('LIST');proposal.buttonDown('SEQ REC');proposal.buttonUp('SEQ REC')",{'note':55,'rec':False,'playing':False,'overlay':'ASK','view':'GEN','undo':0,'redo':1,'buttons':[]})
  await ck('ASK lower knobs remain masked, not history or value changes',"redoFixture();openQuestion('LIST');proposal.event('K5',1);proposal.event('K6',1)",{'note':55,'rec':False,'playing':False,'overlay':'ASK','view':'GEN','undo':0,'redo':1})
  await ck('ASK REC with an availableUndo entry stays masked',"proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);proposal.physicalUp(9,800);openQuestion('LIST');proposal.buttonDown('PATCH');proposal.buttonDown('SEQ REC');proposal.buttonUp('SEQ REC')",{'note':56,'rec':False,'playing':False,'overlay':'ASK','view':'GEN','undo':1,'redo':0,'buttons':['PATCH']})
  await ck('MaskedREC then PATCH release cancels to caller without applyingUndo',"proposal.buttonUp('PATCH')",{'note':56,'rec':False,'playing':False,'overlay':'LIST','view':'GEN','undo':1,'redo':0,'buttons':[]})
  await ck('Existing occupiedCOPY question retains its real caller during Redo',"redoFixture();proposal.event('PATCH');S.patternData[5].drums[0].steps[0]=[2];proposal.buttonDown('BT');proposal.padDown(5);proposal.release(80,5);proposal.buttonUp('BT');hist(true)",{'note':56,'rec':False,'playing':False,'overlay':'ASK','askType':'COPY','view':'HOME','undo':1,'redo':0,'buttons':[]})
  await ck('GEN occupiedCOPY caller also staysGEN after Redo and originalreleases',"redoFixture();S.patternData[5].drums[0].steps[0]=[2];proposal.buttonDown('BT');proposal.padDown(5);proposal.release(80,5);proposal.buttonUp('BT');hist(true)",{'note':56,'rec':False,'playing':False,'overlay':'ASK','askType':'COPY','view':'GEN','undo':1,'redo':0,'buttons':[]})
  await ck('OrdinaryPLAY cancels occupiedCOPY without copying and toggles once',"redoFixture();proposal.event('PATCH');S.patternData[5].drums[0].steps[0]=[2];proposal.buttonDown('BT');proposal.padDown(5);proposal.release(80,5);proposal.buttonUp('BT');proposal.buttonDown('SEQ PLAY');proposal.buttonUp('SEQ PLAY')",{'note':55,'rec':False,'playing':True,'overlay':None,'askType':None,'copyRequest':None,'view':'HOME','undo':0,'redo':1,'buttons':[]})
  await ck('Local ASK Redo exception does not take over clip host transport',"redoFixture();proposal.event('KNOB BANK');openQuestion(null);proposal.buttonDown('PATCH');proposal.buttonDown('SEQ PLAY');proposal.buttonUp('SEQ PLAY')",{'note':55,'rec':False,'playing':False,'overlay':'ASK','view':'CLIP','undo':0,'redo':1,'clipOn':True,'clipRequests':[{'type':'SEQ PLAY host request'}]})
  await browser.close()
 out={'classification':'Producer MODEL/DOM inherited-parity regression checks; no firmware/device/storage/independent proof','html_sha256':hashlib.sha256(raw).hexdigest(),'source_contract':'3c41 play_question.h b981dc579ded10c535272c217d653afb0637991b; ui.c3964–4006','checks':checks,'traces':traces,'errors':errors,'passed':sum(c['pass'] for c in checks),'total':len(checks)};output.write_text(json.dumps(out,indent=2)+'\n');print(json.dumps({'html_sha256':out['html_sha256'],'passed':out['passed'],'total':out['total'],'failed':[c['name'] for c in checks if not c['pass']],'errors':errors}))
 if errors:raise SystemExit(2)
 if out['passed']!=out['total']:raise SystemExit(1)
asyncio.run(main())
