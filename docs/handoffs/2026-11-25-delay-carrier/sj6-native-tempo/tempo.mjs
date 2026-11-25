import * as d from '../../remote206-product-ux/web/product-site/remote/dnr3.mjs';
import fs from 'node:fs';
import assert from 'node:assert/strict';
const [mode, prior, variant, path]=process.argv.slice(2);
const enc=(t,op,seq,fields)=>t+':'+Buffer.from(d.encodeRequest(op,seq,fields)).toString('hex');
const common=[enc(1300,d.OP.HELLO,7,{client_proto:3}),enc(1500,d.OP.OPEN,8,{client_id:1121})];
function decode(p){
 const rows=fs.readFileSync(p,'utf8').split('\n').flatMap(s=>{
 const m=s.match(/\]\s+usb in:\s+(f0 7d 44 4e 03\b.*)/);
 if(!m)return [];
 const b=Buffer.from(m[1].replace(/\s/g,''),'hex');
 return [d.decodeFrame(b).op===d.OP.NOTIFY?d.decodeNotify(b):d.decodeReply(b)];
 });
 const r=seq=>{const q=rows.find(x=>x.seq===seq);assert.ok(q,'missing '+seq);assert.equal(q.status,0,'status '+seq);return q.fields};
 return {r,rows};
}
if(mode==='open'){console.log(common.join(';'));process.exit(0);}
const epoch=decode(prior).r(8).epoch;
assert.ok(epoch);
const p=[118,25,26,27,24].map(c=>({kind:112,a:3,b:1,c}));
const params={params:p};
const first=[...common,enc(1750,d.OP.TEMPO,10,{epoch,bpm_x10:1200}),
 enc(1810,d.OP.STATE,11,{}),enc(1940,d.OP.SYNTH_OWNER,12,{epoch,slot:1}),
 enc(2070,d.OP.PARAM_GET,13,params)];
if(mode==='profile'){console.log(first.join(';'));process.exit(0);}
function ownerBefore(p){const {r}=decode(p);const owner=r(12);
 assert.equal(owner.slot,1);assert.equal(owner.bank,3);assert.equal(owner.settled,1);assert.ok(owner.owner_generation);
 assert.deepEqual(r(13).params.map(x=>x.value),[1,45,63,127,1]);return owner;
}
if(mode==='commands'){
 const owner=ownerBefore(path);const dry=variant==='dry';
 const ads=[...first,enc(2160,d.OP.PARAM_SET,14,{epoch,...p[4],expect:1,value:dry?0:1,expect_bank:3,owner_generation:owner.owner_generation}),
 enc(2250,d.OP.PARAM_GET,15,params),enc(2370,d.OP.WATCH,16,{epoch,mask:d.WATCH.VOICE}),'2600:923c64','2640:823c00',
 enc(2750,d.OP.STATE,17,{})];
 if(variant!=='early'){
  ads.push(enc(2920,d.OP.TEMPO,18,{epoch,bpm_x10:1250}),enc(3030,d.OP.STATE,19,{}),
    enc(3520,d.OP.SYNTH_OWNER,20,{epoch,slot:1}),enc(3670,d.OP.PARAM_GET,21,params));
 }
 console.log(ads.join(';'));process.exit(0);
}
if(mode==='receipt'){
 const {r,rows}=decode(path);const owner=ownerBefore(path);const dry=variant==='dry';
 assert.equal(r(10).bpm_x10,1200);assert.equal(r(11).bpm_x10,1200);assert.equal(r(17).bpm_x10,1200);
 const clock=r(11).clk_src;assert.equal(r(17).clk_src,clock);
 const values=[1,45,63,127,dry?0:1];
 assert.deepEqual(r(15).params.map(x=>x.value),values);
 if(variant!=='early'){
  assert.equal(r(18).bpm_x10,1250);assert.equal(r(19).bpm_x10,1250);assert.equal(r(19).clk_src,clock);
  assert.deepEqual(r(20),owner);assert.deepEqual(r(21).params.map(x=>x.value),values);
 }
 assert.ok(rows.some(q=>q.kind===d.NOTIFY_KIND.VOICE&&q.fields.part===3&&q.fields.note===60&&q.fields.velocity>0));
 console.log(JSON.stringify({owner,clock,variant,positiveNote:60,bpm:variant==='early'?[120]:[120,125],values,clockSourceConstant:true}));
}
