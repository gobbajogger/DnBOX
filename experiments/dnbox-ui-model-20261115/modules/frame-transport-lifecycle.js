/* SPDX-License-Identifier: GPL-3.0-only */
/* Parent-side sandbox MessageChannel, not a musical state implementation. */
(function(root,factory){const api=factory();if(typeof module==='object'&&module.exports)module.exports=api;else root.DnboxSettingsFrame=api;})(globalThis,function(){
 'use strict';
 function connectSettingsFrame(frame,hostPort,options={}){
  for(const name of['readContext','commitScalar','requestClose'])if(typeof hostPort?.[name]!=='function')throw TypeError('Missing host port '+name);
  if(!frame?.contentWindow?.postMessage)throw TypeError('Loaded Settings frame required');
  const channel=new MessageChannel(),server=channel.port1,client=channel.port2,jobs=new Set(),cache=new Map();
  let disposed=false,sessionId=null,closePromise=null,disposePromise=null,notificationSequence=0;
  const notifications=new Map();let readyResolve,readyReject;const ready=new Promise((resolve,reject)=>{readyResolve=resolve;readyReject=reject;});ready.catch(()=>{});
  function closeOnce(args){
   if(closePromise)return closePromise;
   closePromise=Promise.resolve().then(()=>hostPort.requestClose(args));return closePromise;
  }
  async function invoke(method,args){
   if(!['readContext','commitScalar','requestClose'].includes(method))throw Error('UNSUPPORTED_METHOD');
   if(disposed)throw Error('FRAME_DISPOSED');
   if(!args||typeof args!=='object')throw Error('INVALID_ARGUMENT');
   if(method==='readContext'){
    if(args.contractVersion!==1||(sessionId&&args.sessionId!==sessionId))throw Error('STALE_SESSION');
    const context=await hostPort.readContext(structuredClone(args));
    if(typeof context?.session?.id!=='string')throw Error('INVALID_CONTEXT');
    if(sessionId&&context.session.id!==sessionId)throw Error('STALE_SESSION');
    sessionId=context.session.id;
    if(disposed)await closeOnce({sessionId,reason:'frame disposal',consumedInputIds:[]});
    return context;
   }
   if(!sessionId||args.sessionId!==sessionId)throw Error('STALE_SESSION');
   if(method==='requestClose'){
    if(!Array.isArray(args.consumedInputIds)||args.consumedInputIds.some(id=>typeof id!=='string'||!id.startsWith('settings-input:'+sessionId+':')))throw Error('INVALID_INPUT_OWNER');
    return closeOnce(structuredClone(args));
   }
   return hostPort.commitScalar(structuredClone(args));
  }
  server.onmessage=e=>{
   const d=e.data;if(d?.event==='closed'&&d.sessionId===sessionId){options.onClosed?.(d.sessionId,d.reason,d.status);return;}if(d?.event==='mount-failed'&&d.sessionId===sessionId){const error=Error(d.error||'MOUNT_FAILED');readyReject(error);for(const wait of notifications.values())wait.reject(error);notifications.clear();return;}
   if(d?.event==='mounted'&&d.sessionId===sessionId){readyResolve({status:'MOUNTED',sessionId});return;}
   if(typeof d?.notificationId==='string'&&notifications.has(d.notificationId)){const wait=notifications.get(d.notificationId);notifications.delete(d.notificationId);d.status==='REFRESHED'?wait.resolve({status:'REFRESHED'}):wait.reject(Error(d.error||'REFRESH_FAILED'));return;}
   if(!d||typeof d.rpcId!=='string'||!d.rpcId.length)return;
   const key=JSON.stringify([d.method,d.args]);
   if(cache.has(d.rpcId)&&cache.get(d.rpcId).key!==key){server.postMessage({rpcId:d.rpcId,error:'RPC_ID_REUSE'});return;}
   let job=cache.get(d.rpcId)?.job;
   if(!job){job=invoke(d.method,d.args);cache.set(d.rpcId,{key,job});}
   const send=job.then(result=>{if(!disposed)server.postMessage({rpcId:d.rpcId,result});},error=>{if(!disposed)server.postMessage({rpcId:d.rpcId,error:String(error?.message||error)});});
   jobs.add(send);send.finally(()=>jobs.delete(send));
  };
  server.start();frame.contentWindow.postMessage({type:'dnbox-settings-connect',contractVersion:1},'*',[client]);
  return {
   ready,
   async refresh(){await ready;if(disposed)throw Error('FRAME_DISPOSED');return new Promise((resolve,reject)=>{const notificationId='context-refresh-'+(++notificationSequence);notifications.set(notificationId,{resolve,reject});server.postMessage({notification:'context-changed',notificationId});});},
   dispose(){
    if(disposePromise)return disposePromise;disposed=true;readyReject(Error('FRAME_DISPOSED'));for(const wait of notifications.values())wait.reject(Error('FRAME_DISPOSED'));notifications.clear();
    disposePromise=(async()=>{await Promise.allSettled([...jobs]);if(sessionId)await closeOnce({sessionId,reason:'frame disposal',consumedInputIds:[]});server.close();return{status:'DISPOSED'};})();
    return disposePromise;
   }
  };
 }
 return Object.freeze({connectSettingsFrame});
});
