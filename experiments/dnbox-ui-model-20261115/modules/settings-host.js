/* SPDX-License-Identifier: GPL-3.0-only */
/* Whole-instrument DESIGN MODEL bindings. Not firmware/storage/peer APIs. */
(() => {
 const ids=['perform.tempo','perform.swing','perform.filter','perform.pump'];
 const ranges=[[20,275],[0,100],[0,127],[0,127]];
 let generation=1,sequence=0,active=null;
 const revisions=Object.fromEntries(ids.map(id=>[id,{raw:null,revision:0}]));
 const raw=id=>id===ids[0]?S.tempo:id===ids[1]?S.swingRaw:id===ids[2]?(ensureMaster(),S.mst[13]):(ensureMaster(),S.mst[6]);
 function observe(){for(const id of ids){const r=revisions[id],q=raw(id);if(r.raw!==q){if(r.raw!==null)r.revision++;r.raw=q;}}}
 function snapshot(){observe();ensureMaster();const caller=active?.caller??{};return {contract:'dnbox-settings-host-context/1',session:structuredClone(active.session),caller:structuredClone(caller),musical:{currentPatterns:[...S.currentPatterns],pendingPatterns:[...S.pendingPatterns],partMutes:S.partMutes,pendingPartMutes:S.pendingPartMutes,trackMutes:[...S.trackMutes],pendingTrackMutes:[...S.pendingTrackMutes]},transport:{run:S.playing,wait:S.wait,rec:S.rec,clip:S.clipOn,clockSource:S.clockSource,clockSignal:S.clockSignal},scalars:Object.fromEntries(ids.map((id,i)=>[id,{storedRaw:raw(id),appliedRaw:i<2?raw(id):S.mstLive[i===2?13:6],revision:revisions[id].revision,writable:!S.clipOn&&S.overlay!=='ASK'}])),power:S.power,lifecycle:{dirty:S.dirty,save:S.save,state:modelSaveState()},quick:{filterText:masterText(8),pumpText:masterText(6),filterApplied:{mode:S.mstLive[8],cutoff:S.mstLive[9],resonance:S.mstLive[10]}},ownedInputs:{localNotes:Object.keys(S.liveNotes).length,inputEpoch:S.liveEpoch}};}
 function enter(){
  if(active&&!active.closed)return;
  observe();active={session:{id:'settings-'+(++sequence)+'-'+crypto.randomUUID(),projectGeneration:generation,inputEpoch:S.liveEpoch},caller:{view:S.view,part:S.part,displayPart:S.part,sourcePart:S.sourcePart,partLabel:laneName(S.part),soundLabel:S.engine,page:S.page,list:S.list,overlay:S.overlay,pageMemory:structuredClone(S.pageMemory)},receipts:new Map(),closed:false};
  window.__presentSharedSettings?.(active.session.id);
 }
 const port={
  readContext(req){if(req?.contractVersion!==1||!active||active.closed||(req.sessionId&&req.sessionId!==active.session.id))throw Error('STALE_SESSION');if(active.session.projectGeneration!==generation)throw Error('STALE_PROJECT');return structuredClone(snapshot());},
  commitScalar(req){
   if(!active||active.closed)return {status:'STALE_SESSION',eventId:req?.eventId,parameterId:req?.parameterId};
   const key=JSON.stringify(req),cached=active.receipts.get(req?.eventId);
   if(cached)return structuredClone(cached.key===key?cached.receipt:{status:'RESERVED_INTERFACE',reason:'EVENT_ID_REUSE',eventId:req?.eventId,parameterId:req?.parameterId,context:snapshot()});
   const i=ids.indexOf(req?.parameterId),id=req?.parameterId;
   let status=req?.contractVersion!==1||req?.sessionId!==active.session.id||req?.inputEpoch!==active.session.inputEpoch||S.liveEpoch!==active.session.inputEpoch?'STALE_SESSION':req.projectGeneration!==generation||req.projectGeneration!==active.session.projectGeneration?'STALE_PROJECT':i<0?'UNSUPPORTED_REF':!['perform-encoder','settings-arrow'].includes(req.inputClass)||!Number.isSafeInteger(req.valueRaw)||req.valueRaw<ranges[i][0]||req.valueRaw>ranges[i][1]||typeof req.eventId!=='string'||!req.eventId?'RESERVED_INTERFACE':S.overlay==='ASK'?'INPUT_MASKED':S.clipOn?'RESERVED_INTERFACE':req.beforeRaw!==raw(id)?'STALE_VALUE':req.valueRaw===raw(id)?'NO_CHANGE':'APPLIED';
   if(status==='APPLIED'){
    if(i<2){const before=raw(id);if(i===0)S.tempo=req.valueRaw;else S.swingRaw=req.valueRaw;S.genUndo.push({kind:'settingsScalar',id,before,after:req.valueRaw});S.genRedo=[];}
    else{const before=allSoundSnapshot(),r=ref('MST',i===2?13:6);putShared(r,req.valueRaw);motionTouch({key:refKey(r)});pushSoundHistory('SETTINGS '+id,before,allSoundSnapshot());}
    S.dirty=true;observe();render();
   }
   const receipt={status,eventId:req?.eventId,parameterId:id,valueRaw:i<0?undefined:raw(id),refRevision:i<0?undefined:revisions[id].revision,context:snapshot()};
   if(typeof req?.eventId==='string')active.receipts.set(req.eventId,{key,receipt:structuredClone(receipt)});
   return structuredClone(receipt);
  },
  requestClose(req){
   if(!active||req?.sessionId!==active.session.id)return {status:'STALE_SESSION'};
   if(active.closed)return structuredClone(active.closeReceipt);
   if(!Array.isArray(req.consumedInputIds)||req.consumedInputIds.some(x=>typeof x!=='string'||!x.startsWith('settings-input:'+active.session.id+':')))return {status:'RESERVED_INTERFACE',reason:'INVALID_INPUT_OWNER'};
   const beforeClose=snapshot();active.closed=true;
   if(active.session.projectGeneration===generation){const c=active.caller;S.view=c.view;S.part=c.displayPart;S.page=c.page;S.list=c.list;S.overlay=c.overlay;S.pageMemory=structuredClone(c.pageMemory);render();}
   active.closeReceipt={status:'CLOSED',caller:structuredClone(active.caller),context:beforeClose};return structuredClone(active.closeReceipt);
  }
 };
 const baseReset=reset,baseHistory=genHistory;
 reset=function(...args){generation++;if(active)active.closed=true;active=null;for(const id of ids)revisions[id]={raw:null,revision:0};return baseReset(...args);};
 genHistory=function(redo){const from=redo?S.genRedo:S.genUndo,e=from?.at(-1);if(e?.kind!=='settingsScalar')return baseHistory(redo);from.pop();S.usedButtons.push('PATCH');if(e.id===ids[0])S.tempo=redo?e.after:e.before;else S.swingRaw=redo?e.after:e.before;(redo?S.genUndo:S.genRedo).push(e);S.dirty=true;observe();showNotice((redo?'REDO':'UNDO')+' '+e.id.replace('perform.','').toUpperCase());};
 const baseRender=render;render=function(...args){const result=baseRender(...args);if(S.view==='PART'&&S.part>2&&!S.overlay&&pages()[S.page]?.title==='SOUND'){const y=24+(S.page%4)*50;rect(145,y,89,11,C.sel);text('MODEL READ ONLY',232,y,10,C.ink,true,'right');rect(0,227,240,13,C.bg);text('SOUND ADMISSION RESERVED / CP1',4,228,10,C.amber);}if(active&&!active.closed)window.__settingsContextChanged?.(active.session.id);return result;};
 window.__settingsHost=Object.freeze(port);
 window.__openReservedSettings=enter;
 Object.assign(window.proposal,{reset,render,settingsPort:port,enterSettings(){event('GLOBE');return port;}});
})();
