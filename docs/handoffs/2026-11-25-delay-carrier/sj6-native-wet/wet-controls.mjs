import fs from 'node:fs';
import assert from 'node:assert/strict';
import * as d from '../../remote206-product-ux/web/product-site/remote/dnr3.mjs';
const [mode,path,status] = process.argv.slice(2);
const enc=(ms,op,seq,fields)=>ms+':'+Buffer.from(d.encodeRequest(op,seq,fields)).toString('hex');
const base=[enc(1300,d.OP.HELLO,7,{client_proto:3}),enc(1500,d.OP.OPEN,8,{client_id:1121})];
const lookup=(p)=>{
 const rows=fs.readFileSync(p,'utf8').split('\n').flatMap(s=>{
   const hit=s.match(/\]\s+usb in:\s+(f0 7d 44 4e 03\b.*)/);
   if(!hit)return [];
   const b=Buffer.from(hit[1].replace(/\s/g,''),'hex');
   return [d.decodeFrame(b).op===d.OP.NOTIFY?d.decodeNotify(b):d.decodeReply(b)];
 });
 const r=(seq)=>{const row=rows.find(x=>x.seq===seq); assert.ok(row,'missing '+seq); return row;};
 return {r,rows};
};
const ids=[118,25,26,27,24], values=[1,45,64,127,1];
const params=ids.map(c=>({kind:112,a:3,b:1,c}));
const bank={kind:41,a:0,b:1,c:0}, state={kind:43,a:0,b:1,c:0};
let epoch,adopt;
if(mode==='open'){console.log(base.join(';'));process.exit(0);}
epoch=lookup(path).r(8).fields.epoch;assert.ok(epoch);
adopt=[...base,
 enc(1900,d.OP.KEEPALIVE,9,{epoch}),
 enc(2150,d.OP.SYNTH_OWNER,10,{epoch,slot:1}),
 enc(2350,d.OP.PARAM_GET,11,{params:[bank,state,...params]})];
if(mode==='adopt'){console.log(adopt.join(';'));process.exit(0);}
if(mode==='wet'){
 const {r}=lookup(status);const owner=r(10).fields, before=r(11).fields.params;
 assert.equal(owner.bank,3);assert.equal(owner.settled,1);assert.ok(owner.owner_generation);
 assert.deepEqual(before.slice(0,2).map(x=>x.value),[3,1]);
 assert.ok(before.slice(2).every((x,i)=>x.value!==d.KEEP&&values[i]<=x.max));
 const notes=[];for(const [at,n] of [[4030,60],[4670,64],[5300,67],[5950,55],[6610,72]]) {
   notes.push(at+':92'+n.toString(16)+'64',(at+185)+':82'+n.toString(16)+'00');
 }
 console.log([...adopt, enc(2470,d.OP.SYNTH_OWNER,40,{epoch,slot:1}), ...params.map((p,i)=>enc(2500+i*130,d.OP.PARAM_SET,12+i,{epoch,...p,expect:before[i+2].value,value:values[i],expect_bank:3,owner_generation:owner.owner_generation})),
  enc(3420,d.OP.PARAM_GET,31,{params:[bank,state,...params]}), enc(3500,d.OP.WATCH,32,{epoch,mask:d.WATCH.VOICE}),enc(3720,d.OP.SYNTH_OWNER,41,{epoch,slot:1}),enc(4200,d.OP.KEEPALIVE,42,{epoch}),enc(5600,d.OP.KEEPALIVE,43,{epoch}),enc(7000,d.OP.KEEPALIVE,44,{epoch}),...notes,
  enc(7610,d.OP.SYNTH_OWNER,33,{epoch,slot:1}),enc(7780,d.OP.PARAM_GET,34,{params:[bank,state,...params]})].sort((a,b)=>parseInt(a)-parseInt(b)).join(';'));process.exit(0);
}
if(mode==='receipt'){
 const {r,rows}=lookup(status);
 const owner=r(10).fields, finish=r(33).fields;
 assert.equal(r(7).fields.board,+process.argv[5]);
 assert.ok(r(7).fields.caps & d.CAP.SYNTH_OWNER);
 assert.equal(owner.bank,3);assert.equal(owner.settled,1);assert.deepEqual(finish,owner);
 assert.deepEqual(r(40).fields,owner); assert.deepEqual(r(41).fields,owner);
 assert.equal(r(11).fields.params[4].value,63);
 [42,43,44].forEach(seq=>assert.equal(r(seq).status,d.STATUS.OK));
 for(let i=0;i<ids.length;i++){
   assert.ok([d.STATUS.OK,d.STATUS.QUEUED].includes(r(12+i).status));
   assert.equal(r(12+i).fields.value,values[i]);
 }
 [31,34].forEach(i=>assert.deepEqual(r(i).fields.params.map(x=>x.value),[3,1,...values]));
 const active=rows.filter(e=>e.kind===d.NOTIFY_KIND.VOICE&&e.fields.part===3&&e.fields.velocity>0).map(e=>e.fields.note);
 assert.deepEqual(active,[60,64,67,55,72]);
 console.log(JSON.stringify({board:r(7).fields.board,owner,startEndIndependentReadback:ids.map((id,i)=>({id,value:values[i]})),active,emulatedOnly:true}));
}
