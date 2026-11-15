/* Shared current-instrument Settings frame, composed prototype only.
 * UI records own their frame/channel lifetimes. A closed notification is sent
 * only after the child received its close RPC receipt; disposal does not wait
 * behind a refresh whose reply the retiring child can no longer receive. */
(() => {
 const $=id=>document.getElementById(id);
 let current=null,epoch=0;const pending=new Map();
 const report=e=>{$('settingsStage').textContent='Shared Settings unavailable: '+String(e);};
 function closePending(request){
  if(!request.closeJob)request.closeJob=Promise.resolve().then(()=>window.__settingsHost.requestClose({sessionId:request.sessionId,reason:'frame disposal',consumedInputIds:[]}));
  return request.closeJob;
 }
 function retire(record,reason='frame disposal'){
  if(!record)return Promise.resolve();if(record.retireJob)return record.retireJob;
  record.retiring=true;record.refreshScheduled=false;
  record.retireJob=(async()=>{
   try{if(record.connection)await record.connection.dispose();else await window.__settingsHost.requestClose({sessionId:record.sessionId,reason,consumedInputIds:[]});}
   finally{record.frame?.remove();if(current===record){current=null;$('settingsMount').replaceChildren();$('closeSettings').hidden=true;$('settingsStage').textContent='Shared Settings closed. Current instrument values retained.';}}
  })();return record.retireJob;
 }
 async function present(request){
  await retire(current);
  if(request.epoch!==epoch){await closePending(request);pending.delete(request.epoch);return;}
  pending.delete(request.epoch);const record={...request,frame:null,connection:null,retiring:false,retireJob:null,refreshScheduled:false,refreshJob:Promise.resolve()};current=record;
  try{
   const frame=document.createElement('iframe');record.frame=frame;frame.id='settingsFrame';frame.title='Shared current instrument Settings design model';frame.setAttribute('sandbox','allow-scripts');frame.style.cssText='display:block;width:100%;height:1500px;border:1px solid #48616a;background:#101719';
   const load=new Promise(resolve=>frame.addEventListener('load',resolve,{once:true}));frame.srcdoc=new TextDecoder().decode(Uint8Array.from(atob(window.__sharedSettingsB64),c=>c.charCodeAt(0)));$('settingsMount').append(frame);$('closeSettings').hidden=false;$('settingsStage').textContent='Opening current caller context; no scene/reset or storage.';await load;
   if(record.retiring||current!==record||record.epoch!==epoch){await retire(record);return;}
   record.connection=DnboxSettingsFrame.connectSettingsFrame(frame,window.__settingsHost,{onClosed:window.__settingsClosed});await record.connection.ready;
   if(record.retiring||current!==record||record.epoch!==epoch){await retire(record);return;}
   $('settingsStage').textContent='Shared current instrument. K5–K8 use actual typed prototype bindings. USB/clock source, PLAY, presets and lifecycle remain reserved. Browser/peer/audio/storage not validated.';
  }catch(e){await retire(record).catch(()=>{});if(record.epoch===epoch)throw e;}
 }
 window.__presentSharedSettings=sessionId=>{
  const request={sessionId,epoch:++epoch};pending.set(request.epoch,request);document.querySelector('[data-tab="settings"]').click();present(request).catch(report);
 };
 window.__settingsContextChanged=sessionId=>{
  const record=current;if(!record||record.sessionId!==sessionId||!record.connection||record.retiring||record.refreshScheduled)return;
  record.refreshScheduled=true;record.refreshJob=record.refreshJob.then(async()=>{record.refreshScheduled=false;if(current===record&&!record.retiring)await record.connection.refresh();}).catch(e=>{if(!record.retiring)report(e);});
 };
 window.__settingsClosed=(sessionId,reason)=>{
  const record=current;if(!record||record.sessionId!==sessionId)return;
  retire(record,reason).then(()=>{if(epoch===record.epoch&&reason!=='frame disposal')document.querySelector('[data-tab="model"]').click();}).catch(report);
 };
 function leave(returnToModel){
  const leaveEpoch=++epoch;for(const request of pending.values())closePending(request).catch(report);
  retire(current).then(()=>{if(returnToModel&&epoch===leaveEpoch)document.querySelector('[data-tab="model"]').click();}).catch(report);
 }
 $('openSettings').onclick=()=>proposal.event('GLOBE');$('closeSettings').onclick=()=>leave(true);
 $('settingsChoice').innerHTML='<option>Shared current instrument context</option>';$('settingsChoice').disabled=true;
 $('settingsPair').innerHTML='<h3>Same instrument, same current values</h3><p>Enter from the musical view. Four direct PERFORM controls stay visible. Current/pending patterns, RUN/WAIT/REC, power and dirty state are read from the host. Unsupported interfaces are labelled, not simulated as completed actions.</p>';
 $('showLoad').onclick=()=>{$('settingsStage').textContent='SAVE/LOAD lifecycle is reserved. No storage action occurred.';};
 document.querySelectorAll('[data-tab]').forEach(b=>b.addEventListener('click',()=>{if(b.dataset.tab!=='settings')leave(false);}));
})();
