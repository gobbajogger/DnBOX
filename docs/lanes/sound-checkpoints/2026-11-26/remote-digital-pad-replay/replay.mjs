import assert from 'node:assert/strict';
import {spawn} from 'node:child_process';
import readline from 'node:readline';
import fs from 'node:fs';
import {mountDnr3Link} from './remote/dnr3-link.mjs';
import * as d from './remote/dnr3.mjs';
const raw=[];
const replies=[];
const events=[];
const proc=spawn(new URL(process.argv.includes('--dual')?'./typed-cable':'./native-cable',import.meta.url).pathname);
const reader=readline.createInterface({input:proc.stdout})[Symbol.asyncIterator]();
const input=new EventTarget(); Object.assign(input,{name:'DnBOX',state:'connected'});
const output=new EventTarget(); Object.assign(output,{name:'DnBOX',state:'connected'});
let queue=Promise.resolve();
function command(line){
  const work=queue.then(async()=>{
    proc.stdin.write(line+'\n');
    for(;;){
      const r=await reader.next(); if(r.done)throw Error('Firmware subprocess exited early');
      if(r.value==='END')break;
      assert.match(r.value,/^(?:[a-f0-9]{2})+$/);
      const bytes=Buffer.from(r.value,'hex'),e=new Event('midimessage');
      replies.push(bytes); Object.defineProperty(e,'data',{value:Uint8Array.from(bytes)});
      input.dispatchEvent(e);
    }
  });
  queue=work;
  return work;
}
output.send=bytes=>{raw.push(Uint8Array.from(bytes));void command('M '+Buffer.from(bytes).toString('hex'));};
const label={dataset:{}};
let requests=[];
const link=mountDnr3Link({document:{getElementById:id=>id==='conn-state'?label:null},navigator:{
  requestMIDIAccess:async options=>{requests.push(options);return {inputs:new Map([[1,input]]),outputs:new Map([[2,output]])};}
},timeoutMs:1000});
link.onNotify(e=>events.push(e));
// Use the exact PR app's event consumers, with a purpose-built Node DOM adapter
// that throws on any unexpected selector. This is not browser/layout evidence.
const source=fs.readFileSync(new URL('./remote/app.mjs',import.meta.url),'utf8');
const block=(start,end)=>source.slice(source.indexOf(start),source.indexOf(end,source.indexOf(start)));
function node(){
 const set=new Set(),attributes=new Map();
 return {dataset:{},classList:{add:k=>set.add(k),remove:k=>set.delete(k),
   toggle:(k,v)=>v?set.add(k):set.delete(k),contains:k=>set.has(k)},
   value:'chicago',setAttribute:(k,v)=>attributes.set(k,v),getAttribute:k=>attributes.get(k),
   addEventListener(){}};
}
const ns=Object.fromEntries(['rec','play','stop','part','tempo','other-view'].map(id=>[id,node()]));
const padNote=node(),positionPainter=[];
const bind=new Function('$','document','window','status','playhead','CAP','NOTIFY_KIND','setInterval','clearInterval',
  block('function bindDevicePerformance(link)', '// ---- DnBOX Remote: connection')+'\nreturn bindDevicePerformance;');
bind(id=>ns[id],{querySelector(s){
 assert.equal(s,'#pitch-rail [data-key-note="41"]');return padNote;
}}, {addEventListener(){}}, ()=>{}, {start(){},stop(){}},d.CAP,d.NOTIFY_KIND,()=>0,()=>{})(link);
const proto=new Function('$','NOTIFY_KIND','fwColumns','playColour',
 'let fwPositions=null;\n'+block('function fwProtocol(event)', 'if(document.documentElement?.style')+'\nreturn fwProtocol;');
link.onNotify(proto(id=>ns[id],d.NOTIFY_KIND,steps=>positionPainter.push(steps),()=>0));

try{
  assert.equal(label.textContent,'Disconnected');
  await link.connect();
  assert.equal(label.textContent,'Connected');assert.equal(link.connected(),true);assert.deepEqual(requests,[{sysex:true}]);
  const ops=raw.map(d.decodeRequest); assert.deepEqual(ops.slice(0,3).map(x=>x.op),[d.OP.HELLO,d.OP.OPEN,d.OP.WATCH]);
  const watched=replies.map(d.decodeFrame).find(x=>x.op===(d.OP.WATCH|64)&&x.seq===ops[2].seq);
  assert.ok(watched); assert.equal(d.decodeReply(d.encodeFrame(watched.op,watched.seq,watched.payload)).fields.mask&(d.WATCH.VOICE|d.WATCH.POSITION),d.WATCH.VOICE|d.WATCH.POSITION);
  const client=link.drumClient();
  const pre=await client.state();
  assert.equal(await link.writeTempo(104.5),1045);
  assert.equal(await link.readTempo(),1045);
  const epoch=ops[2].fields.epoch;
  const mark=raw.length;
  await command('M '+Buffer.from(d.encodeRequest(d.OP.TEMPO,14000,{epoch,bpm_x10:1357})).toString('hex'));
  await command('T 70');
  assert.equal(await link.readTempo(),1357);
  assert.ok(events.some(x=>x.kind===d.NOTIFY_KIND.TRANSPORT&&x.fields.bpm_x10===1357),JSON.stringify(events));
  const startPad=events.length;
  await command('H 8 107');
  await command('T 45');
  const pad=events.slice(startPad).filter(e=>e.kind===d.NOTIFY_KIND.VOICE);
  console.error('PAD',JSON.stringify(events.slice(startPad)));
  assert.ok(pad.some(e=>e.fields.part===2&&e.fields.velocity===107),JSON.stringify(pad));
  assert.equal(events.slice(startPad).some(e=>e.kind===d.NOTIFY_KIND.POSITION&&e.fields.step_pos.some(x=>x!==127)),false);
  assert.equal(padNote.dataset.deviceVoiceNote,'41');
  assert.equal(padNote.classList.contains('device-voice'),true);
  assert.equal(positionPainter.some(steps=>steps.some(p=>p!==127)),false);
  await command('H 8 0');
  await command('T 45');
  assert.equal(padNote.dataset.deviceVoiceNote,undefined);
  assert.equal(padNote.classList.contains('device-voice'),false);
  const evt=events.length;
  await command('M fa');
  await command('T 300');
  await command('T 300');
  const started=events.slice(evt).filter(e=>e.kind===d.NOTIFY_KIND.POSITION).map(e=>e.fields.step_pos);
  assert.ok(started.some(a=>a[0]!==127),JSON.stringify(events.slice(evt)));
  await command('M fc');
  await command('T 60');
  assert.ok(events.slice(evt).some(e=>e.kind===d.NOTIFY_KIND.POSITION&&e.fields.step_pos.every(v=>v===127)),JSON.stringify(events.slice(evt)));
  assert.ok(raw.slice(mark).every(x=>[d.OP.STATE,d.OP.KEEPALIVE].includes(d.decodeRequest(x).op)));
  console.log(JSON.stringify({transport:'actual #206 JS Web MIDI selector/link + restored native C full-app host over raw-byte cable; no authored protocol replies',handshake:ops.slice(0,3).map(x=>x.op),caps:link.hello().caps,tempos:[pre.bpm_x10,1045,1357],platformDigitalPad:{pad:8,events:pad.map(e=>e.fields)},nativePositions:started.length,appConsumerFiredAndCleared:true,positionConsumerCount:positionPainter.length,stopCleared:true,externalStimuliNoPageEdits:true}));
  await link.disconnect();assert.equal(label.textContent,'Disconnected');
}finally{proc.stdin.end();proc.kill();}
