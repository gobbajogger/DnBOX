"""Source-contract history chords under LIST; real proposal DOM, no firmware execution.

Catches history incorrectly masked by LIST, leaking into REC/transport/PATCH release.
Literal note55/56 and stack sizes come from gen-known plus one explicit K5 turn.
"""
from pathlib import Path
import asyncio, hashlib, json, sys
from playwright.async_api import async_playwright

async def main():
    source, output = map(Path, sys.argv[1:3])
    raw = (source / 'workbench.html').read_bytes()
    checks, traces, errors = [], [], []
    async with async_playwright() as pw:
        browser = await pw.chromium.launch(executable_path='/usr/bin/chromium', headless=True, args=['--no-sandbox'])
        page = await browser.new_page()
        page.on('pageerror', lambda e: errors.append(str(e)))
        await page.route('**/*', lambda r: r.abort())
        await page.set_content(raw.decode())
        await page.wait_for_function('window.__ready')
        await page.evaluate("""window.overlayHistorySnapshot=()=>{const s=proposal.state;return {note:s.patternData[0].bass[1].notes[2].note,rec:s.rec,playing:s.playing,overlay:s.overlay,view:s.view,page:s.page,list:s.list,undo:s.genUndo.length,redo:s.genRedo.length,buttons:s.buttons,usedButtons:s.usedButtons,held:s.held,physical:Object.keys(s.physicalOwners)}};window.historyChord=(redo)=>{proposal.buttonDown('PATCH');proposal.buttonDown(redo?'SEQ PLAY':'SEQ REC');proposal.buttonUp(redo?'SEQ PLAY':'SEQ REC');proposal.buttonUp('PATCH')};window.seedHistory=()=>{proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);proposal.physicalUp(9,800)}""")
        async def run(name, script, expected):
            await page.evaluate(script)
            actual = await page.evaluate('overlayHistorySnapshot()')
            checks.append({'name': name, 'pass': all(actual[k] == v for k,v in expected.items()), 'expected': expected, 'actual': actual})
        await page.evaluate("seedHistory();proposal.event('SCALE');proposal.buttonDown('PATCH')")
        for label, script in [('PATCH held', ''), ('REC down', "proposal.buttonDown('SEQ REC')"), ('REC release', "proposal.buttonUp('SEQ REC')"), ('PATCH release', "proposal.buttonUp('PATCH')")]:
            if script: await page.evaluate(script)
            traces.append({'case':'exact reviewer Undo reproducer','stage':label,'state':await page.evaluate('overlayHistorySnapshot()')})
        s=await page.evaluate('overlayHistorySnapshot()')
        expected={'note':55,'rec':False,'playing':False,'overlay':'LIST','view':'GEN','undo':0,'redo':1,'buttons':[]}
        checks.append({'name':'Exact LIST Undo restores note, leaves REC/transport/list and consumes releases','pass':all(s[k]==v for k,v in expected.items()),'expected':expected,'actual':s})
        await run('LIST Redo restores note and preserves list caller', "seedHistory();historyChord(false);proposal.event('ARP');proposal.event('SCALE');proposal.event('K2',2);historyChord(true)", {'note':56,'rec':False,'playing':False,'overlay':'LIST','view':'GEN','page':1,'list':2,'undo':1,'redo':0,'buttons':[]})
        for rec,playing in [(False,True),(True,False),(True,True)]:
            setup="seedHistory();proposal.event('SCALE');proposal.configure("+json.dumps({'rec':rec,'playing':playing})+");"
            await run('LIST Undo preserves existing REC/transport '+str((rec,playing)),setup+"historyChord(false)",{'note':55,'rec':rec,'playing':playing,'overlay':'LIST','undo':0,'redo':1,'buttons':[]})
            await run('LIST Redo preserves existing REC/transport '+str((rec,playing)),setup+"historyChord(false);historyChord(true)",{'note':56,'rec':rec,'playing':playing,'overlay':'LIST','undo':1,'redo':0,'buttons':[]})
        await run('Empty LIST Undo consumes PATCH without changing REC',"proposal.reset('gen-known');proposal.event('SCALE');historyChord(false)",{'note':55,'rec':False,'playing':False,'overlay':'LIST','undo':0,'redo':0,'buttons':[]})
        await run('Empty LIST Redo consumes PATCH without changing transport',"proposal.reset('gen-known');proposal.event('SCALE');historyChord(true)",{'note':55,'rec':False,'playing':False,'overlay':'LIST','undo':0,'redo':0,'buttons':[]})
        await run('Old physical cell release after LIST Undo does not reapply/duplicate edit',"proposal.reset('gen-known');proposal.physicalDown(9);proposal.event('K5',1);proposal.event('SCALE');historyChord(false);proposal.physicalUp(9,800)",{'note':55,'rec':False,'playing':False,'overlay':'LIST','undo':0,'redo':1,'held':-1,'physical':[],'buttons':[]})
        await run('Plain LIST REC retains its ordinary command',"proposal.reset('gen-known');proposal.event('SCALE');proposal.buttonDown('SEQ REC');proposal.buttonUp('SEQ REC')",{'note':55,'rec':True,'playing':False,'overlay':'LIST','undo':0,'redo':0,'buttons':[]})
        await run('Plain LIST PLAY retains its ordinary transport command',"proposal.reset('gen-known');proposal.event('SCALE');proposal.buttonDown('SEQ PLAY');proposal.buttonUp('SEQ PLAY')",{'note':55,'rec':False,'playing':True,'overlay':'LIST','undo':0,'redo':0,'buttons':[]})
        await run('No-overlay Undo/Redo remains available',"seedHistory();historyChord(false);historyChord(true)",{'note':56,'rec':False,'playing':False,'overlay':None,'undo':1,'redo':0,'buttons':[]})
        await browser.close()
    result={'classification':'Producer MODEL/DOM source-contract checks; no firmware, independent, device or audio proof','input_html_sha256':hashlib.sha256(raw).hexdigest(),'source_contract':'current3c41 ui.c4009–4034; O_ASK precedence at3997–4008','checks':checks,'traces':traces,'errors':errors,'passed':sum(c['pass'] for c in checks),'total':len(checks)}
    output.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps({'html_sha256':result['input_html_sha256'],'passed':result['passed'],'total':result['total'],'failed':[c['name'] for c in checks if not c['pass']],'errors':errors,'output':str(output)}))
    if errors: raise SystemExit(2)
    if result['passed']!=result['total']: raise SystemExit(1)

asyncio.run(main())
