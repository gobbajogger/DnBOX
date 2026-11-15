import test from 'node:test';
import assert from 'node:assert/strict';
import {heldRouter,mountMomentary,bindCancellation} from './input.mjs';
class Target extends EventTarget {
 constructor(){super();this.attrs={};this.captures=[];this.disabled=false;this.hidden=false;}
 setAttribute(k,v){this.attrs[k]=v;}
 setPointerCapture(id){this.captures.push(id);}
 fire(type,props={}){const e=new Event(type,{cancelable:true});for(const [k,v] of Object.entries(props))Object.defineProperty(e,k,{value:v});this.dispatchEvent(e);return e;}
}
const setup=()=>{const sent=[];return {sent,r:heldRouter((...args)=>sent.push(args))};};
test('mixed sources same physical key send first down and last up in either order',()=>{
 for(const order of [['p','k'],['k','p']]){const {sent,r}=setup();r.press('p','physical',36);r.press('k','physical',36);assert.equal(r.count,2);assert.equal(r.countFor('physical',36),2);assert.deepEqual(sent,[['physical',36,100]]);r.release(order[0]);assert.equal(sent.length,1);r.release(order[1]);assert.deepEqual(sent,[['physical',36,100],['physical',36,0]]);}
});
test('repeated press owns original target, release twice is idempotent',()=>{
 const {sent,r}=setup();assert.equal(r.press('same','physical',0),true);assert.equal(r.press('same','physical',17),false);assert.equal(r.release('same'),true);assert.equal(r.release('same'),false);assert.deepEqual(sent,[['physical',0,100],['physical',0,0]]);
});
test('pads and physical keys are distinct controls; reset releases each once',()=>{
 const {sent,r}=setup();r.press('p1','pad',0);r.press('p2','pad',0);r.press('key','physical',0);r.releaseAll();r.releaseAll();assert.equal(r.count,0);assert.deepEqual(sent,[['pad',0,1],['physical',0,100],['pad',0,0],['physical',0,0]]);
});
test('invalid kind/id/token fails without commands or ownership',()=>{
 const {sent,r}=setup();for(const [kind,id] of [['key',0],['physical',-1],['physical',37],['physical',1.1],['physical',NaN],['pad',16],['pad','1']])assert.throws(()=>r.press('a',kind,id),/invalid/);for(const token of ['',null,3])assert.throws(()=>r.press(token,'physical',0),/token/);assert.equal(r.count,0);assert.deepEqual(sent,[]);
});
test('two pointer IDs hold same target and lostcapture does not double release',()=>{
 const {sent,r}=setup(),b=new Target();mountMomentary(b,{router:r,kind:'physical',getId:()=>5});b.fire('pointerdown',{pointerId:1,button:0});b.fire('pointerdown',{pointerId:2,button:0});assert.equal(r.count,2);assert.equal(b.attrs['aria-pressed'],'true');b.fire('pointerup',{pointerId:1});b.fire('lostpointercapture',{pointerId:1});assert.equal(r.count,1);b.fire('pointercancel',{pointerId:2});assert.deepEqual(sent,[['physical',5,100],['physical',5,0]]);assert.equal(b.attrs['aria-pressed'],'false');
});
test('selection changes never retarget held pointer or keyboard release',()=>{
 const {sent,r}=setup(),b=new Target();let id=0;mountMomentary(b,{router:r,kind:'physical',getId:()=>id});b.fire('pointerdown',{pointerId:1,button:0});id=36;b.fire('keydown',{key:'Enter',repeat:false});id=17;b.fire('pointerup',{pointerId:1});b.fire('keyup',{key:'Enter'});assert.deepEqual(sent,[['physical',0,100],['physical',36,100],['physical',0,0],['physical',36,0]]);
});
test('Space and Enter independently hold; repeat does not read new selection; local blur only releases keyboard',()=>{
 const {sent,r}=setup(),b=new Target();let id=4;mountMomentary(b,{router:r,kind:'physical',getId:()=>id});b.fire('pointerdown',{pointerId:1,button:0});b.fire('keydown',{key:' ',repeat:false});b.fire('keydown',{key:'Enter',repeat:false});id=99;b.fire('keydown',{key:' ',repeat:true});assert.equal(r.count,3);b.fire('blur');assert.equal(r.count,1);assert.equal(sent.length,1);b.fire('pointerup',{pointerId:1});assert.deepEqual(sent,[['physical',4,100],['physical',4,0]]);
});
test('window blur, hide, reset clear all controls and synchronize aria',()=>{
 for(const reason of ['blur','hide','reset']){const {sent,r}=setup(),win=new Target(),doc=new Target(),b=new Target(),c=bindCancellation(r,{win,doc});mountMomentary(b,{router:r,kind:'pad',getId:()=>15});b.fire('pointerdown',{pointerId:2,button:0});if(reason==='blur')win.fire('blur');if(reason==='hide'){doc.hidden=true;doc.fire('visibilitychange');}if(reason==='reset')c.reset();assert.equal(r.count,0);assert.equal(b.attrs['aria-pressed'],'false');b.fire('pointerup',{pointerId:2});assert.deepEqual(sent,[['pad',15,1],['pad',15,0]]);}
});
test('mount reset and destroy release only owned tokens, remove listeners',()=>{
 const {sent,r}=setup(),a=new Target(),b=new Target();const ma=mountMomentary(a,{router:r,kind:'physical',getId:()=>12});const mb=mountMomentary(b,{router:r,kind:'physical',getId:()=>12});a.fire('keydown',{key:' ',repeat:false});b.fire('keydown',{key:'Enter',repeat:false});ma.destroy();assert.equal(r.count,1);a.fire('pointerdown',{pointerId:1,button:0});mb.reset();assert.equal(r.count,0);assert.deepEqual(sent,[['physical',12,100],['physical',12,0]]);
});
test('disabled/nonprimary/repeat inputs ignored; cancellation still releases',()=>{
 const {sent,r}=setup(),b=new Target();mountMomentary(b,{router:r,kind:'pad',getId:()=>3});b.disabled=true;b.fire('pointerdown',{pointerId:1,button:0});b.disabled=false;b.fire('pointerdown',{pointerId:1,button:2});b.fire('keydown',{key:' ',repeat:true});assert.equal(sent.length,0);b.fire('keydown',{key:' ',repeat:false});b.disabled=true;b.fire('keyup',{key:' '});assert.deepEqual(sent,[['pad',3,1],['pad',3,0]]);
});
