/* SPDX-License-Identifier: GPL-3.0-only */
/* Child bridge: accepts one transferred capability from its embedding parent. */
(function(){
 let connected=false;
 addEventListener('message',async e=>{
  if(connected||e.source!==parent||e.data?.type!=='dnbox-settings-connect'||e.data.contractVersion!==1||e.ports?.length!==1)return;
  connected=true;const port=e.ports[0],pending=new Map();let sequence=0,closed=false,bridge=null;
  function request(method,args){
   if(closed)return Promise.reject(Error('FRAME_DISPOSED'));
   return new Promise((resolve,reject)=>{const rpcId='settings-rpc-'+(++sequence);pending.set(rpcId,{resolve,reject});port.postMessage({rpcId,method,args});});
  }
  port.onmessage=async event=>{const d=event.data;if(d?.notification==='context-changed'&&bridge&&!closed){try{await bridge.refresh();port.postMessage({notificationId:d.notificationId,status:'REFRESHED'});}catch(error){port.postMessage({notificationId:d.notificationId,status:'REFRESH_FAILED',error:String(error)});}return;}const wait=pending.get(d?.rpcId);if(!wait)return;pending.delete(d.rpcId);d.error?wait.reject(Error(d.error)):wait.resolve(d.result);};port.start();
  const hostPort={readContext:a=>request('readContext',a),commitScalar:a=>request('commitScalar',a),requestClose:a=>request('requestClose',a)};
  try{
   bridge=await mountSharedSettings(hostPort);
   const original=bridge.close;
   bridge.close=async reason=>{let result;try{result=await original(reason);return result;}finally{port.postMessage({event:'closed',sessionId:bridge.adapter.sessionId,reason,status:result?.status||'CLOSE_ERROR'});closed=true;for(const wait of pending.values())wait.reject(Error('FRAME_DISPOSED'));pending.clear();port.close();}};
   port.postMessage({event:'mounted',sessionId:bridge.adapter.sessionId});
   document.getElementById('sharedMountState').textContent='Connected shared design model. Only four PERFORM refs are admitted. Other settings/actions are reserved.';
  }catch(error){port.postMessage({event:'mount-failed',sessionId:window.sharedSettings?.adapter.sessionId??null,error:String(error)});document.getElementById('sharedMountState').textContent='Shared model did not open: '+String(error);closed=true;for(const wait of pending.values())wait.reject(Error('MOUNT_FAILED'));pending.clear();port.close();}
 });
})();
