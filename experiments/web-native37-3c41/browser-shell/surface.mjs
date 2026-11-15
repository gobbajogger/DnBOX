// SPDX-License-Identifier: MIT
// Counted ownership uses the frozen #899-derived module; this layer only wires controls/observed feedback.
import {heldRouter,mountMomentary} from '../browser-input/input.mjs';
const WHITE=new Set([0,2,4,5,7,9,11]);
const LETTERS=['C','C♯','D','D♯','E','F','F♯','G','G♯','A','A♯','B'];
const WHITE_GRID=[0,2,4,6,7,9,11,12,14,16,18,19,21,23,24,26];
export function keyGeometry(){let white=0;return Array.from({length:37},(_,id)=>{const black=!WHITE.has(id%12),x=black?white*30-9:white++*30;return {id,black,x,width:black?18:30,height:black?86:128,label:LETTERS[id%12]+(3+Math.floor(id/12))};});}
// Frozen mount reports intent via aria-pressed. Keep intent diagnostic-only and reserve
// the actual element's aria-pressed and amber classes for acknowledged C response bits.
function intentTarget(element){return {
 get disabled(){return element.disabled;},set type(v){element.type=v;},
 addEventListener:(...a)=>element.addEventListener(...a),removeEventListener:(...a)=>element.removeEventListener(...a),
 setPointerCapture:id=>element.setPointerCapture(id),
 setAttribute:(name,value)=>element.setAttribute(name==='aria-pressed'?'data-input-held':name,value)
};}
export function wireSurface({send,keys,pads,holdKey,holdPad,keyValue,padValue,win,doc,onError=()=>{}}){
 const dispatch=(...a)=>{let work;try{work=Promise.resolve(send(...a));}catch(e){work=Promise.reject(e);}work.catch(onError);return work;};
 const router=heldRouter(dispatch),mounts=[];
 const mount=(e,kind,getId)=>{e.disabled=true;e.setAttribute('aria-pressed','false');mounts.push(mountMomentary(intentTarget(e),{router,kind,getId}));};
 keys.forEach((e,id)=>mount(e,'physical',()=>id));pads.forEach((e,id)=>mount(e,'pad',()=>id));mount(holdKey,'physical',keyValue);mount(holdPad,'pad',padValue);
 let current=null,disposed=false,transitioning=false,transitionPromise=null,destroyPromise=null;
 const alive=()=>{if(disposed)throw Error('surface is disposed');};
 const disable=()=>{for(const e of [...keys,...pads,holdKey,holdPad]){e.disabled=true;e.setAttribute('aria-disabled','true');}};
 function beginTransition(fixture=null){
  alive();if(transitioning)throw Error('surface transition in progress');transitioning=true;disable();
  const work=(async()=>{
   try{router.releaseAll();await dispatch('release');alive();if(fixture!==null)return await dispatch('fixture',fixture);}
   catch(error){current=null;throw error;}
   finally{transitioning=false;if(!disposed){if(current)update(current);else disable();}}
  })();
  transitionPromise=work;work.then(()=>{if(transitionPromise===work)transitionPromise=null;},()=>{if(transitionPromise===work)transitionPromise=null;});return work;
 }
 function releaseAll(){alive();return transitionPromise??beginTransition();}
 const safeRelease=()=>{if(disposed)return;try{releaseAll().catch(()=>{});}catch(error){onError(error);}};
 const hide=()=>{if(doc.hidden)safeRelease();};
 win.addEventListener('blur',safeRelease);doc.addEventListener('visibilitychange',hide);
 const setState=(e,pressed,css)=>{e.classList.toggle(css,pressed);e.setAttribute('aria-pressed',String(pressed));};
 function update(frame){
  alive();
  const held=frame?.physical?.held,padHeld=frame?.physical?.padHeld;
  if(!Number.isSafeInteger(held)||held<0||held>=2**37||!Number.isInteger(padHeld)||padHeld<0||padHeld>65535)throw Error('invalid physical acknowledgement');
  current=frame;const bitmap=BigInt(held),leds=frame.leds.keys,available=frame.state.gridPadAvailable===true;
  keys.forEach((e,id)=>{e.disabled=transitioning;e.setAttribute('aria-disabled',String(transitioning));setState(e,Boolean(bitmap&(1n<<BigInt(id))),'key-pressed');const k=id-5;e.classList.toggle('key-led',k>=0&&k<27&&Boolean(leds&(1<<k)));});
  pads.forEach((e,id)=>{e.disabled=transitioning||!available;e.setAttribute('aria-disabled',String(e.disabled));setState(e,Boolean(padHeld&(1<<id)),'pressed');e.classList.toggle('lit',available&&Boolean(leds&(1<<WHITE_GRID[id])));});
  holdKey.disabled=transitioning;holdPad.disabled=transitioning||!available;holdKey.setAttribute('aria-disabled',String(holdKey.disabled));holdPad.setAttribute('aria-disabled',String(holdPad.disabled));
  const k=Number(keyValue()),p=Number(padValue());
  holdKey.setAttribute('aria-pressed',String(Number.isInteger(k)&&k>=0&&k<37&&Boolean(bitmap&(1n<<BigInt(k)))));
  holdPad.setAttribute('aria-pressed',String(Number.isInteger(p)&&p>=0&&p<16&&Boolean(padHeld&(1<<p))));
 }
 return Object.freeze({
  update,releaseAll,
  async loadFixture(id){alive();if(!Number.isInteger(id)||id<0||id>19)throw RangeError('invalid fixture');return beginTransition(id);},
  pressLesson(token,kind,id){alive();if(transitioning)throw Error('surface transition in progress');if(!current)throw Error('current C acknowledgement required');if(kind==='pad'&&current?.state.gridPadAvailable!==true)throw Error('pad mode unavailable');return router.press('lesson:'+token,kind,id);},
  releaseLesson(token){alive();return router.release('lesson:'+token);},
  get active(){return !disposed&&router.count>0;},
  destroy(){if(disposed)return destroyPromise;disposed=true;disable();for(const m of mounts)m.destroy();router.releaseAll();win.removeEventListener('blur',safeRelease);doc.removeEventListener('visibilitychange',hide);destroyPromise=(async()=>{if(transitionPromise)await transitionPromise.catch(()=>{});return dispatch('release');})();return destroyPromise;}
 });
}
